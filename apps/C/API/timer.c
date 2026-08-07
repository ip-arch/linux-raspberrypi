// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <stdio.h>
#include <signal.h>
#include <time.h>

struct sigevent evp;
timer_t tm;
struct itimerspec itim;

void thread_func(union sigval val) {
	struct timespec tp;
	clock_gettime(CLOCK_REALTIME, &tp);
	printf("Got timer at %s\n", ctime(&tp.tv_sec));
	return;
}

int main () {
	evp.sigev_notify =SIGEV_THREAD;
	evp.sigev_notify_function = thread_func;
	timer_create(CLOCK_REALTIME, &evp, &tm);
	itim.it_value.tv_sec = 2;
	itim.it_value.tv_nsec = 0;
	itim.it_interval.tv_sec = 3;
	itim.it_interval.tv_nsec = 0;
	timer_settime(tm, 0,  &itim, NULL);
	sleep(30);
	return 0;
}
