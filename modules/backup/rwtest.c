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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char**argv) {
  int fd, i, buffer, sz;
  fd=open("/dev/Mmap",O_RDWR);
  if(argc>1) {
	  buffer=atoi(argv[1]);
	  sz=write(fd, &buffer, sizeof(int));
	  if(sz!=4) {printf("not 4 byte sz %d\n",sz); return(1);}
  }
  sz=read(fd, &buffer, sizeof(int));
  if(sz!=4) {printf("not 4 byte sz %d\n",sz); return(1);}
  printf("Read Data = %d\n",buffer);
  return(0);

}
