// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
/*
 * exboard_map.c
 *
 * /sys/devices/platform/exboard/of_node/ 以下の生プロパティを読み、
 * 名前 <-> (offset, flags) の対応表を組み立てる。
 *
 * *-gpios プロパティは <phandle, pin, flags> の3セル(uint32, big-endian)
 * の繰り返し (#gpio-cells = 2 を想定)。
 * gpio-line-names は NUL区切りの文字列配列。
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>   /* ntohl() */

#include "exboard_map.h"

/* exboard.dtso に書かれている *-gpios プロパティの並び順
 * (gpio-line-names の並びともこの順で対応している前提) */
static const char *gpio_props[] = {
    "ex_sw-gpios",
    "ex_led-gpios",
    "ex_disp-gpios",
    "ex_seg-gpios",
};
static const int n_gpio_props = sizeof(gpio_props) / sizeof(gpio_props[0]);

/* 1プロパティファイルを読み、<phandle,pin,flags>の並びを追加していく。
 * 戻り値: 追加できたエントリ数。エラー時は -1。 */
static int append_gpios_prop(const char *prop_name,
                              struct exboard_gpio_entry *entries,
                              int start, int max_entries)
{
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", EXBOARD_OF_NODE, prop_name);

    FILE *f = fopen(path, "rb");
    if (!f) {
        /* このボードに存在しないプロパティは無視してよい */
        if (errno == ENOENT)
            return 0;
        return -1;
    }

    int idx = start;
    uint32_t cell[3]; /* phandle, pin, flags (#gpio-cells = 2) */

    while (idx < max_entries &&
           fread(cell, sizeof(uint32_t), 3, f) == 3) {
        entries[idx].phandle  = ntohl(cell[0]);
        entries[idx].offset   = ntohl(cell[1]);
        entries[idx].dt_flags = ntohl(cell[2]);
        idx++;
    }

    fclose(f);
    return idx - start;
}

/* gpio-line-names (NUL区切り文字列の配列) を読み、
 * entries[]のnameフィールドへ順番に書き込む。
 * 戻り値: 読めた名前の数。エラー時は -1。 */
static int fill_line_names(struct exboard_gpio_entry *entries, int n_entries)
{
    char path[256];
    snprintf(path, sizeof(path), "%s/gpio-line-names", EXBOARD_OF_NODE);

    FILE *f = fopen(path, "rb");
    if (!f)
        return -1;

    char buf[4096];
    size_t len = fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);
    buf[len] = '\0';

    int idx = 0;
    size_t pos = 0;
    while (pos < len && idx < n_entries) {
        const char *s = buf + pos;
        size_t slen = strnlen(s, len - pos);
        if (slen > 0) {
            snprintf(entries[idx].name, EXBOARD_NAME_MAX, "%s", s);
            idx++;
        }
        pos += slen + 1; /* NUL文字の次へ */
    }

    return idx;
}

int exboard_load_gpio_map(struct exboard_gpio_entry *entries, int max_entries)
{
    memset(entries, 0, sizeof(*entries) * max_entries);

    int n = 0;
    for (int i = 0; i < n_gpio_props; i++) {
        int added = append_gpios_prop(gpio_props[i], entries, n, max_entries);
        if (added < 0)
            return -1;
        n += added;
    }

    int n_names = fill_line_names(entries, n);
    if (n_names < 0)
        return -1;

    if (n_names != n) {
        fprintf(stderr,
                "[exboard_map] warning: gpio-line-names(%d) と "
                "*-gpios合計(%d) の数が一致していません\n",
                n_names, n);
    }

    return n;
}

const struct exboard_gpio_entry *exboard_find_gpio(
    const struct exboard_gpio_entry *entries, int n, const char *name)
{
    for (int i = 0; i < n; i++) {
        if (strncmp(entries[i].name, name, EXBOARD_NAME_MAX) == 0)
            return &entries[i];
    }
    return NULL;
}
