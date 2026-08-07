/*
 *   This sample source codes are design for a seminar.
 *
 *   Copyright (c) 2005-2015 Naohiko Shimizu, All Rights Reserved.
 *   Author: Naohiko Shimizu : nshimizu@ip-arch.jp
 *
 *   Everyone is permitted to copy and distribute verbatim copies of
 *   this source code under GPL Version 2.
 */
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, char**argv) {
  int fd, i;
  int *buffer;
  fd=open("/dev/Mmap",O_RDWR);
  printf("open fd(%d)\n",fd);
  if(argc>1) {
	  buffer=(int*)mmap(NULL,0x1000,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	  printf("buffer address=%p\n",buffer);
	  for(i=1;i<argc;i++)  {
		buffer[i-1]=atoi(argv[i]);
	  }
  }
  else 
	  buffer=(int*)mmap(NULL,0x1000,PROT_READ,MAP_PRIVATE,fd,0);
  msync(buffer,0x1000,MS_SYNC);
  for(i=0;i<10;i++) {
	printf("buff[%d]=%08x\n", i, buffer[i]);
  }
  return(0);
}
