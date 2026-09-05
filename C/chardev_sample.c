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
enum { IDX_SW1 = 0, IDX_LED1 = 1 };

int chip_fd;
struct gpio_v2_line_request req = {.offsets={2,4}, .num_lines=2};
struct gpio_v2_line_values values;

int main() {
	chip_fd = open("/dev/gpiochip0", O_RDONLY);
	if(chip_fd < 0) {
		perror("open");
		return 1;
	}

	/* attrs は req 直下ではなく req.config の下にある。
	 * また、いくつ有効な attrs を使うかを num_attrs で明示する必要がある。 */
	req.config.num_attrs = 2;

	req.config.attrs[IDX_SW1].attr.id = GPIO_V2_LINE_ATTR_ID_FLAGS;
	req.config.attrs[IDX_SW1].attr.flags = GPIO_V2_LINE_FLAG_ACTIVE_LOW | GPIO_V2_LINE_FLAG_INPUT;
	req.config.attrs[IDX_SW1].mask = 1UL<<IDX_SW1;

	/* active-high はデフォルト状態なのでフラグとして立てるものは無く、
	 * GPIO_V2_LINE_FLAG_OUTPUT のみを指定すればよい。 */
	req.config.attrs[IDX_LED1].attr.id = GPIO_V2_LINE_ATTR_ID_FLAGS;
	req.config.attrs[IDX_LED1].attr.flags = GPIO_V2_LINE_FLAG_OUTPUT;
	req.config.attrs[IDX_LED1].mask = 1UL<<IDX_LED1;

	int ret = ioctl(chip_fd, GPIO_V2_GET_LINE_IOCTL, &req);
	if(ret < 0) {
		perror("GET_LINE");
		return 1;
	}

	/* GET_LINE が終われば chip_fd 自体はもう使わないので閉じてよい。
	 * 以降のやり取りは req.fd (ラインリクエストのfd) に対して行う。 */
	close(chip_fd);

	while (1) {
		int sw;

		values.mask = 1UL<<IDX_SW1;
		ret = ioctl(req.fd, GPIO_V2_LINE_GET_VALUES_IOCTL, &values);
		if(ret < 0) {
			perror("LINE_GET");
			return 1;
		}
		sw = (values.bits & (1UL << IDX_SW1)) ? 1 : 0;

		/* GET で使った mask が残ったままだと SET が SW1 のビット位置に
		 * 対して行われてしまう。SET 対象を LED1 に切り替える。 */
		values.mask = 1UL<<IDX_LED1;
		values.bits = ((__u64)sw) << IDX_LED1;
		ret = ioctl(req.fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &values);
		if(ret < 0) {
			perror("LINE_SET");
			return 1;
		}

		usleep(10 * 1000); /* 10ms間隔でポーリング。CPUを使い切らないための待ち */
	}
	return 0;
}
