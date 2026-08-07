// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
void gotalarm() {
 printf("Got SIGALARM\n");
 return;
}
int main() {
 struct sigaction act;
 struct itimerval itval;
 int i;
 memset(&act, 0, sizeof(act));
 act.sa_handler = gotalarm;
 sigaction(SIGALRM, &act, NULL);
 memset(&itval, 0, sizeof(itval));
 itval.it_value.tv_sec=2; itval.it_value.tv_usec=500000;
 itval.it_interval.tv_sec=1; itval.it_interval.tv_usec=500000;
 setitimer(ITIMER_REAL, &itval, NULL);
 for(i=0;i<10;i++) {
   printf("%d sec\n",i);
   sleep(1);
  }
 printf("finished\n");
}

