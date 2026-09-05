// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
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
#define CHIP_NUM 0  /* gpiodetect2.py と同じチップ番号。環境に合わせて調整 */
enum { IDX_SW1 = 0, IDX_DEBOUNCE = 1 };
#define EVENT_NUM 10

int chip_fd;
struct gpio_v2_line_request req = {.offsets={2}, .num_lines=1, .event_buffer_size=EVENT_NUM};
struct gpio_v2_line_event event;

int main() {
	chip_fd = open("/dev/gpiochip0", O_RDONLY);
	if(chip_fd < 0) {
		perror("open");
		return errno;
	}

	/* attrs は req 直下ではなく req.config の下にある。
	 * また、いくつ有効な attrs を使うかを num_attrs で明示する必要がある。 */

#if defined(DEBOUNCE)
	req.config.num_attrs = 2;
	req.config.attrs[IDX_DEBOUNCE].attr.id = GPIO_V2_LINE_ATTR_ID_DEBOUNCE;
	req.config.attrs[IDX_DEBOUNCE].attr.debounce_period_us = 50000; /* 50ms */
	req.config.attrs[IDX_DEBOUNCE].mask = 1UL<<IDX_SW1;
#else
	req.config.num_attrs = 1;
#endif
	req.config.attrs[IDX_SW1].attr.id = GPIO_V2_LINE_ATTR_ID_FLAGS;
	req.config.attrs[IDX_SW1].attr.flags = GPIO_V2_LINE_FLAG_ACTIVE_LOW | GPIO_V2_LINE_FLAG_INPUT |
		GPIO_V2_LINE_FLAG_EDGE_RISING;
	req.config.attrs[IDX_SW1].mask = 1UL<<IDX_SW1;

	int ret = ioctl(chip_fd, GPIO_V2_GET_LINE_IOCTL, &req);
	if(ret < 0) {
		perror("GET_LINE");
		return errno;
	}

	/* GET_LINE が終われば chip_fd 自体はもう使わないので閉じてよい。
	 * 以降のやり取りは req.fd (ラインリクエストのfd) に対して行う。 */
	close(chip_fd);

	while (1) {
		int n;

		n = read(req.fd, &event, sizeof(event));
		if (n < 0) {
		    if (errno == EINTR) continue;
		    perror("read");
		    return errno;
		}
		if(n != sizeof(event)) continue;
		if(event.id == GPIO_V2_LINE_EVENT_RISING_EDGE) {
			printf("%llu ns: id=%u, offset=%u, seqno=%u, line_seqno=%u\n",
				       	event.timestamp_ns, event.id, event.offset, event.seqno, event.line_seqno);
		}
	}
	return 0;
}
