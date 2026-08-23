#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>

"""
exboard の DT overlay (exboard.dtso) に書かれた *-gpios プロパティを
生バイナリのまま読んで、"ex_sw1" のような名前 -> gpiochip上のline offset
の対応表を作る。

前提:
  - <&gpio N FLAGS> の形式 (#gpio-cells = 2) を想定
    -> 1エントリは <phandle, pin, flags> の3セル(uint32, big-endian)
  - gpio-line-names の並び順が、dtso中の
    ex_sw-gpios, ex_led-gpios, ex_disp-gpios, ex_seg-gpios
    の記述順とそのまま対応している(このdtsoの書き方が前提)

exboardノード自身のgpio-line-namesは、カーネルのgpiolibが読む
ものではない(gpiolibは&gpio自身のgpio-line-namesしか見ない)ので、
実機のgpiochipのline名には反映されない。ここではDTソースの内容
そのものから、名前<->line offsetの対応をユーザー空間側で組み立てる。
"""

import os
import struct

OF_NODE = "/sys/devices/platform/exboard/of_node"

# exboard.dtso に書かれている *-gpios プロパティの並び順
GPIO_PROPS = [
    "ex_sw-gpios",
    "ex_led-gpios",
    "ex_disp-gpios",
    "ex_seg-gpios",
]

GPIO_ACTIVE_LOW = 1  # dt-bindings/gpio/gpio.h


def read_gpios_prop(name, ngpio_cells=2):
    """<phandle, cell0, cell1, ...> のn個組をリストで返す"""
    path = os.path.join(OF_NODE, name)
    with open(path, "rb") as f:
        data = f.read()

    n_u32 = 1 + ngpio_cells  # phandle + gpio-specifier cells
    entry_size = n_u32 * 4

    entries = []
    for off in range(0, len(data), entry_size):
        raw = struct.unpack_from(f">{n_u32}I", data, off)
        phandle = raw[0]
        pin = raw[1]
        flags = raw[2] if n_u32 >= 3 else 0
        entries.append((phandle, pin, flags))

    return entries


def read_line_names():
    path = os.path.join(OF_NODE, "gpio-line-names")
    with open(path, "rb") as f:
        data = f.read()
    # NUL区切りの文字列配列
    return [s.decode() for s in data.split(b"\0") if s]


def main():
    all_entries = []
    for prop in GPIO_PROPS:
        try:
            all_entries.extend(read_gpios_prop(prop))
        except FileNotFoundError:
            print(f"[warn] {prop} が見つかりません (of_node={OF_NODE})")

    names = read_line_names()

    if len(names) != len(all_entries):
        print(
            f"[warn] gpio-line-names の数({len(names)}) と "
            f"*-gpios の総数({len(all_entries)}) が一致していません。"
            f"並び順の対応が崩れている可能性があります。"
        )

    phandles = {e[0] for e in all_entries}
    if len(phandles) > 1:
        print(
            f"[warn] 複数のgpioコントローラのphandleが混在しています: {phandles} "
            f"(このスクリプトは全ライン同一チップ前提)"
        )

    print(f"{'name':12} {'line':6} {'active_low':10}")
    print("-" * 32)
    for name, (phandle, pin, flags) in zip(names, all_entries):
        active_low = bool(flags & GPIO_ACTIVE_LOW)
        print(f"{name:12} {pin:<6} {str(active_low):10}")


if __name__ == "__main__":
    main()
