// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <asm/unistd.h>

int main() {
char *buf="Hello World\n";
register int r0 asm("r0");
register int r1 asm("r1");
register int r2 asm("r2");
register int r7 asm("r7");

	r7 = __NR_write;
	r0 = STDOUT_FILENO;
	r1 = (int)buf;
	r2 = strlen(buf);
	__asm__ volatile(
	"svc #0\n":
	"=r"(r0):"r"(r7),"r"(r0),"r"(r1),"r"(r2));
	printf("\nres=%d\n", r0);
	return 0;
}
	
