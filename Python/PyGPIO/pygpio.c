/*
 * pygpio.c -- GPIO character device API v2 (linux/gpio.h) 用のPython拡張モジュール
 *
 * py_timer.c と同じ設計方針:
 *   - ioctl呼び出しと構造体のバイナリレイアウトはすべてC側に隠す
 *   - Python側にはタプル/intの素直なインターフェースだけを見せる
 *
 * py_timer.c のSIGEV_THREAD方式(カーネル/pthread側のスレッドから
 * 直接Pythonコールバックを呼ぶ)とは異なり、本モジュールは
 * request_line() が返す line_fd をそのままのUNIX fdとして扱う。
 * epoll/timerfdによる多重待機はPython側 (select.epoll, ctypes等)
 * に委ねる。理由: コールバックをC側のスレッドから直接Pythonへ
 * 呼び戻す設計では、Pythonコールバック内の長時間処理(GC一時停止等)
 * がイベント配送そのものをブロックしうる。fdだけを返せば、
 * Pythonのイベントループが主導権を握ったまま扱える。
 *
 * chip_fd自体はただのファイルディスクリプタなので、open/closeは
 * Python標準の os.open()/os.close() をそのまま使えばよい
 * (本モジュールでは提供しない)。
 *
 * ビルド:
 *   $ python3 setup.py build_ext --inplace
 *   または
 *   $ gcc -O2 -shared -fPIC $(python3-config --includes) \
 *         -o pygpio$(python3-config --extension-suffix) pygpio.c
 */

#include <Python.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>

#define PYGPIO_MAX_ATTRS GPIO_V2_LINE_NUM_ATTRS_MAX  /* 10 */
#define PYGPIO_MAX_LINES GPIO_V2_LINES_MAX           /* 64 */

/* ---------------------------------------------------------------
 * get_chip_info(chip_fd) -> (name, label, lines)
 * --------------------------------------------------------------- */
static PyObject *py_get_chip_info(PyObject *self, PyObject *args, PyObject *kwargs)
{
    int chip_fd;
    static char *kwlist[] = {"chip_fd", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i", kwlist, &chip_fd))
        return NULL;

    struct gpiochip_info ci;
    memset(&ci, 0, sizeof(ci));

    if (ioctl(chip_fd, GPIO_GET_CHIPINFO_IOCTL, &ci) < 0) {
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    }

    return Py_BuildValue("(ssI)", ci.name, ci.label, (unsigned int)ci.lines);
}

/* ---------------------------------------------------------------
 * get_line_info(chip_fd, offset) -> (offset, flags, name, consumer)
 * --------------------------------------------------------------- */
static PyObject *py_get_line_info(PyObject *self, PyObject *args, PyObject *kwargs)
{
    int chip_fd;
    unsigned int offset;
    static char *kwlist[] = {"chip_fd", "offset", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "iI", kwlist, &chip_fd, &offset))
        return NULL;

    struct gpio_v2_line_info li;
    memset(&li, 0, sizeof(li));
    li.offset = offset;

    if (ioctl(chip_fd, GPIO_V2_GET_LINEINFO_IOCTL, &li) < 0) {
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    }

    return Py_BuildValue("(IKss)", li.offset, (unsigned long long)li.flags,
                          li.name, li.consumer);
}

/*
 * attrs引数を解析して gpio_v2_line_config に詰める。
 * attrs は [(attr_id:int, value:int, mask:int), ...] の形式
 * (Python側は python-listのtupleをそのまま渡す = struct
 *  gpio_v2_line_config_attribute の (attr.id, attr.flags/values, mask)
 *  に1対1対応)。
 * 戻り値: 成功なら0、失敗なら-1 (PyErrをセット済み)
 */
static int fill_line_config(PyObject *attrs_obj, uint64_t default_flags,
                             struct gpio_v2_line_config *cfg)
{
    memset(cfg, 0, sizeof(*cfg));
    cfg->flags = default_flags;

    if (attrs_obj == NULL || attrs_obj == Py_None)
        return 0;

    if (!PySequence_Check(attrs_obj)) {
        PyErr_SetString(PyExc_TypeError, "attrs must be a sequence of (id, value, mask) tuples");
        return -1;
    }

    Py_ssize_t n = PySequence_Size(attrs_obj);
    if (n > PYGPIO_MAX_ATTRS) {
        PyErr_Format(PyExc_ValueError, "attrs has %zd entries, max is %d",
                     n, PYGPIO_MAX_ATTRS);
        return -1;
    }

    for (Py_ssize_t i = 0; i < n; i++) {
        PyObject *item = PySequence_GetItem(attrs_obj, i); /* new ref */
        unsigned int attr_id;
        unsigned long long value, mask;

        int ok = PyArg_ParseTuple(item, "IKK", &attr_id, &value, &mask);
        Py_DECREF(item);
        if (!ok) {
            PyErr_SetString(PyExc_TypeError,
                             "each attrs entry must be (id:int, value:int, mask:int)");
            return -1;
        }

        cfg->attrs[i].attr.id = attr_id;
        cfg->attrs[i].attr.flags = value;  /* union: flags/values/debounce_period_us 共用 */
        cfg->attrs[i].mask = mask;
    }

    cfg->num_attrs = (uint32_t)n;
    return 0;
}

/* ---------------------------------------------------------------
 * request_line(chip_fd, offsets, consumer=b"pygpio",
 *              default_flags=0, attrs=None, event_buffer_size=0)
 *   -> line_fd
 * --------------------------------------------------------------- */
static PyObject *py_request_line(PyObject *self, PyObject *args, PyObject *kwargs)
{
    int chip_fd;
    PyObject *offsets_obj;
    const char *consumer = "pygpio";
    unsigned long long default_flags = 0;
    PyObject *attrs_obj = NULL;
    unsigned int event_buffer_size = 0;

    static char *kwlist[] = {
        "chip_fd", "offsets", "consumer", "default_flags",
        "attrs", "event_buffer_size", NULL
    };

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "iO|sKOI", kwlist,
            &chip_fd, &offsets_obj, &consumer, &default_flags,
            &attrs_obj, &event_buffer_size))
        return NULL;

    if (!PySequence_Check(offsets_obj)) {
        PyErr_SetString(PyExc_TypeError, "offsets must be a sequence of ints");
        return NULL;
    }

    Py_ssize_t num_lines = PySequence_Size(offsets_obj);
    if (num_lines <= 0 || num_lines > PYGPIO_MAX_LINES) {
        PyErr_Format(PyExc_ValueError, "offsets must have 1..%d entries",
                     PYGPIO_MAX_LINES);
        return NULL;
    }

    struct gpio_v2_line_request req;
    memset(&req, 0, sizeof(req));

    for (Py_ssize_t i = 0; i < num_lines; i++) {
        PyObject *item = PySequence_GetItem(offsets_obj, i); /* new ref */
        long off = PyLong_AsLong(item);
        Py_DECREF(item);
        if (off == -1 && PyErr_Occurred())
            return NULL;
        req.offsets[i] = (uint32_t)off;
    }
    req.num_lines = (uint32_t)num_lines;

    snprintf(req.consumer, sizeof(req.consumer), "%s", consumer);
    req.event_buffer_size = event_buffer_size;

    if (fill_line_config(attrs_obj, default_flags, &req.config) < 0)
        return NULL;

    if (ioctl(chip_fd, GPIO_V2_GET_LINE_IOCTL, &req) < 0) {
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    }

    return PyLong_FromLong(req.fd);
}

/* ---------------------------------------------------------------
 * set_config(line_fd, default_flags=0, attrs=None)
 *   GPIO_V2_LINE_SET_CONFIG_IOCTL: line_fdをcloseせずに再設定する
 * --------------------------------------------------------------- */
static PyObject *py_set_config(PyObject *self, PyObject *args, PyObject *kwargs)
{
    int line_fd;
    unsigned long long default_flags = 0;
    PyObject *attrs_obj = NULL;

    static char *kwlist[] = {"line_fd", "default_flags", "attrs", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i|KO", kwlist,
                                      &line_fd, &default_flags, &attrs_obj))
        return NULL;

    struct gpio_v2_line_config cfg;
    if (fill_line_config(attrs_obj, default_flags, &cfg) < 0)
        return NULL;

    if (ioctl(line_fd, GPIO_V2_LINE_SET_CONFIG_IOCTL, &cfg) < 0) {
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    }

    Py_RETURN_NONE;
}

/* ---------------------------------------------------------------
 * get_values(line_fd, mask) -> bits
 * --------------------------------------------------------------- */
static PyObject *py_get_values(PyObject *self, PyObject *args, PyObject *kwargs)
{
    int line_fd;
    unsigned long long mask;
    static char *kwlist[] = {"line_fd", "mask", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "iK", kwlist, &line_fd, &mask))
        return NULL;

    struct gpio_v2_line_values vals = { .bits = 0, .mask = mask };

    if (ioctl(line_fd, GPIO_V2_LINE_GET_VALUES_IOCTL, &vals) < 0) {
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    }

    return PyLong_FromUnsignedLongLong(vals.bits);
}

/* ---------------------------------------------------------------
 * set_values(line_fd, bits, mask) -> None
 * --------------------------------------------------------------- */
static PyObject *py_set_values(PyObject *self, PyObject *args, PyObject *kwargs)
{
    int line_fd;
    unsigned long long bits, mask;
    static char *kwlist[] = {"line_fd", "bits", "mask", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "iKK", kwlist,
                                      &line_fd, &bits, &mask))
        return NULL;

    struct gpio_v2_line_values vals = { .bits = bits, .mask = mask };

    if (ioctl(line_fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &vals) < 0) {
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    }

    Py_RETURN_NONE;
}

/* ---------------------------------------------------------------
 * read_event(line_fd) -> (timestamp_ns, id, offset, seqno, line_seqno)
 *
 * line_fdがreadableになってから呼ぶこと(Python側でepollしておく)。
 * readableでない状態で呼ぶとブロックする点はCのread(2)と同じ。
 * --------------------------------------------------------------- */
static PyObject *py_read_event(PyObject *self, PyObject *args, PyObject *kwargs)
{
    int line_fd;
    static char *kwlist[] = {"line_fd", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i", kwlist, &line_fd))
        return NULL;

    struct gpio_v2_line_event ev;
    ssize_t n;

    Py_BEGIN_ALLOW_THREADS
    n = read(line_fd, &ev, sizeof(ev));
    Py_END_ALLOW_THREADS

    if (n < 0) {
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    }
    if ((size_t)n != sizeof(ev)) {
        PyErr_SetString(PyExc_IOError, "short read on gpio line event");
        return NULL;
    }

    return Py_BuildValue("(KIIII)",
                          (unsigned long long)ev.timestamp_ns,
                          (unsigned int)ev.id,
                          (unsigned int)ev.offset,
                          (unsigned int)ev.seqno,
                          (unsigned int)ev.line_seqno);
}

/* ---------------------------------------------------------------
 * モジュール定義
 * --------------------------------------------------------------- */
static const char moduledocstring[] =
    "GPIO character device API v2 (linux/gpio.h) の薄いラッパー。\n"
    "chip_fd/line_fdは通常のUNIX fd (os.open/os.closeで開閉し、\n"
    "select.epoll等でそのまま多重待機できる)。";

static PyMethodDef PyGpio_methods[] = {
    {"get_chip_info", (PyCFunction)py_get_chip_info, METH_VARARGS | METH_KEYWORDS,
     "get_chip_info(chip_fd) -> (name, label, lines)"},
    {"get_line_info", (PyCFunction)py_get_line_info, METH_VARARGS | METH_KEYWORDS,
     "get_line_info(chip_fd, offset) -> (offset, flags, name, consumer)"},
    {"request_line", (PyCFunction)py_request_line, METH_VARARGS | METH_KEYWORDS,
     "request_line(chip_fd, offsets, consumer=\"pygpio\", default_flags=0,\n"
     "              attrs=None, event_buffer_size=0) -> line_fd\n"
     "attrs is a sequence of (attr_id, value, mask) tuples, 1:1 with\n"
     "struct gpio_v2_line_config_attribute."},
    {"set_config", (PyCFunction)py_set_config, METH_VARARGS | METH_KEYWORDS,
     "set_config(line_fd, default_flags=0, attrs=None) -> None"},
    {"get_values", (PyCFunction)py_get_values, METH_VARARGS | METH_KEYWORDS,
     "get_values(line_fd, mask) -> bits"},
    {"set_values", (PyCFunction)py_set_values, METH_VARARGS | METH_KEYWORDS,
     "set_values(line_fd, bits, mask) -> None"},
    {"read_event", (PyCFunction)py_read_event, METH_VARARGS | METH_KEYWORDS,
     "read_event(line_fd) -> (timestamp_ns, id, offset, seqno, line_seqno)"},
    {NULL, NULL, 0, NULL}
};

static void define_constants(PyObject *module)
{
    PyModule_AddIntConstant(module, "LINE_FLAG_USED",        GPIO_V2_LINE_FLAG_USED);
    PyModule_AddIntConstant(module, "LINE_FLAG_ACTIVE_LOW",  GPIO_V2_LINE_FLAG_ACTIVE_LOW);
    PyModule_AddIntConstant(module, "LINE_FLAG_INPUT",       GPIO_V2_LINE_FLAG_INPUT);
    PyModule_AddIntConstant(module, "LINE_FLAG_OUTPUT",      GPIO_V2_LINE_FLAG_OUTPUT);
    PyModule_AddIntConstant(module, "LINE_FLAG_EDGE_RISING", GPIO_V2_LINE_FLAG_EDGE_RISING);
    PyModule_AddIntConstant(module, "LINE_FLAG_EDGE_FALLING",GPIO_V2_LINE_FLAG_EDGE_FALLING);
    PyModule_AddIntConstant(module, "LINE_FLAG_OPEN_DRAIN",  GPIO_V2_LINE_FLAG_OPEN_DRAIN);
    PyModule_AddIntConstant(module, "LINE_FLAG_OPEN_SOURCE", GPIO_V2_LINE_FLAG_OPEN_SOURCE);
    PyModule_AddIntConstant(module, "LINE_FLAG_BIAS_PULL_UP",  GPIO_V2_LINE_FLAG_BIAS_PULL_UP);
    PyModule_AddIntConstant(module, "LINE_FLAG_BIAS_PULL_DOWN",GPIO_V2_LINE_FLAG_BIAS_PULL_DOWN);
    PyModule_AddIntConstant(module, "LINE_FLAG_BIAS_DISABLED", GPIO_V2_LINE_FLAG_BIAS_DISABLED);

    PyModule_AddIntConstant(module, "LINE_ATTR_ID_FLAGS",         GPIO_V2_LINE_ATTR_ID_FLAGS);
    PyModule_AddIntConstant(module, "LINE_ATTR_ID_OUTPUT_VALUES", GPIO_V2_LINE_ATTR_ID_OUTPUT_VALUES);
    PyModule_AddIntConstant(module, "LINE_ATTR_ID_DEBOUNCE",      GPIO_V2_LINE_ATTR_ID_DEBOUNCE);

    PyModule_AddIntConstant(module, "LINE_EVENT_RISING_EDGE",  GPIO_V2_LINE_EVENT_RISING_EDGE);
    PyModule_AddIntConstant(module, "LINE_EVENT_FALLING_EDGE", GPIO_V2_LINE_EVENT_FALLING_EDGE);
}

static struct PyModuleDef PyGpiomodule = {
    PyModuleDef_HEAD_INIT,
    "pygpio",
    moduledocstring,
    -1,
    PyGpio_methods
};

PyMODINIT_FUNC PyInit_pygpio(void)
{
    PyObject *module = PyModule_Create(&PyGpiomodule);
    if (module == NULL)
        return NULL;

    define_constants(module);
    return module;
}
