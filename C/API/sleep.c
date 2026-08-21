// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <stdio.h>
#include <time.h>
struct timespec req, rem, tps, tpe;

int main () {
	req.tv_sec = 2;
	req.tv_nsec = 500000000;
	clock_gettime(CLOCK_REALTIME, &tps);
	nanosleep(&req, &rem);
	clock_gettime(CLOCK_REALTIME, &tpe);
	printf("Start to sleep at %s\n", ctime(&tps.tv_sec));
	printf("End to sleep at %s\n", ctime(&tpe.tv_sec));
	printf("Diff = %f\n", (double)(tpe.tv_sec+tpe.tv_nsec*1e-9) -
			(double)(tps.tv_sec+tps.tv_nsec*1e-9));
	return 0;
}

