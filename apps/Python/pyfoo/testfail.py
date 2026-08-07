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
import unittest
import pyfoo

non_interactive = False
for i,val in enumerate(sys.argv):
    if val == '--non_interactive':
        non_interactive = True
        sys.argv.pop(i)


class TestFoo(unittest.TestCase):

    def test_foo1(self):
        """Test foo with 1 argument"""
        self.assertEqual(pyfoo.foo(400), (401.0,-1.0))

    def test_foo2(self):
        """Test foo with 2 argument"""
        self.assertEqual(pyfoo.foo(3,5), (3.0,5.0))

    def test_fook1(self):
        """Test foo with 1 keyword argument"""
        self.assertEqual(pyfoo.foo(arg=3), (3.0,-1.0))

    def test_fook2(self):
        """Test foo with 2 keyword argument"""
        self.assertEqual(pyfoo.foo(oparg=10, arg=3), (3.0,10.0))

    def test_goo0(self):
        """Test goo(0)"""
        self.assertEqual(pyfoo.goo(0), 'arg=1')

    def test_over_goo1(self):
        """Test goo(1)"""
        with self.assertRaises(ValueError):
          pyfoo.goo(1)

    def test_over_goo2(self):
        """Test goo(2)"""
        with self.assertRaises(ValueError):
          pyfoo.goo(2)

    def test_over_goo3(self):
        """Test goo(3)"""
        with self.assertRaises(BaseException):
          pyfoo.goo(3)

    def test_over_goo4(self):
        """Test goo(4)"""
        with self.assertRaises(SystemError):
          pyfoo.goo(4)

    def test_over_goo5(self):
        """Test goo(5)"""
        with self.assertRaises(OSError):
          pyfoo.goo(5)

    def test_over_goo6(self):
        """Test goo(6)"""
        self.assertIsNone(pyfoo.goo(6))

if __name__ == '__main__':
    unittest.main()
