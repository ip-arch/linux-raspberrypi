// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <stdio.h>
#include <signal.h>

struct sigaction sa;

void sig_handler(int sig, siginfo_t *si, void *context) {
	printf("got a signal %d(%d)\n", sig, si->si_value.sival_int);
	return;
}
int main () {
	int sig;
	sa.sa_handler = sig_handler;
	sa.sa_flags   = SA_SIGINFO;
	for(sig=SIGRTMIN;sig<SIGRTMAX;sig++) sigaction(sig, &sa, NULL);
	while (1) sleep(0);
	return 0;
}
