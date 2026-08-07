// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <stdio.h>
#include <time.h>
struct timespec tp;

int main () {
	clock_gettime(CLOCK_REALTIME, &tp);
	printf("ctime = %s\n", ctime(&tp.tv_sec));
	clock_getres(CLOCK_REALTIME, &tp);
	printf("clock resolution is %dnS\n", tp.tv_nsec);
	return 0;
}
