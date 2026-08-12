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


int main () {
  uint32_t start, end;
  volatile int *p,x;

	  p=(int*)mmap(NULL,100,PROT_READ|PROT_WRITE,
		MAP_ANONYMOUS|MAP_PRIVATE,0,0);
	  start=read_freerun_counter();
	  x=*p;
	  end=read_freerun_counter();
	  printf("1st READ: clock:%d, %es\n", (end-start), (double)(end-start)/CPUFREQ);
	  start=read_freerun_counter();
	  x=*p;
	  end=read_freerun_counter();
	  printf("2nd READ: clock:%d, %es\n", (end-start), (double)(end-start)/CPUFREQ);

  return 0;
}
