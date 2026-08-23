// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
/*
 * exboard_blink.c
 *
 * exboard.dtso の定義に基づき、
 *   - LED1 (ex_led1) を1秒間隔で点滅
 *   - SW1  (ex_sw1)  の状態をコンソールに出力
 *
 * exboard_map.c でDTから読み取った名前->(offset,flags)を使うので、
 * GPIO番号(2, 4, ...)をこのファイルにマジックナンバーとして
 * 持たなくてよい。
 *
 * ioctl周りの構造体・マクロは全て <linux/gpio.h> (カーネルUAPI)
 * のものをそのまま使う。Python版で自前実装していた
 * _IOC/_IOWR や struct レイアウトは不要になる。
 *
 * ビルド例:
 *   $ gcc -O2 -Wall -o exboard_blink exboard_blink.c exboard_map.c
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>

#include "exboard_map.h"

#define CHIP_NUM 0  /* gpiodetect2.py と同じチップ番号。環境に合わせて調整 */

/* offsets[] 内でのインデックス (= line_request内のビット位置) */
enum { IDX_SW1 = 0, IDX_LED1 = 1 };

static volatile sig_atomic_t g_stop;

static void on_sigint(int signo)
{
    (void)signo;
    g_stop = 1;
}

/* config.attrs[attr_index] に id/flags(またはvalues)/mask を設定する */
static void set_line_config_attr(struct gpio_v2_line_config *cfg,
                                  int attr_index, uint32_t id,
                                  uint64_t value, uint64_t mask)
{
    struct gpio_v2_line_config_attribute *a = &cfg->attrs[attr_index];
    a->attr.id = id;
    a->attr.flags = value; /* union: flags/values 共用 */
    a->mask = mask;
}

/* SW1/LED1をまとめてリクエストし、line fdを返す (エラー時は-1) */
static int request_lines(int chip_fd,
                          const struct exboard_gpio_entry *sw1,
                          const struct exboard_gpio_entry *led1)
{
    struct gpio_v2_line_request req;
    memset(&req, 0, sizeof(req));

    req.offsets[IDX_SW1]  = sw1->offset;
    req.offsets[IDX_LED1] = led1->offset;
    req.num_lines = 2;
    snprintf(req.consumer, sizeof(req.consumer), "exboard-blink");

    /* attr0: SW1 = input (+ DTから読んだ極性) */
    uint64_t sw1_flags = GPIO_V2_LINE_FLAG_INPUT;
    if (sw1->dt_flags & EXBOARD_DT_ACTIVE_LOW)
        sw1_flags |= GPIO_V2_LINE_FLAG_ACTIVE_LOW;
    set_line_config_attr(&req.config, 0, GPIO_V2_LINE_ATTR_ID_FLAGS,
                          sw1_flags, 1ULL << IDX_SW1);

    /* attr1: LED1 = output (+ DTから読んだ極性) */
    uint64_t led1_flags = GPIO_V2_LINE_FLAG_OUTPUT;
    if (led1->dt_flags & EXBOARD_DT_ACTIVE_LOW)
        led1_flags |= GPIO_V2_LINE_FLAG_ACTIVE_LOW;
    set_line_config_attr(&req.config, 1, GPIO_V2_LINE_ATTR_ID_FLAGS,
                          led1_flags, 1ULL << IDX_LED1);

    /* attr2: LED1の初期値 = 0 (消灯) */
    set_line_config_attr(&req.config, 2, GPIO_V2_LINE_ATTR_ID_OUTPUT_VALUES,
                          0, 1ULL << IDX_LED1);

    req.config.num_attrs = 3;

    if (ioctl(chip_fd, GPIO_V2_GET_LINE_IOCTL, &req) < 0)
        return -1;

    return req.fd;
}

static int set_led(int line_fd, int on)
{
    struct gpio_v2_line_values vals = {
        .bits = on ? (1ULL << IDX_LED1) : 0,
        .mask = 1ULL << IDX_LED1,
    };
    return ioctl(line_fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &vals);
}

static int read_sw1(int line_fd, int *pressed)
{
    struct gpio_v2_line_values vals = {
        .bits = 0,
        .mask = 1ULL << IDX_SW1,
    };
    int ret = ioctl(line_fd, GPIO_V2_LINE_GET_VALUES_IOCTL, &vals);
    if (ret < 0)
        return ret;
    *pressed = (vals.bits & (1ULL << IDX_SW1)) != 0;
    return 0;
}

int main(void)
{
    struct exboard_gpio_entry entries[EXBOARD_MAX_LINES];
    int n = exboard_load_gpio_map(entries, EXBOARD_MAX_LINES);
    if (n < 0) {
        fprintf(stderr, "exboard_load_gpio_map failed: %s\n", strerror(errno));
        return 1;
    }

    const struct exboard_gpio_entry *sw1 = exboard_find_gpio(entries, n, "ex_sw1");
    const struct exboard_gpio_entry *led1 = exboard_find_gpio(entries, n, "ex_led1");
    if (!sw1 || !led1) {
        fprintf(stderr, "ex_sw1 / ex_led1 が見つかりません\n");
        return 1;
    }

    char chip_path[64];
    snprintf(chip_path, sizeof(chip_path), "/dev/gpiochip%d", CHIP_NUM);

    int chip_fd = open(chip_path, O_RDONLY);
    if (chip_fd < 0) {
        fprintf(stderr, "open(%s) failed: %s\n", chip_path, strerror(errno));
        return 1;
    }

    int line_fd = request_lines(chip_fd, sw1, led1);
    close(chip_fd); /* line_fdが独立して使えるのでchip_fdはもう不要 */

    if (line_fd < 0) {
        fprintf(stderr, "GPIO_V2_GET_LINE_IOCTL failed: %s\n", strerror(errno));
        return 1;
    }

    signal(SIGINT, on_sigint);

    int led_state = 0;
    while (!g_stop) {
        led_state = !led_state;
        if (set_led(line_fd, led_state) < 0) {
            fprintf(stderr, "set_led failed: %s\n", strerror(errno));
            break;
        }

        int pressed = 0;
        if (read_sw1(line_fd, &pressed) < 0) {
            fprintf(stderr, "read_sw1 failed: %s\n", strerror(errno));
            break;
        }

        printf("LED1=%s  SW1=%s\n",
               led_state ? "ON " : "OFF",
               pressed ? "pressed" : "released");
        fflush(stdout);

        sleep(1);
    }

    printf("\n終了します\n");
    set_led(line_fd, 0);
    close(line_fd);
    return 0;
}
