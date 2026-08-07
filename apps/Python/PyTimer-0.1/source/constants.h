/*
Copyright (c) 2023 Naohiko Shimizu

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

#include <signal.h>

#ifndef PYTIMER
PyObject *clock_monotonic;
PyObject *clock_realtime;
PyObject *clock_process_cputime_id;
PyObject *clock_thread_cputime_id;
PyObject *sigev_none;
PyObject *sigev_thread;
PyObject *timer_abstime;
PyObject *version;
#else
extern PyObject *clock_monotonic;
extern PyObject *clock_realtime;
extern PyObject *clock_process_cputime_id;
extern PyObject *clock_thread_cputime_id;
extern PyObject *sigev_none;
extern PyObject *sigev_thread;
extern PyObject *timer_abstime;
extern PyObject *version;
#endif

void define_constants(PyObject *module);
