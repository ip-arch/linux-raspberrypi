# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>

import PyTimer
import time
def cb(x):
  print('call by '+str(x))


t0=PyTimer.create(PyTimer.CLOCK_REALTIME, cb)
t1=PyTimer.create(PyTimer.CLOCK_REALTIME, cb)

PyTimer.settime(t0, 5, 1)
PyTimer.settime(t1, 3, 1)

time.sleep(20)
PyTimer.delete(t0)
PyTimer.delete(t1)
