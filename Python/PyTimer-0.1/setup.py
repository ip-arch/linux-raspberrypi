# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>

from distutils.core import setup, Extension

classifiers = ['Development Status :: 5 - Production/Stable',
               'Operating System :: POSIX :: Linux',
               'License :: OSI Approved :: MIT License',
               'Intended Audience :: Developers',
               'Programming Language :: Python :: 2.7',
               'Programming Language :: Python :: 3',
               'Topic :: Software Development',
               'Topic :: Home Automation',
               'Topic :: System :: Hardware']
module = Extension ('PyTimer',
		libraries = ['rt'],
		sources = ['PyTimer/py_timer.c',  'PyTimer/constants.c'])

setup(name             = 'PyTimer',
      version          = '0.1',
      author           = 'Naohiko Shimizu',
      author_email     = 'nshimizu@ip-arch.jp',
      description      = 'A module to control Interval Timer',
      long_description = open('README.txt').read() + open('CHANGELOG.txt').read(),
      license          = 'MIT',
      keywords         = 'Interval Timer',
      package_data     = {'': ['CHANGELOG.txt', 'MANIFEST.ini', 'README.txt', 'LICENSE.txt', 'INSTALL.txt']}, 
      include_package_data = True,
      url              = 'http://www.ip-arch.jp',
      classifiers      = classifiers,
      packages         = ['PyTimer'],
      ext_modules      = [module])

