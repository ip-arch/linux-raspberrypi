#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>

"""
exboard.dtso の *-gpios プロパティと gpio-line-names を
/sys/devices/platform/exboard/of_node から読み取り、
{ "ex_sw1": (offset, flags), "ex_led1": (offset, flags), ... }
という辞書を作る。

バイナリ(DTのcell配列/NUL区切り文字列配列)の解析はpygpio.so
(dt_read_gpio_cells/dt_read_line_names)に委ね、Python側は
辞書の組み立てだけを行う。
"""

import pygpio

OF_NODE = "/sys/devices/platform/exboard/of_node"

# exboard.dtso に書かれている *-gpios プロパティの並び順
# (gpio-line-names の並びともこの順で対応している前提)
GPIO_PROPS = [
    "ex_sw-gpios",
    "ex_led-gpios",
    "ex_disp-gpios",
    "ex_seg-gpios",
]

GPIO_ACTIVE_LOW = 1  # dt-bindings/gpio/gpio.h


def get_gpio_map():
    """
    { name: (offset, flags) } の辞書を返す。
    flags は GPIO_V2_LINE_FLAG_ACTIVE_LOW 等と同じビット定義
    (dt-bindings/gpio/gpio.h の GPIO_ACTIVE_LOW=1 がそのまま
    GPIO_V2_LINE_FLAG_ACTIVE_LOW=1<<1の"素の値"ではないので注意。
    ここではDT上のflagsセルの値(0=ACTIVE_HIGH, 1=ACTIVE_LOW)を
    そのまま返す。呼び出し側でGPIO_V2_LINE_FLAG_ACTIVE_LOWに変換する)
    """
    all_entries = []
    for prop in GPIO_PROPS:
        path = f"{OF_NODE}/{prop}"
        for phandle, pin, flags in pygpio.dt_read_gpio_cells(path, ngpio_cells=2):
            all_entries.append((phandle, pin, flags))

    names = pygpio.dt_read_line_names(f"{OF_NODE}/gpio-line-names")

    if len(names) != len(all_entries):
        raise RuntimeError(
            f"gpio-line-names({len(names)}) と "
            f"*-gpios合計({len(all_entries)}) の数が一致しません"
        )

    gpio_map = {}
    for name, (phandle, pin, flags) in zip(names, all_entries):
        gpio_map[name] = (pin, flags)

    return gpio_map


if __name__ == "__main__":
    for name, (offset, flags) in get_gpio_map().items():
        active_low = bool(flags & GPIO_ACTIVE_LOW)
        print(f"{name:12} offset={offset:3}  active_low={active_low}")
