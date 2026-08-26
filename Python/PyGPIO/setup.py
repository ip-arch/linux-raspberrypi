# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>

from setuptools import setup, Extension

setup(
    name="pygpio",
    version="0.1",
    ext_modules=[
        Extension("pygpio", sources=["pygpio.c"]),
    ],
)
