#!/usr/bin/env python2.7
"""
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
"""

import sys
import time
import unittest
import PyTimer

non_interactive = False
for i,val in enumerate(sys.argv):
    if val == '--non_interactive':
        non_interactive = True
        sys.argv.pop(i)


class TestIntervalTimer(unittest.TestCase):

    def test_create_simple(self):
        """Test create and gettime and delete"""
        t0 = PyTimer.create(PyTimer.CLOCK_REALTIME)
        self.assertEqual(PyTimer.gettime(t0), (0.,0.))
        PyTimer.delete(t0)

    def test_create_type(self):
        """Test illeagal argument for create"""
        with self.assertRaises(ValueError):
            t0 = PyTimer.create(0)

    def test_over_delete(self):
        """Test delete already deleted timer"""
        t0 = PyTimer.create(PyTimer.CLOCK_REALTIME)
        PyTimer.delete(t0)
        with self.assertRaises(RuntimeError):
          PyTimer.delete(t0)

    def test_delete_2nd(self):
        """Test create and gettime and delete"""
        t0 = PyTimer.create(PyTimer.CLOCK_REALTIME)
        t1 = PyTimer.create(PyTimer.CLOCK_REALTIME)
        PyTimer.delete(t0)
        self.assertEqual(PyTimer.gettime(t1), (0.,0.))
        PyTimer.delete(t1)

    def test_delete_2nd_a(self):
        """Test create and gettime and delete"""
        t0 = PyTimer.create(PyTimer.CLOCK_REALTIME)
        t1 = PyTimer.create(PyTimer.CLOCK_REALTIME)
        t2 = PyTimer.create(PyTimer.CLOCK_REALTIME)
        PyTimer.delete(t0)
        self.assertEqual(PyTimer.gettime(t1), (0.,0.))
        self.assertEqual(PyTimer.gettime(t2), (0.,0.))
        PyTimer.delete(t1)
        self.assertEqual(PyTimer.gettime(t2), (0.,0.))
        PyTimer.delete(t2)

    def test_delete_2nd_aa(self):
        """Test create and gettime and delete"""
        t0 = PyTimer.create(PyTimer.CLOCK_REALTIME)
        t1 = PyTimer.create(PyTimer.CLOCK_REALTIME)
        t2 = PyTimer.create(PyTimer.CLOCK_REALTIME)
        PyTimer.delete(t0)
        self.assertEqual(PyTimer.gettime(t1), (0.,0.))
        self.assertEqual(PyTimer.gettime(t2), (0.,0.))
        PyTimer.delete(t2)
        self.assertEqual(PyTimer.gettime(t1), (0.,0.))
        PyTimer.delete(t1)

    def test_delete_2nd_b(self):
        """Test create and gettime and delete"""
        t0 = PyTimer.create(PyTimer.CLOCK_REALTIME)
        t1 = PyTimer.create(PyTimer.CLOCK_REALTIME)
        t2 = PyTimer.create(PyTimer.CLOCK_REALTIME)
        PyTimer.delete(t1)
        self.assertEqual(PyTimer.gettime(t0), (0.,0.))
        self.assertEqual(PyTimer.gettime(t2), (0.,0.))
        PyTimer.delete(t0)
        self.assertEqual(PyTimer.gettime(t2), (0.,0.))
        PyTimer.delete(t2)

    def test_delete_2nd_ba(self):
        """Test create and gettime and delete"""
        t0 = PyTimer.create(PyTimer.CLOCK_REALTIME)
        t1 = PyTimer.create(PyTimer.CLOCK_REALTIME)
        t2 = PyTimer.create(PyTimer.CLOCK_REALTIME)
        PyTimer.delete(t1)
        self.assertEqual(PyTimer.gettime(t0), (0.,0.))
        self.assertEqual(PyTimer.gettime(t2), (0.,0.))
        PyTimer.delete(t2)
        self.assertEqual(PyTimer.gettime(t0), (0.,0.))
        PyTimer.delete(t0)

    def test_delete_2nd_c(self):
        """Test create and gettime and delete"""
        t0 = PyTimer.create(PyTimer.CLOCK_REALTIME)
        t1 = PyTimer.create(PyTimer.CLOCK_REALTIME)
        t2 = PyTimer.create(PyTimer.CLOCK_REALTIME)
        PyTimer.delete(t2)
        self.assertEqual(PyTimer.gettime(t0), (0.,0.))
        self.assertEqual(PyTimer.gettime(t1), (0.,0.))
        PyTimer.delete(t0)
        self.assertEqual(PyTimer.gettime(t1), (0.,0.))
        PyTimer.delete(t1)

    def test_delete_2nd_ca(self):
        """Test create and gettime and delete"""
        t0 = PyTimer.create(PyTimer.CLOCK_REALTIME)
        t1 = PyTimer.create(PyTimer.CLOCK_REALTIME)
        t2 = PyTimer.create(PyTimer.CLOCK_REALTIME)
        PyTimer.delete(t2)
        self.assertEqual(PyTimer.gettime(t0), (0.,0.))
        self.assertEqual(PyTimer.gettime(t1), (0.,0.))
        PyTimer.delete(t1)
        self.assertEqual(PyTimer.gettime(t0), (0.,0.))
        PyTimer.delete(t0)

    def test_settime_0(self):
        """Test settime and gettime and delete"""
        t0 = PyTimer.create(PyTimer.CLOCK_REALTIME)
        PERIOD = 1000.
        now = time.time()
        PyTimer.settime(t0, PERIOD)
        time.sleep(2)
        diff = abs((time.time() - now) - (PERIOD - PyTimer.gettime(t0)[0]))
        self.assertTrue(diff < 0.1)
        PyTimer.delete(t0)

    def test_settime_1(self):
        """Test settime and gettime and delete"""
        self.count=0
        def cb(x):
                self.count = self.count +1
        t0 = PyTimer.create(PyTimer.CLOCK_REALTIME, cb)
        PERIOD = 1.
        INTERVAL = 1.
        WAIT = 10
        now = time.time()
        PyTimer.settime(t0, PERIOD, INTERVAL)
        time.sleep(WAIT)
        diff = abs(int(time.time() - now + 0.5) - self.count)
        self.assertTrue(diff < 2)
        PyTimer.delete(t0)

    def test_settime_2(self):
        """Test settime and gettime and delete"""
        t0 = PyTimer.create(PyTimer.CLOCK_REALTIME)
        PERIOD = 1000.
        PyTimer.delete(t0)
        with self.assertRaises(RuntimeError):
            PyTimer.settime(t0, PERIOD)

if __name__ == '__main__':
    unittest.main()
