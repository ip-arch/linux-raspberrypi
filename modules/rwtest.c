// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
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
