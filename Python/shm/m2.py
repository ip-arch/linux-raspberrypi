# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>

def foo(m,a):
  m.value = 5
  for i in range(len(a)):
     a[i]= - a[i]

