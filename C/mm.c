// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <stdio.h>
#include <stdlib.h>
int global;
int iglobal=10;
int main(int argc, char*argv) {
  static int x;
  int y, *p;
 p=(int *)malloc(100);

 printf("%p,%p,%p,%p,%p,%p,%p\n",
  &global, &iglobal, main, &argc,
  &x, &p, p);
}

