// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <stdio.h>
#include <fcntl.h>
int main(int argc, char *argv[]) {
  int led;
  int fd;
  int i;
  if(argc<3) {
	printf("Usage sudo ./device_ua.exe device_file value\n");
	return 1;
  }
  fd=open(argv[1],O_RDWR);
  if(fd<0) return 1;
  read(fd,&led,4);
  printf("current LED=%d\n",led);
  led=atoi(argv[2]);
  for(i=0;i<10;i++) {
	write(fd,&led,4);
	sleep(1);
	led=1-led;
  }
  return 0;
}
