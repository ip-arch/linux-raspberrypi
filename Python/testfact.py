#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>

import sys
import unittest
import math
import fact


non_interactive = False
for i,val in enumerate(sys.argv):
    if val == '--non_interactive':
        non_interactive = True
        sys.argv.pop(i)


class TestFact(unittest.TestCase):
    def test_over_fact0(self):
        """Test fact(0)"""
        self.assertEqual(fact.fact(0),math.factorial(0))

    def test_over_fact1(self):
        """Test fact(1)"""
        self.assertEqual(fact.fact(1),math.factorial(1))

    def test_over_fact2(self):
        """Test fact(2)"""
        self.assertEqual(fact.fact(2),math.factorial(2))

    def test_over_fact10(self):
        """Test fact(10)"""
        self.assertEqual(fact.fact(10),math.factorial(10))

    def test_over_fact30(self):
        """Test fact(30)"""
        self.assertEqual(fact.fact(30),math.factorial(30))

    def test_over_fact20(self):
        """Test fact(20)"""
        self.assertEqual(fact.fact(20),math.factorial(20))

    def test_over_fact5(self):
        """Test fact(5)"""
        self.assertEqual(fact.fact(5),math.factorial(5))

if __name__ == '__main__':
    unittest.main()
