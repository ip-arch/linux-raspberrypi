// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#define CPUFREQ		900.0

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>

static inline uint32_t
rdtsc32(void)
{
  volatile uint32_t pmccntr;
  // Read the user mode perf monitor counter access permissions.
      asm volatile("mrc p15, 0, %0, c9, c13, 0" : "=r"(pmccntr));
//      asm volatile("mrc p15, 0, %0, c15, c12, 1" : "=r"(pmccntr));
      return pmccntr;  
}

#ifndef START
#define START 0x20001000
#define LENGTH 0x1000
#endif

int main() {

 char *area1, *area2, *area3;
 uint32_t st,et;

  area1=(char *)mmap((void *)0x20000000,0x1000,PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);
  area2=(char *)mmap((void *)0x20004000,0x2000,PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);

 st=rdtsc32();
  area3=(char *)mmap((void *)START,LENGTH,PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);
 et=rdtsc32();

 printf("mmap time %f uS\n", (double)(et-st)/(CPUFREQ));
 printf("area1=%p, area2=%p, area3=%p\n",area1,area2,area3);

 return 0;
}
