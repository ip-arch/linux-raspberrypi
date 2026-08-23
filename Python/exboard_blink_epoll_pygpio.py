#!/usr/bin/env python3
"""
exboard_blink_epoll_pygpio.py

exboard_blink_epoll.py と全く同じ動作
  - LED1 (ex_led1) を周期タイマー(timerfd)で1秒ごとに点滅
  - SW1  (ex_sw1)  の立ち上がりエッジ(押下)を検出

だが、GPIO chardev v2のioctl/構造体マーシャリングは全て
pygpio拡張モジュール(pygpio.c)にやらせる。
struct.pack_into/unpack_fromや_IOC/_IOWRの自前実装は一切出てこない。

事前に以下でビルドしておくこと:
    $ python3 setup.py build_ext --inplace
"""

import os
import time
import select
import signal
import ctypes
import ctypes.util

import pygpio
from exboard_map import get_gpio_map, GPIO_ACTIVE_LOW as DT_ACTIVE_LOW


CHIP_NUM = 0  # 環境に合わせて調整

# チャタリング除去時間 [us]
SW_DEBOUNCE_US = 10_000  # 10ms

BLINK_PERIOD_SEC = 1

# offsets[] 内でのインデックス (= mask のビット位置)
IDX_SW1, IDX_LED1 = 0, 1


# ----------------------------------------------------------------------
# timerfd (ctypes経由。pygpio側に含めていないので、こちらはPython側の資産のまま)
# ----------------------------------------------------------------------

_libc = ctypes.CDLL(ctypes.util.find_library("c"), use_errno=True)
CLOCK_MONOTONIC = 1


class _timespec(ctypes.Structure):
    _fields_ = [("tv_sec", ctypes.c_long), ("tv_nsec", ctypes.c_long)]


class _itimerspec(ctypes.Structure):
    _fields_ = [("it_interval", _timespec), ("it_value", _timespec)]


def timerfd_create_periodic(period_sec):
    fd = _libc.timerfd_create(CLOCK_MONOTONIC, 0)
    if fd < 0:
        errno = ctypes.get_errno()
        raise OSError(errno, os.strerror(errno), "timerfd_create")

    its = _itimerspec(
        it_interval=_timespec(period_sec, 0),
        it_value=_timespec(period_sec, 0),
    )
    if _libc.timerfd_settime(fd, 0, ctypes.byref(its), None) < 0:
        errno = ctypes.get_errno()
        os.close(fd)
        raise OSError(errno, os.strerror(errno), "timerfd_settime")

    return fd


def build_attrs(sw1_offset_active_low, led1_active_low):
    sw1_flags = pygpio.LINE_FLAG_INPUT | pygpio.LINE_FLAG_EDGE_RISING
    if sw1_offset_active_low:
        sw1_flags |= pygpio.LINE_FLAG_ACTIVE_LOW

    led1_flags = pygpio.LINE_FLAG_OUTPUT
    if led1_active_low:
        led1_flags |= pygpio.LINE_FLAG_ACTIVE_LOW

    attrs = [
        (pygpio.LINE_ATTR_ID_FLAGS, sw1_flags, 1 << IDX_SW1),
        (pygpio.LINE_ATTR_ID_FLAGS, led1_flags, 1 << IDX_LED1),
        (pygpio.LINE_ATTR_ID_OUTPUT_VALUES, 0, 1 << IDX_LED1),  # LED1初期値=消灯
    ]
    if SW_DEBOUNCE_US > 0:
        attrs.append((pygpio.LINE_ATTR_ID_DEBOUNCE, SW_DEBOUNCE_US, 1 << IDX_SW1))

    return attrs


def main():
    gpio_map = get_gpio_map()
    sw1_offset, sw1_dt_flags = gpio_map["ex_sw1"]
    led1_offset, led1_dt_flags = gpio_map["ex_led1"]
    sw1_active_low = bool(sw1_dt_flags & DT_ACTIVE_LOW)
    led1_active_low = bool(led1_dt_flags & DT_ACTIVE_LOW)
    count = 0

    chip_fd = os.open(f"/dev/gpiochip{CHIP_NUM}", os.O_RDONLY)
    try:
        line_fd = pygpio.request_line(
            chip_fd,
            [sw1_offset, led1_offset],
            consumer="exboard-blink-pygpio",
            attrs=build_attrs(sw1_active_low, led1_active_low),
            event_buffer_size=4,
        )
    finally:
        os.close(chip_fd)  # line_fdが独立して使えるのでchip_fdはもう不要

    timer_fd = timerfd_create_periodic(BLINK_PERIOD_SEC)

    ep = select.epoll()
    ep.register(timer_fd, select.EPOLLIN)
    ep.register(line_fd, select.EPOLLIN)

    stop = False

    def on_sigint(signo, frame):
        nonlocal stop
        stop = True

    signal.signal(signal.SIGINT, on_sigint)

    led_state = False
    try:
        while not stop:
            for fd, _flag in ep.poll():
                if fd == timer_fd:
                    os.read(timer_fd, 8)  # 満了回数を読み捨てる
                    led_state = not led_state
                    pygpio.set_values(
                        line_fd,
                        (1 << IDX_LED1) if led_state else 0,
                        1 << IDX_LED1,
                    )
                elif fd == line_fd:
                    timestamp_ns, ev_id, offset, seqno, line_seqno = pygpio.read_event(line_fd)
                    if offset == sw1_offset and ev_id == pygpio.LINE_EVENT_RISING_EDGE:
                        count += 1
                        print("SW1 pressed",count)
    except KeyboardInterrupt:
        pass
    finally:
        print("\n終了します")
        pygpio.set_values(line_fd, 0, 1 << IDX_LED1)
        os.close(timer_fd)
        os.close(line_fd)


if __name__ == "__main__":
    main()
