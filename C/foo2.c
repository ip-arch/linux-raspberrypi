// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
int foo2(int arg) {
  if(arg<2) return arg;
  return foo2(arg-1)+foo2(arg-2);
}
