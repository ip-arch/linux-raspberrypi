// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <stdio.h>
#include <time.h>
struct timespec tp;

int main () {
	long start,end;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	start = tp.tv_nsec;
	//printf("ctime = %s\n", ctime(&tp.tv_sec));
	printf("Hello World\n");
	//usleep(100);
	clock_gettime(CLOCK_MONOTONIC, &tp);
	end = tp.tv_nsec;
	printf("clock diff is %dnS\n", end - start);
	printf("ctime = %s\n", ctime(&tp.tv_sec));
	return 0;
}
