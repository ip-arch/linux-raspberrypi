// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <linux/module.h>
#include <linux/kernel.h>

static void enable_ccnt_read(void* data)
{
  // WRITE PMUSERENR = 1
  asm volatile ("mcr p15, 0, %0, c9, c14, 0\n\t" : : "r" (1));
  // PMCR.E = 1
  asm volatile ("mcr p15, 0, %0, c9, c12, 0\n\t" : : "r" (1));
  // PMCNTENSET.C = 1
  asm volatile ("mcr p15, 0, %0, c9, c12, 1\n\t" : : "r" (1<<31));
}
static void disable_ccnt_read(void* data)
{
  // WRITE PMUSERENR = 1
  asm volatile ("mcr p15, 0, %0, c9, c14, 0\n\t" : : "r" (0));
  // PMCR.E = 1
  asm volatile ("mcr p15, 0, %0, c9, c12, 0\n\t" : : "r" (0));
  // PMCNTENSET.C = 1
  asm volatile ("mcr p15, 0, %0, c9, c12, 1\n\t" : : "r" (0<<31));
}

int init_module()
{
  on_each_cpu(enable_ccnt_read, NULL, 1);
  return 0;
}

void cleanup_module()
{
  on_each_cpu(disable_ccnt_read, NULL, 1);
}

MODULE_LICENSE("GPL");

