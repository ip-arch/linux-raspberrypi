// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
/*
 * exboard_blink_epoll.c
 *
 * exboard.dtso の定義に基づき、
 *   - LED1 (ex_led1) を周期タイマー(timerfd)で1秒ごとに点滅
 *   - SW1  (ex_sw1)  の立ち上がりエッジ(押下)をline fd自体の
 *     読み出しイベントとして検出し、そのときだけメッセージを出す
 *
 * ポーリングをやめ、timerfdとGPIOのline fdをepollで同時に監視する
 * イベント駆動モデルに変更。SW1のチャタリング除去はカーネル側の
 * デバウンス機能(GPIO_V2_LINE_ATTR_ID_DEBOUNCE)を使い、時間は
 * SW_DEBOUNCE_US で変更できる。
 *
 * ビルド例:
 *   $ gcc -O2 -Wall -o exboard_blink_epoll exboard_blink_epoll.c exboard_map.c
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
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <linux/gpio.h>

#include "exboard_map.h"

#define CHIP_NUM 0  /* gpiodetect2.py と同じチップ番号。環境に合わせて調整 */

/* チャタリング除去時間 [us]。カーネル/ドライバがデバウンスに
 * 対応していない場合、GPIO_V2_GET_LINE_IOCTL が -EINVAL で
 * 失敗することがある(その場合は0にするか、EDGE検出側で
 * ソフトウェア的にデバウンスする必要がある)。 */
#define SW_DEBOUNCE_US 10000  /* 10ms */

#define BLINK_PERIOD_SEC 1

/* offsets[] 内でのインデックス (= line_request内のビット位置) */
enum { IDX_SW1 = 0, IDX_LED1 = 1 };

static volatile sig_atomic_t g_stop;

static void on_sigint(int signo)
{
    (void)signo;
    g_stop = 1;
}

static void set_line_config_attr(struct gpio_v2_line_config *cfg,
                                  int attr_index, uint32_t id,
                                  uint64_t value, uint64_t mask)
{
    struct gpio_v2_line_config_attribute *a = &cfg->attrs[attr_index];
    a->attr.id = id;
    a->attr.flags = value; /* union: flags/values/debounce_period_us 共用 */
    a->mask = mask;
}

/*
 * SW1(入力・エッジ検出・デバウンス付き)とLED1(出力)をまとめてリクエストし、
 * line fdを返す(エラー時は-1)。
 */
static int request_lines(int chip_fd,
                          const struct exboard_gpio_entry *sw1,
                          const struct exboard_gpio_entry *led1)
{
    struct gpio_v2_line_request req;
    memset(&req, 0, sizeof(req));

    req.offsets[IDX_SW1]  = sw1->offset;
    req.offsets[IDX_LED1] = led1->offset;
    req.num_lines = 2;
    req.event_buffer_size = 4; /* SW1のエッジイベント用。0ならカーネル既定値 */
    snprintf(req.consumer, sizeof(req.consumer), "exboard-blink-epoll");

    /* attr0: SW1 = input + 立ち上がりエッジ検出 (+ DTから読んだ極性) */
    uint64_t sw1_flags = GPIO_V2_LINE_FLAG_INPUT
                        | GPIO_V2_LINE_FLAG_EDGE_RISING;
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

    /* attr3: SW1のデバウンス時間 */
    if (SW_DEBOUNCE_US > 0) {
        set_line_config_attr(&req.config, 3, GPIO_V2_LINE_ATTR_ID_DEBOUNCE,
                              SW_DEBOUNCE_US, 1ULL << IDX_SW1);
        req.config.num_attrs = 4;
    } else {
        req.config.num_attrs = 3;
    }

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

/* line fdから読めるだけのイベントを処理し、SW1の立ち上がりのときだけ
 * メッセージを出す */
static void handle_line_events(int line_fd, uint32_t sw1_offset)
{
    struct gpio_v2_line_event evbuf[8];
    static int sw_count=0;

    ssize_t n = read(line_fd, evbuf, sizeof(evbuf));
    if (n < 0) {
        if (errno != EAGAIN)
            fprintf(stderr, "line event read failed: %s\n", strerror(errno));
        return;
    }

    size_t count = (size_t)n / sizeof(struct gpio_v2_line_event);
    for (size_t i = 0; i < count; i++) {
        struct gpio_v2_line_event *ev = &evbuf[i];
        if (ev->offset == sw1_offset &&
            ev->id == GPIO_V2_LINE_EVENT_RISING_EDGE) {
            printf("SW1 pressed %d times\n", sw_count);
            fflush(stdout);
	    sw_count++;
        }
    }
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

    /* 1秒周期のタイマーfd */
    int timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (timer_fd < 0) {
        fprintf(stderr, "timerfd_create failed: %s\n", strerror(errno));
        close(line_fd);
        return 1;
    }
    struct itimerspec its = {
        .it_interval = { .tv_sec = BLINK_PERIOD_SEC, .tv_nsec = 0 },
        .it_value    = { .tv_sec = BLINK_PERIOD_SEC, .tv_nsec = 0 },
    };
    if (timerfd_settime(timer_fd, 0, &its, NULL) < 0) {
        fprintf(stderr, "timerfd_settime failed: %s\n", strerror(errno));
        close(line_fd);
        close(timer_fd);
        return 1;
    }

    int epfd = epoll_create1(0);
    if (epfd < 0) {
        fprintf(stderr, "epoll_create1 failed: %s\n", strerror(errno));
        close(line_fd);
        close(timer_fd);
        return 1;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = timer_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, timer_fd, &ev);

    ev.events = EPOLLIN;
    ev.data.fd = line_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, line_fd, &ev);

    signal(SIGINT, on_sigint);

    int led_state = 0;
    struct epoll_event events[2];

    while (!g_stop) {
        int nfds = epoll_wait(epfd, events, 2, -1);
        if (nfds < 0) {
            if (errno == EINTR)
                continue;
            fprintf(stderr, "epoll_wait failed: %s\n", strerror(errno));
            break;
        }

        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == timer_fd) {
                uint64_t expirations;
                ssize_t r = read(timer_fd, &expirations, sizeof(expirations));
                if (r != sizeof(expirations))
                    continue;

                led_state = !led_state;
                if (set_led(line_fd, led_state) < 0)
                    fprintf(stderr, "set_led failed: %s\n", strerror(errno));

            } else if (events[i].data.fd == line_fd) {
                handle_line_events(line_fd, sw1->offset);
            }
        }
    }

    printf("\n終了します\n");
    set_led(line_fd, 0);
    close(epfd);
    close(timer_fd);
    close(line_fd);
    return 0;
}
