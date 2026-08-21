// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
// THIS PROGRAM DOES NOT WORK WITH ILLEAGAL INSTRUCTION!
#define RPI2
#ifdef RPI2
#define BCM2708_PERI_BASE        0x3F000000
#else
#define BCM2708_PERI_BASE        0x20000000
#endif
#define TIMER_BASE                (BCM2708_PERI_BASE + 0x3000) /* TIMER controller */

#define CPUFREQ		900.

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#define BLOCK_SIZE (4*1024)



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
  int  mem_fd;
  volatile unsigned *timer;
  static int i=0;
  uint32_t start, end;
  struct timespec st,et,tmp;

  asm volatile ("mcr p15, 0, %0, c9, c14, 0\n\t" : : "r" (1));
  asm volatile ("mcr p15, 0, %0, c9, c12, 0\n\t" : : "r" (1));
  asm volatile ("mcr p15, 0, %0, c9, c12, 1\n\t" : : "r" (1<<31));

  asm volatile("mcr p15, 0, %0, c9, c12,0\n"::"r"(23));
  asm volatile("mcr p15, 0, %0,c9, c12, 1\n"::"r"(0x8000000f));
  asm volatile("mcr p15, 0, %0,c9, c12, 3\n"::"r"(0x8000000f));


	  clock_gettime(CLOCK_REALTIME,&st);
	  asm volatile("mcr p15, 0, %0, c9, c12,0\n"::"r"(23));
	  start=rdtsc32();
	  sleep(1);
	  end=rdtsc32();
	  clock_gettime(CLOCK_REALTIME,&et);
	  printf("sysc:%f,%f\n", (end-start)/CPUFREQ,  ((et.tv_sec-st.tv_sec)+(et.tv_nsec-st.tv_nsec)*1e-9));

  return 0;
}
