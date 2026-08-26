# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>

import threading

def hello():
    t = threading.Timer(1.0, hello)
    t.start()
    print("hello, world") 
    quit()
t = threading.Timer(1.0, hello)
t.start() 
