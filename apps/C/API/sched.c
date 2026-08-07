// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <sched.h>
#include <stdio.h>

int main (int argc, char *argv[]) {
	int count;
	if(argc < 2) return -1;
	struct sched_param sp = {10};
	
	sched_setscheduler(0, SCHED_RR, &sp);
 	printf("schedule = %d\n", sched_getscheduler(0));
	for(count=0;count<atoi(argv[1]);count++) {
		if(!(count%65536)) printf("C:%d\n", count);
	}
	return 0;
}
