// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
// Modified version of Dom and Gert program for the basic operation.
//  How to access GPIO registers from C-code on the Raspberry-Pi
//  Example program

#define RPI2
#ifdef RPI2
#define BCM2708_PERI_BASE        0x3F000000
#else
#define BCM2708_PERI_BASE        0x20000000
#endif
#define TIMER_BASE                (BCM2708_PERI_BASE + 0x3000) /* TIMER controller */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define BLOCK_SIZE (4*1024)
int  mem_fd;
volatile unsigned *timer;

int main(int argc, char **argv)
{
  int rep;
  int timer_val;

   /* open /dev/mem */
   if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
      printf("can't open /dev/mem \n");
      exit(-1);
   }

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


  for(rep=0;rep<100;rep++) {
	  timer_val =  *(timer+1);
	  sleep(1);
          printf("sleep = %f\n", (double)(*(timer+1)-timer_val)/1e6);
	  timer_val =  *(timer+1);
	  sleep(1);
          printf("sleep = %f\n", (double)(*(timer+1)-timer_val)/1e6);
  }
  return 0;

} // main

