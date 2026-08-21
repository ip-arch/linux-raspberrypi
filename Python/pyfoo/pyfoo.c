/*
Copyright (c) 2015 Naohiko Shimizu

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "Python.h"

// python function foo(arg, oparg)
static PyObject *py_foo(PyObject *self, PyObject *args, PyObject *kwargs)
{
   double arg,oparg=-1;
 
   static char *kwlist[] = {"arg", "oparg", NULL};

   if (!PyArg_ParseTupleAndKeywords(args, kwargs, "d|d", kwlist, &arg, &oparg))
      return NULL;
   return Py_BuildValue("(dd)", arg, oparg);
}

// python function goo(arg)
static PyObject *py_goo(PyObject *self, PyObject *args)
{
   int arg;
   if (!PyArg_ParseTuple(args, "i", &arg))
       return NULL;
   switch(arg)
   {
    case 0:    return Py_BuildValue("s", "arg=0");
    case 1:    PyErr_SetString(PyExc_RuntimeError, "arg==1");
	       return NULL;
    case 2:    PyErr_SetString(PyExc_ValueError, "arg==2");
	       return NULL;
    case 3:    PyErr_SetString(PyExc_BaseException, "arg==3");
	       return NULL;
#if PY_MAJOR_VERSION > 2
    case 4:    PyErr_SetString(PyExc_SystemError, "arg==4");
	       return NULL;
#else
    case 4:    PyErr_SetString(PyExc_StandardError, "arg==4");
	       return NULL;
#endif
    case 5:    PyErr_SetString(PyExc_OSError, "arg==5");
	       return NULL;
    default:   break;
   } 
   Py_RETURN_NONE;
}

int fib(int arg) {
	if(arg<2) return arg;
	return (fib(arg-1)+fib(arg-2));
}

// python function fib(arg)
static PyObject *py_fib(PyObject *self, PyObject *args)
{
   int arg;
   if (!PyArg_ParseTuple(args, "i", &arg))
       return NULL;
   return Py_BuildValue("i", fib(arg));
   Py_RETURN_NONE;
}


static const char moduledocstring[] = "pyfoo functions";

static PyMethodDef pyfoo_methods[] = {
   {"foo", (PyCFunction)py_foo, METH_VARARGS | METH_KEYWORDS, "foo - function1\narg - argument\n[oparg] - optional argument\n"},
   {"goo", (PyCFunction)py_goo, METH_VARARGS, "\ngoo - function2\narg - argument"},
   {"fib", (PyCFunction)py_fib, METH_VARARGS, "\nfib - function2\narg - argument"},
   {NULL, NULL, 0, NULL}
};

#if PY_MAJOR_VERSION > 2
static struct PyModuleDef pyfoomodule = {
   PyModuleDef_HEAD_INIT,
   "pyfoo",       // name of module
   moduledocstring,  // module documentation, may be NULL
   -1,               // size of per-interpreter state of the module, or -1 if the module keeps state in global variables.
   pyfoo_methods
};
#endif



#if PY_MAJOR_VERSION > 2
PyMODINIT_FUNC PyInit_pyfoo(void)
#else
PyMODINIT_FUNC initpyfoo(void)
#endif
{
   PyObject *module = NULL;

#if PY_MAJOR_VERSION > 2
   if ((module = PyModule_Create(&pyfoomodule)) == NULL)
      return NULL;
#else
   if ((module = Py_InitModule("pyfoo", pyfoo_methods)) == NULL)
      return;
#endif

   if (!PyEval_ThreadsInitialized())
      PyEval_InitThreads();

#if PY_MAJOR_VERSION > 2
   return module;
#else
   return;
#endif
}
