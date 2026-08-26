/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
*/

#define PYTIMER

#define TIME_T_32
#undef  USE_MUTEX

#if defined(TIME_T_32)
#define U_TIMER_T	unsigned int
#else
#define U_TIMER_T	timer_t
#endif
#include "Python.h"
#include "constants.h"
#include <time.h>
#include <errno.h>
#if defined(USE_MUTEX)
#include <pthread.h>
#endif

static unsigned int timercount = 1;

struct py_callback
{
   int timerid;
   PyObject *py_cb;
   struct py_callback *next;
};
static struct py_callback *py_callbacks = NULL;
#if defined(USE_MUTEX)
pthread_mutex_t mutex;
#endif

struct py_timer
{
    U_TIMER_T  timerid;
    unsigned int timercount;
    struct sigevent *sevp;
    struct py_timer *next;
};
static struct py_timer *pytimers = NULL;


static void run_py_callbacks(union sigval timerid)
{
   PyObject *result;
   PyGILState_STATE gstate;
   struct py_callback *cb = py_callbacks;

   while (cb != NULL)
   {
      if (cb->timerid == timerid.sival_int) {
         // run callback
         gstate = PyGILState_Ensure();
         result = PyObject_CallFunction(cb->py_cb, "i", timerid.sival_int);
         if (result == NULL && PyErr_Occurred()){
            PyErr_Print();
            PyErr_Clear();
         }
         Py_XDECREF(result);
         PyGILState_Release(gstate);
      }
      cb = cb->next;
   }
}

static int add_py_callback(unsigned timerid, struct sigevent *sevp, PyObject *cb_func)
{
   struct py_callback *new_py_cb;
   struct py_callback *cb = py_callbacks;

   // add callback to py_callbacks list
   new_py_cb = malloc(sizeof(struct py_callback));
   if (new_py_cb == 0)
   {
      PyErr_NoMemory();
      return -1;
   }
   new_py_cb->py_cb = cb_func;
   Py_XINCREF(cb_func);         // Add a reference to new callback
   new_py_cb->timerid = timerid;
   new_py_cb->next = NULL;
#if defined(USE_MUTEX)
   pthread_mutex_lock(&mutex);
#endif
   if (py_callbacks == NULL) {
      py_callbacks = new_py_cb;
   } else {
      // add to end of list
      while (cb->next != NULL)
         cb = cb->next;
      cb->next = new_py_cb;
   }
#if defined(USE_MUTEX)
   pthread_mutex_unlock(&mutex);
#endif
   sevp->sigev_notify_function = run_py_callbacks;
   sevp->sigev_value.sival_int = timerid;
   sevp->sigev_notify = SIGEV_THREAD;
   return 0;
}

// python function create(clockid, callback)
static PyObject *py_create(PyObject *self, PyObject *args, PyObject *kwargs)
{
   struct sigevent *sevp = NULL;
   struct py_timer *newtimer;
   U_TIMER_T  timerid;
   int clockid;
   PyObject *cb_func = NULL;

   char *kwlist[] = {"clockid", "callback", NULL};

   if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i|O", kwlist, &clockid, &cb_func))
      return NULL;

   if (cb_func && !PyCallable_Check(cb_func))
   {
      PyErr_SetString(PyExc_TypeError, "Parameter must be callable");
      return NULL;
   }

   sevp = (struct sigevent*)malloc(sizeof(struct sigevent));
   if (sevp == NULL)
   {
      PyErr_NoMemory();
      return NULL;
   }
   memset(sevp, 0, sizeof(struct sigevent));
   sevp->sigev_notify = SIGEV_NONE;

   switch (clockid)
   {
     case CLOCK_REALTIME:
     case CLOCK_MONOTONIC:
     case CLOCK_PROCESS_CPUTIME_ID:
     case CLOCK_THREAD_CPUTIME_ID:
         break;
     default:
         PyErr_SetString(PyExc_ValueError, "Invalid clockid value");
         return NULL;
   }

   if (cb_func)
   {
      if (add_py_callback(timercount, sevp, cb_func) != 0)
      {
         PyErr_SetString(PyExc_RuntimeError, "add callback failed");
         return NULL;
      }
   }

   if (timer_create(clockid, sevp, &timerid))
   {
      PyErr_SetString(PyExc_RuntimeError, "timer_create cause an error");
      free(sevp);
      return NULL;
   }
   newtimer = (struct py_timer*)malloc(sizeof(struct py_timer));
   if (newtimer == NULL)
   {
      PyErr_NoMemory();
      return NULL;
   }
   newtimer->timerid = timerid;
   newtimer->timercount = timercount;
   newtimer->sevp = sevp;
   newtimer->next = pytimers;
   pytimers = newtimer;

   timercount++;
   return Py_BuildValue("i", newtimer->timercount);
}

// python function delete(timerid)
static PyObject *py_delete(PyObject *self, PyObject *args)
{
   unsigned int timerid;
   struct py_timer *ct = pytimers, **oct = &pytimers;

   if (!PyArg_ParseTuple(args, "i", &timerid))
      return NULL;

   while(ct)
   {
     if(ct->timercount == timerid)
     {
        if(timer_delete(ct->timerid))
        {
          PyErr_SetString(PyExc_RuntimeError, "timer_delete cause an error");
          return NULL;
        }
        free(ct->sevp);
        *oct = ct->next;
        free(ct);
        Py_RETURN_NONE;
     }
     oct = &ct->next;
     ct = ct->next;
   } 
   PyErr_SetString(PyExc_RuntimeError, "Specified timer does not exist");
   return NULL;
}

// python function gettime(timerid)
static PyObject *py_gettime(PyObject *self, PyObject *args)
{
   double curtime, curinterval;
   unsigned int timerid;
   struct itimerspec itm;
   struct py_timer *ct = pytimers;

   if (!PyArg_ParseTuple(args, "i", &timerid))
      return NULL;
   while(ct)
   {
     if(ct->timercount == timerid)
     {
	break;
     }
     ct = ct->next;
   } 
   if(!ct) {
        PyErr_SetString(PyExc_RuntimeError, "could not find the specific timer");
	return NULL;
   }
   if(timer_gettime(ct->timerid, &itm))
   {
     PyErr_SetString(PyExc_RuntimeError, "timer_gettime cause an error");
     return NULL;
   }
   curtime = itm.it_value.tv_sec + itm.it_value.tv_nsec/1e9;
   curinterval = itm.it_interval.tv_sec + itm.it_interval.tv_nsec/1e9;
   return Py_BuildValue("(dd)", curtime, curinterval);
}

// python function settime(timerid, it_value, it_interval)
static PyObject *py_settime(PyObject *self, PyObject *args)
{
   double d_value, d_interval=0.;
   unsigned int timerid;
   int flags=0;
   struct itimerspec itm;
   struct py_timer *ct = pytimers;
   
   if (!PyArg_ParseTuple(args, "id|di", &timerid, &d_value, &d_interval, &flags))
       return NULL;
   while(ct)
   {
     if(ct->timercount == timerid)
     {
	break;
     }
     ct = ct->next;
   } 
   if(!ct) {
        PyErr_SetString(PyExc_RuntimeError, "could not find the specific timer");
	return NULL;
   }
   itm.it_value.tv_sec = (int)d_value;
   itm.it_value.tv_nsec = (d_value-(int)d_value)*1e9;
   itm.it_interval.tv_sec = (int)d_interval;
   itm.it_interval.tv_nsec = (d_interval-(int)d_interval)*1e9;
   if(flags != TIMER_ABSTIME) flags = 0;
   if(timer_settime(ct->timerid, flags, &itm, NULL))
   {
     PyErr_SetString(PyExc_RuntimeError, "timer_settime cause an error");
     return NULL;
   }
   Py_RETURN_NONE;
}




static const char moduledocstring[] = "Posix interval timer functions";

static PyMethodDef PyTimer_methods[] = {
   {"create", (PyCFunction)(void(*)(void))py_create, METH_VARARGS | METH_KEYWORDS, "Create a timer module\nclockid        - type of the clock\n[callback]    - callback function\n"},
   {"settime", (PyCFunction)py_settime, METH_VARARGS, "\nTimerId - TimerId from create\nStart time - First occurence of timer event in floating point number\nIteration period - interval time in floating point number"},
   {"gettime", (PyCFunction)py_gettime, METH_VARARGS, "Get the current timer value in tuple (remain, interval)\nTimerId - Timer id"},
   {"delete", (PyCFunction)py_delete, METH_VARARGS, "Delete the timer\nTimerId - timer id"},
   {NULL, NULL, 0, NULL}
};

#if PY_MAJOR_VERSION > 2
static struct PyModuleDef PyTimermodule = {
   PyModuleDef_HEAD_INIT,
   "PyTimer",       // name of module
   moduledocstring,  // module documentation, may be NULL
   -1,               // size of per-interpreter state of the module, or -1 if the module keeps state in global variables.
   PyTimer_methods
};
#endif



#if PY_MAJOR_VERSION > 2
PyMODINIT_FUNC PyInit_PyTimer(void)
#else
PyMODINIT_FUNC initPyTimer(void)
#endif
{
   PyObject *module = NULL;

#if PY_MAJOR_VERSION > 2
   if ((module = PyModule_Create(&PyTimermodule)) == NULL)
      return NULL;
#else
   if ((module = Py_InitModule("PyTimer", PyTimer_methods)) == NULL)
      return;
#endif

   define_constants(module);


#if PY_MAJOR_VERSION > 2
   return module;
#else
   return;
#endif
}
