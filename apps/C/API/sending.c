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

	if(argc < 3) errExit("arg1 ipaddr, arg2 strings");
	if((soc = socket(AF_INET, SOCK_DGRAM, 0)) < 0) errExit("socket");
	memset(&sv, 0, sizeof(struct sockaddr_in));
	sv.sin_family = AF_INET;
	sv.sin_port = htons(PORT);
	if(inet_pton(AF_INET, argv[1], &sv.sin_addr)<=0) errExit("inet_pton");
	if(sendto(soc, argv[2], strlen(argv[2]),0,(struct socaddr*)&sv,
		sizeof(struct sockaddr_in)) != strlen(argv[2])) errExit("sendto");
	memset(buf,0,sizeof(buf));
	if(recvfrom(soc, buf, sizeof(buf), 0, NULL, NULL)<0) errExit("recvfrom");
	printf("Recv:%s\n", buf);
	exit(EXIT_SUCCESS);
}
