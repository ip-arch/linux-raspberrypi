// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int fd, value;
  fd = open("/dev/LED", O_RDWR|O_SYNC);
  if(argc>1) {
     value=atoi(argv[1]);
     write(fd, &value,4);
  }
  else {
     read(fd, &value, 4);
     printf("value - %d\n", value);
  }
  return 0;
}
