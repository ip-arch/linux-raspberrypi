# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>

from multiprocessing import Process, Value, Array
import m2

num = Value('i',0)
arr = Array('i', range(10))

m2.foo(num,arr)

print(num.value)
print(arr[:])
quit()

