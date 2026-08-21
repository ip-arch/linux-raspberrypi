// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <stdio.h>
#include <sys/mman.h>

int main() {

  int *p;

  p=(int*)mmap((void*)0x20000000, 0x1000, PROT_WRITE|PROT_READ, MAP_ANONYMOUS|MAP_PRIVATE, 0, 0);

  *p=55;
  printf("%p:%d\n", p, *p);
  return 0;
}
