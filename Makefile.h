#
# Raspberry PI OS exercise environment
#   Firmware version is captured in the release notes.
#   2026-07-28: Change retrieve method for bookworm
#   2023-05-03 release failed then I go back to 2022-09-21
#
#   By Naohiko Shimizu
#
#
#
#

RPI ?= 4

# Raspberry Pi kernel package suffix
RPI_SUFFIX_5 = 2712
RPI_SUFFIX_4 = v7l
RPI_SUFFIX_3 = v7
RPI_SUFFIX_2 = v6
ARCH_SUFFIX = $(RPI_SUFFIX_$(RPI))

# Raspberry Pi OS
DEBIAN_CODENAME = bookworm
RASPIOS_SZ = lite
RASPIOS_DATE = 2025-05-13

LINUXVER = 6.12.25

ifeq ($(RPI), 5)
KERNEL_ARCH_BIT=64
RASPIOS_TARG = arm64
KERNEL_NAME=kernel_$(ARCH_SUFFIX)
else
KERNEL_ARCH_BIT=32
RASPIOS_TARG = armhf
KERNEL_NAME=kernel$(ARCH_SUFFIX)
endif

ifeq ($(KERNEL_ARCH_BIT),64)
ARCH=arm64
GNU_ARCH=aarch64
else
ARCH=arm
GNU_ARCH=arm
endif

SYS=$(shell uname -o)
ifeq ($(SYS),Cygwin)
	OS=Cygwin
else
OS=$(shell lsb_release -si)
endif

PKGWD=$(shell pwd)

MANDB=$(PKGWD)/share/man_db.conf
MANPATH=$(PKGWD)/share/man
LINUX_MAJOR_MINOR = $(word 1,$(subst ., ,$(LINUXVER))).$(word 2,$(subst ., ,$(LINUXVER)))
LINUX_BRANCH = rpi-$(LINUX_MAJOR_MINOR).y

ifneq (,$(filter Ubuntu Debian,$(OS)))
TOOLDIR=/usr/bin/
ifeq ($(ARCH), arm)
CROSS_COMPILE=arm-linux-gnueabihf-
$(TOOLDIR)$(CROSS_COMPILE)gcc:
	sudo apt install crossbuild-essential-armhf git bc bison flex libssl-dev libc6-dev libncurses5-dev
else
CROSS_COMPILE=aarch64-linux-gnu-
$(TOOLDIR)$(CROSS_COMPILE)gcc:
	sudo apt install crossbuild-essential-arm64 git bc bison flex libssl-dev libc6-dev libncurses5-dev
endif
else ifeq ($(OS),Cygwin)
ifeq ($(ARCH), arm)
CROSS_COMPILE=arm-armv7hf-linux-gnueabihf-
TOOLDIR=$(HOME)/x-tools/arm-armv7hf-linux-gnueabihf/bin/
else
CROSS_COMPILE=aarch64-aarch64-linux-gnu-
TOOLDIR=$(HOME)/x-tools/aarch64-aarch64-linux-gnu/bin/
endif
PATH=/usr/bin:/usr/local/bin:$(TOOLDIR)
else
TOOLDIR=/usr/bin/
CROSS_COMPILE=$(GNU_ARCH)-linux-gnu-
$(TOOLDIR)$(CROSS_COMPILE)gcc:
	sudo dnf install  gcc-$(GNU_ARCH)-linux-gnu git bc bison flex openssl-devel ncurses-devel
endif

