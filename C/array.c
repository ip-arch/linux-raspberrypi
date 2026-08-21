// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <stdio.h>
#include <stdint.h>
#include "read_counter.h"

#define ARRAYSIZE 1000

int A[ARRAYSIZE][ARRAYSIZE], B[ARRAYSIZE][ARRAYSIZE];

int main () {
  uint64_t start_time, end_time;
  int i,j;
  // initialize
  for(i=0; i<ARRAYSIZE; i++)
	  for(j=0; j<ARRAYSIZE; j++)
		  A[i][j] = B[i][j] = i+j;
  // i->j loop
  start_time=read_freerun_counter();
  for(i=0; i<ARRAYSIZE; i++)
	  for(j=0; j<ARRAYSIZE; j++)
		  A[i][j] += B[i][j]+ i+j;
  end_time=read_freerun_counter();
  printf("i->j loop is %lld ticks %3.6fs\n", end_time - start_time, (double)(end_time-start_time)/read_freerun_freq());

  // j->i loop
  start_time=read_freerun_counter();
  for(j=0; j<ARRAYSIZE; j++)
	  for(i=0; i<ARRAYSIZE; i++)
		  A[i][j] += B[i][j]+ i+j;
  end_time=read_freerun_counter();
  printf("j->i loop is %lld ticks %3.6fs\n", end_time - start_time, (double)(end_time-start_time)/read_freerun_freq());
  return 0;
}


