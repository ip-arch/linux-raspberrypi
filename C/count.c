// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
// Modified version of Dom and Gert program for the basic operation.
//  How to access GPIO registers from C-code on the Raspberry-Pi
//  Example program

#define BCM2708_PERI_BASE        0x3F000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "7seg.h"

#define BLOCK_SIZE (4*1024)
int  mem_fd;
volatile unsigned *gpio;

int main(int argc, char **argv)
{
  int rep;
  int pat=0;

   /* open /dev/mem */
   if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
      printf("can't open /dev/mem \n");
      exit(-1);
   }
   /* mmap GPIO */
   gpio = (volatile unsigned *)mmap(
      NULL,             //Any adddress in our space will do
      BLOCK_SIZE,       //Map length
      PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
      MAP_SHARED,       //Shared with other processes
      mem_fd,           //File to map
      GPIO_BASE         //Offset to GPIO peripheral
   );

   close(mem_fd); //No need to keep mem_fd open after mmap
   if (gpio == MAP_FAILED) {
      printf("mmap error %d\n", (int)gpio);//errno also set!
      exit(-1);
   }

  // Set gpio direction 0 is input, 1 is output.
  *gpio     = 01110010000;  /* GPSEL0 GPIO  9- 0 */
  *(gpio+1) = 00110110000;  /* GPSEL1 GPIO 19-10 */
  *(gpio+2) = 00000111100;  /* GPSEL2 GPIO 29-20 */

  *(gpio+37)= 2; /* GPPUD enable pull up */
  *(gpio+38)= 0x0000000C; /* GPPUDCLK0 set GPIO 2,3 pull up clock */

  for(rep=0;rep<100;rep++) {

	  *(gpio+10)=pat;  /* GPCLR0 turn off all LEDs */
	  pat = gen_regpat(rep%10);
	  printf("i(%d):pat = %08x\n",rep%10,pat);
	  *(gpio+7)=pat;  /* GPSET0 turn on all LEDs */
	  printf("%08X\n", *(gpio+13));  /* GPLEV0 print the level value */
	  sleep(1);
  }
  return 0;

} // main

