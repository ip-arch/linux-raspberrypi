// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#define RPI2
#ifdef RPI2
#define BCM2708_PERI_BASE        0x3F000000
#else
#define BCM2708_PERI_BASE        0x20000000
#endif
#define TIMER_BASE                (BCM2708_PERI_BASE + 0x3000) /* TIMER controller */

#define CPUFREQ		900

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
  unsigned tst,tet;
  if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
      printf("can't open /dev/mem \n");
      exit(-1);
  }

  asm volatile("mcr p15, 0, %0, c9, c12,0\n"::"r"(23));
  asm volatile("mcr p15, 0, %0,c9, c12, 1\n"::"r"(0x8000000f));
  asm volatile("mcr p15, 0, %0,c9, c12, 3\n"::"r"(0x8000000f));

  timer = (volatile unsigned *)mmap(
      NULL,             //Any adddress in our space will do
      BLOCK_SIZE,       //Map length
      PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
      MAP_SHARED,       //Shared with other processes
      mem_fd,           //File to map
      TIMER_BASE         //Offset to GPIO peripheral
  );

  close(mem_fd); //No need to keep mem_fd open after mmap
  if (timer == MAP_FAILED) {
      printf("mmap error %d\n", (int)timer);//errno also set!
      exit(-1);
  }

	  clock_gettime(CLOCK_REALTIME,&st);
	  asm volatile("mcr p15, 0, %0, c9, c12,0\n"::"r"(23));
	  tst = *(timer+1);
	  start=rdtsc32();
	  for(i=0; i<1000; i++) {
		  volatile static int x;
		  clock_gettime(CLOCK_REALTIME,&tmp);
		  x+=tmp.tv_nsec;
	  }
	  end=rdtsc32();
	  tet = *(timer+1);
	  clock_gettime(CLOCK_REALTIME,&et);
	  printf("sysc:%d,%d,%d,%f,%d\n", i, (end-start)/CPUFREQ, tet-tst, ((et.tv_sec-st.tv_sec)+(et.tv_nsec-st.tv_nsec)*1e-9),end);
	  clock_gettime(CLOCK_REALTIME,&st);
	  tst = *(timer+1);
	  start=rdtsc32();
	  for(i=0; i<1000; i++) {
		  volatile static int x;
		  x+=*(timer+1);
	  }
	  end=rdtsc32();
	  tet = *(timer+1);
	  clock_gettime(CLOCK_REALTIME,&et);
	  printf("mmap:%d,%d,%d,%f,%d\n", i, (end-start)/CPUFREQ, tet-tst, ((et.tv_sec-st.tv_sec)+(et.tv_nsec-st.tv_nsec)*1e-9),end);

  return 0;
}
