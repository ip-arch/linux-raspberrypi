// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#define PORT 55123
void errExit(char str[]) {
	fprintf(stderr,"Error with %s\n",str);
	exit(-1);
}
int main(int argc, char *argv[]) {
	struct sockaddr_in sv, cl;
	int soc;
	char buf[100];

	if((soc = socket(AF_INET, SOCK_DGRAM, 0)) < 0) errExit("socket");
	memset(&sv, 0, sizeof(sv));
	sv.sin_family = AF_INET;
	sv.sin_addr.s_addr = INADDR_ANY;
	sv.sin_port = htons(PORT);

	if(bind(soc, (struct sockaddr *)&sv, sizeof(sv)) < 0) errExit("bind");
	while (1) {
		int i;
		socklen_t len = sizeof(struct sockaddr_in);
		ssize_t sz;
		if((sz = recvfrom(soc, buf, sizeof(buf), 0,
			(struct sockaddr *)&cl, &len)) < 0) errExit("recvfrom");
		for(i=0;i<sz;i++)
			buf[i] = (buf[i]>='a' && buf[i] <='z')? buf[i]-0x20:buf[i]+0x20;
		buf[i]=0;
		if(sendto(soc, buf, sz, 0, (struct sockaddr*)&cl, len) != sz) errExit("sendto");
	}
}

