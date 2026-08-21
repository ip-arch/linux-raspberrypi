// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <stdio.h>
#include <signal.h>

int main(int argc, char *argv[]) {
	union sigval sv;
	if(argc < 4) {
		printf("%s pid sig dat\n",argv[0]);
		return -1;
	}
	sv.sival_int = atoi(argv[3]);
	sigqueue(atoi(argv[1]), atoi(argv[2]), sv);
	return 0;
}
