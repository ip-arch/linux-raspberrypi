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

int fib(int x) {
  int i;
  if(x<2) return x;
  else return fib(x-1)+fib(x-2);
}

// python function fib(arg, oparg)
static PyObject *py_fib(PyObject *self, PyObject *args, PyObject *kwargs)
{
   int arg;
 
   static char *kwlist[] = {"arg", NULL};

   if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i", kwlist, &arg))
      return NULL;
   return Py_BuildValue("i", fib(arg));
}



static const char moduledocstring[] = "pyfib functions";

static PyMethodDef pyfib_methods[] = {
   {"fib", (PyCFunction)py_fib, METH_VARARGS | METH_KEYWORDS, "fib - function\narg - argument\n"},
   {NULL, NULL, 0, NULL}
};

#if PY_MAJOR_VERSION > 2
static struct PyModuleDef pyfib = {
   PyModuleDef_HEAD_INIT,
   "pyfib",       // name of module
   moduledocstring,  // module documentation, may be NULL
   -1,               // size of per-interpreter state of the module, or -1 if the module keeps state in global variables.
   pyfib_methods
};
#endif



#if PY_MAJOR_VERSION > 2
PyMODINIT_FUNC PyInit_pyfib(void)
#else
PyMODINIT_FUNC initpyfib(void)
#endif
{
   PyObject *module = NULL;

#if PY_MAJOR_VERSION > 2
   if ((module = PyModule_Create(&pyfibmodule)) == NULL)
      return NULL;
#else
   if ((module = Py_InitModule("pyfib", pyfib_methods)) == NULL)
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
