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

int main () {
  uint32_t start, end;
  volatile int *p,x;

  asm volatile("mcr p15, 0, %0, c9, c12,0\n"::"r"(23));
  asm volatile("mcr p15, 0, %0,c9, c12, 1\n"::"r"(0x8000000f));
  asm volatile("mcr p15, 0, %0,c9, c12, 3\n"::"r"(0x8000000f));

	  asm volatile("mcr p15, 0, %0, c9, c12,0\n"::"r"(23));
	  p=(int*)mmap(NULL,100,PROT_READ|PROT_WRITE,
		MAP_ANONYMOUS|MAP_PRIVATE,0,0);
	  start=rdtsc32();
	  x=*p;
	  end=rdtsc32();
	  printf("1st READ: clock:%d, %fuS\n", (end-start), (double)(end-start)/CPUFREQ);
	  start=rdtsc32();
	  x=*p;
	  end=rdtsc32();
	  printf("2nd READ: clock:%d, %fuS\n", (end-start), (double)(end-start)/CPUFREQ);

  return 0;
}
