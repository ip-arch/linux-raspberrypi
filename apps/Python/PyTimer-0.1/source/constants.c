/*
Copyright (c) 2015-2023 Naohiko Shimizu

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
#include "constants.h"

void define_constants(PyObject *module)
{

   clock_monotonic = Py_BuildValue("i", CLOCK_MONOTONIC);
   PyModule_AddObject(module, "CLOCK_MONOTONIC", clock_monotonic);

   clock_realtime = Py_BuildValue("i", CLOCK_REALTIME);
   PyModule_AddObject(module, "CLOCK_REALTIME", clock_realtime);

   clock_process_cputime_id = Py_BuildValue("i", CLOCK_PROCESS_CPUTIME_ID);
   PyModule_AddObject(module, "CLOCK_PROCESS_CPUTIME_ID", clock_process_cputime_id);

   clock_thread_cputime_id = Py_BuildValue("i", CLOCK_THREAD_CPUTIME_ID);
   PyModule_AddObject(module, "CLOCK_THREAD_CPUTIME_ID", clock_thread_cputime_id);

   sigev_none = Py_BuildValue("i", SIGEV_NONE);
   PyModule_AddObject(module, "SIGEV_NONE", sigev_none);

   sigev_thread = Py_BuildValue("i", SIGEV_THREAD);
   PyModule_AddObject(module, "SIGEV_THREAD", sigev_thread);

   timer_abstime = Py_BuildValue("i", TIMER_ABSTIME);
   PyModule_AddObject(module, "TIMER_ABSTIME", timer_abstime);


   version = Py_BuildValue("s", "0.1");
   PyModule_AddObject(module, "VERSION", version);
}
