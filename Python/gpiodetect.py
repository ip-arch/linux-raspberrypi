#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>


import os
import fcntl
import struct


# ----------------------------------------------------------------------
# Linux ioctl encoding
# ----------------------------------------------------------------------

IOC_NRBITS   = 8
IOC_TYPEBITS = 8
IOC_SIZEBITS = 14
IOC_DIRBITS  = 2

IOC_NRSHIFT   = 0
IOC_TYPESHIFT = IOC_NRSHIFT + IOC_NRBITS
IOC_SIZESHIFT = IOC_TYPESHIFT + IOC_TYPEBITS
IOC_DIRSHIFT  = IOC_SIZESHIFT + IOC_SIZEBITS

IOC_NONE  = 0
IOC_WRITE = 1
IOC_READ  = 2


def _IOC(direction, type_, nr, size):
    return (
        (direction << IOC_DIRSHIFT) |
        (type_ << IOC_TYPESHIFT) |
        (nr << IOC_NRSHIFT) |
        (size << IOC_SIZESHIFT)
    )


def _IOR(type_, nr, size):
    return _IOC(IOC_READ, type_, nr, size)


def _IOWR(type_, nr, size):
    return _IOC(IOC_READ | IOC_WRITE, type_, nr, size)


# ----------------------------------------------------------------------
# GPIO UAPI
# ----------------------------------------------------------------------

# GPIO ioctlの"type"はASCIIの'B'(0x42)ではなく
# カーネルヘッダ(linux/gpio.h)で定義されたマジックナンバー0xB4
GPIO_MAGIC = 0xB4

GPIO_MAX_NAME_SIZE = 32

GPIO_V2_LINE_FLAG_USED       = 1 << 0
GPIO_V2_LINE_FLAG_ACTIVE_LOW = 1 << 1
GPIO_V2_LINE_FLAG_INPUT      = 1 << 2
GPIO_V2_LINE_FLAG_OUTPUT     = 1 << 3


# struct gpio_v2_chip_info {
#     char name[32];
#     char label[32];
#     __u32 lines;
# };
GPIO_V2_CHIP_INFO_SIZE = 32 + 32 + 4

GPIO_V2_GET_CHIPINFO_IOCTL = _IOR(
    GPIO_MAGIC, 0x01, GPIO_V2_CHIP_INFO_SIZE
)


# struct gpio_v2_line_info {
#     char name[32];
#     char consumer[32];
#     __u32 offset;
#     __u32 num_attrs;
#     __u64 flags;
#     struct gpio_v2_line_attribute attrs[10];  // 16 bytes * 10 = 160
#     __u32 padding[4];                          // 16
# };
# name(32) + consumer(32) + offset(4) + num_attrs(4) + flags(8)
#   + attrs(160) + padding(16) = 256
GPIO_V2_LINE_INFO_SIZE = 32 + 32 + 4 + 4 + 8 + 16 * 10 + 16

# name(32)+consumer(32) の直後、offsetフィールドのバイトオフセット
GPIO_V2_LINE_INFO_OFFSET_POS = 32 + 32

GPIO_V2_GET_LINEINFO_IOCTL = _IOWR(
    GPIO_MAGIC, 0x05, GPIO_V2_LINE_INFO_SIZE
)


def get_string(data):
    return data.split(b"\0", 1)[0].decode(errors="replace")


def get_chip_info(fd):
    buf = bytearray(GPIO_V2_CHIP_INFO_SIZE)

    fcntl.ioctl(
        fd,
        GPIO_V2_GET_CHIPINFO_IOCTL,
        buf,
        True
    )

    name, label, lines = struct.unpack(
        "32s32sI", buf
    )

    return (
        get_string(name),
        get_string(label),
        lines
    )


def get_line_info(fd, offset):
    buf = bytearray(GPIO_V2_LINE_INFO_SIZE)

    # ioctl argumentのoffsetにline番号を入れる
    # (name[32]+consumer[32]の後にoffsetフィールドがある)
    struct.pack_into("I", buf, GPIO_V2_LINE_INFO_OFFSET_POS, offset)

    fcntl.ioctl(
        fd,
        GPIO_V2_GET_LINEINFO_IOCTL,
        buf,
        True
    )

    name, consumer, offset, num_attrs, flags = struct.unpack_from(
        "32s32sIIQ", buf
    )

    return offset, flags, get_string(name), get_string(consumer)


def flags_to_string(flags):
    direction = []

    if flags & GPIO_V2_LINE_FLAG_INPUT:
        direction.append("input")

    if flags & GPIO_V2_LINE_FLAG_OUTPUT:
        direction.append("output")

    if not direction:
        direction.append("-")

    active_low = bool(
        flags & GPIO_V2_LINE_FLAG_ACTIVE_LOW
    )

    used = bool(
        flags & GPIO_V2_LINE_FLAG_USED
    )

    return (
        ",".join(direction),
        active_low,
        used
    )


# ----------------------------------------------------------------------
# main
# ----------------------------------------------------------------------

chip = 0
path = f"/dev/gpiochip{chip}"

fd = os.open(path, os.O_RDONLY)

try:
    name, label, lines = get_chip_info(fd)

    print(f"name  : {name}")
    print(f"label : {label}")
    print(f"lines : {lines}")
    print()

    print(
        f"{'line':4} "
        f"{'name':20} "
        f"{'consumer':24} "
        f"{'direction':8} "
        f"{'active-low':10} "
        f"{'used':5}"
    )

    print("-" * 80)

    for offset in range(lines):
        offset, flags, name, consumer = \
            get_line_info(fd, offset)

        direction, active_low, used = \
            flags_to_string(flags)

        print(
            f"{offset:4} "
            f"{name:20} "
            f"{consumer:24} "
            f"{direction:8} "
            f"{str(active_low):10} "
            f"{str(used):5}"
        )

finally:
    os.close(fd)
