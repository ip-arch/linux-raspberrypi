// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
/*
 * exboard_map.h
 *
 * exboard.dtso の *-gpios プロパティと gpio-line-names を
 * /sys/devices/platform/exboard/of_node から読み取り、
 * "名前" -> (chip offset, DT flags) の対応表を作る。
 *
 * exboard_gpio_map.py / exboard_map.py (Python版) のC言語版。
 */
#ifndef EXBOARD_MAP_H
#define EXBOARD_MAP_H

#include <stdint.h>

#define EXBOARD_OF_NODE   "/sys/devices/platform/exboard/of_node"
#define EXBOARD_NAME_MAX  32
#define EXBOARD_MAX_LINES 32

/* dt-bindings/gpio/gpio.h の GPIO_ACTIVE_LOW と同じ意味の値(セルそのもの) */
#define EXBOARD_DT_ACTIVE_LOW 1

struct exboard_gpio_entry {
    char     name[EXBOARD_NAME_MAX]; /* gpio-line-names 由来 */
    uint32_t phandle;                /* 参照先ノードのphandle (通常は全エントリ同一) */
    uint32_t offset;                 /* GPIOコントローラ上のline offset */
    uint32_t dt_flags;               /* DTのgpio-specifier内のflagsセル (0/1) */
};

/*
 * exboard.dtso由来のGPIOマップを読み込む。
 *
 * entries: 呼び出し側が確保した配列
 * max_entries: entries[]の要素数上限
 *
 * 戻り値: 読み込んだエントリ数 (>=0)、エラー時は -1 (errnoを設定)
 */
int exboard_load_gpio_map(struct exboard_gpio_entry *entries, int max_entries);

/*
 * 名前で1件検索する。見つからなければ NULL。
 */
const struct exboard_gpio_entry *exboard_find_gpio(
    const struct exboard_gpio_entry *entries, int n, const char *name);

#endif /* EXBOARD_MAP_H */
