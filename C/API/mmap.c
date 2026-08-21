// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>

#define MAPFILE "mmaptest.txt"
#define BUFSZ	4096
int main(int argc, char *argv[]) {
	int i,fd;
	char *buf, *bufo;
	fd = open(MAPFILE, O_RDWR|O_CREAT);
	if(fd<0) return 1;
	buf=(char*)mmap(NULL, BUFSZ, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	bufo=buf;
	for(i=0;i<argc;i++){
		sprintf(buf,"%s\n",argv[i]);
		buf+=strlen(buf);
	}
	msync(bufo, BUFSZ, MS_SYNC);
	return 0;
}
