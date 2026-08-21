// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <stdio.h>
#include <signal.h>
#include <time.h>

struct sigaction sa;
struct timespec tp;

void sig_handler() {
	clock_gettime(CLOCK_REALTIME, &tp);
	printf("got a signal\n");
	printf("ctime = %s\n", ctime(&tp.tv_sec));
	return;
}
int main () {
	sa.sa_handler = sig_handler;
	sigaction(SIGUSR1,&sa,NULL);
	while (1) sleep(0);
	return 0;
}
