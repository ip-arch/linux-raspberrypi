// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <stdio.h>
#include <pthread.h>
#define LOOP 500000
int comm, t1, t2, mx;
pthread_mutex_t lock;
void *thread(void *arg) {
	int i;
	for(i=0;i<LOOP;i++) {
		if(mx) pthread_mutex_lock(&lock);
//		*(int* )arg=(*(int*)arg)+1;
		comm++;
		if(mx) pthread_mutex_unlock(&lock);
		t1++;
	}
	return NULL;
}

int main(int argc, char *argv[]) {
	pthread_t t;
	int i;
	if(argc>1) mx=1;
	pthread_mutex_init(&lock, NULL);
	if(pthread_create(&t, NULL, thread, &comm)) {
		return 1;
	}
	for(i=0;i<LOOP;i++) {
		if(mx) pthread_mutex_lock(&lock);
		comm++;
		if(mx) pthread_mutex_unlock(&lock);
		t2++;
	}
	pthread_join(t, NULL);
	pthread_mutex_destroy(&lock);
	printf("comm=%d, t1=%d, t2=%d\n", comm, t1, t2);
	return 0;
}
