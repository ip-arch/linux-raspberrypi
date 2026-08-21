// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <stdio.h>

int main() {

  int *p=(int*) 0x20000000;

  *p=1;
}
