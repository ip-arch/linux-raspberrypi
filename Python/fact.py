#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>

def fact(x):
	if(x==0):
		return 1
	else:
		return x*fact(x-1)

