// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#define CPUFREQ		read_freerun_freq()

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>

#include "../read_counter.h"
#ifndef START
#define START 0x20001000
#define LENGTH 0x1000
#endif

int main() {

 char *area1, *area2, *area3;
 uint32_t st,et;

  area1=(char *)mmap((void *)0x20000000,0x1000,PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);
  area2=(char *)mmap((void *)0x20004000,0x2000,PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);

 st=read_freerun_counter();
  area3=(char *)mmap((void *)START,LENGTH,PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);
 et=read_freerun_counter();

 printf("mmap time %e s\n", (double)(et-st)/(CPUFREQ));
 printf("area1=%p, area2=%p, area3=%p\n",area1,area2,area3);

 return 0;
}
