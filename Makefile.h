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

SBOM_DIST_URL := https://downloads.raspberrypi.com/raspios_lite_armhf/images/raspios_lite_armhf-2025-05-13/2025-05-13-raspios-bookworm-armhf-lite.sbom.xz

# --- linux/ の場所を自己検出する ---
# Makefile.h が include された相対パス（"Makefile.h" または "../Makefile.h" 等）から
# ディレクトリ部分を取り出す。トップレベルからは "./"、演習ディレクトリからは "../" になる。
MAKEFILE_H_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
BUILD_CONFIG   := $(MAKEFILE_H_DIR)linux/.build_config
-include $(BUILD_CONFIG)
BUILD_DIR   := $(MAKEFILE_H_DIR)linux

ifndef RPI
ifdef BUILT_RPI
RPI := $(BUILT_RPI)
else
RPI := 4
endif
endif

ifndef SBOM_URL
ifdef BUILT_SBOM_URL
SBOM_URL := $(BUILT_SBOM_URL)
else
SBOM_URL := $(SBOM_DIST_URL)
endif
endif



# pilinux系（linux/を作り直す）ターゲットの実行時はチェックしない
CONFIG_CHECK_SKIP_GOALS := pilinux pi3 pi4 pi5 veryclean
ifeq ($(filter $(CONFIG_CHECK_SKIP_GOALS),$(MAKECMDGOALS)),)
ifdef BUILT_RPI
ifneq ($(RPI),$(BUILT_RPI))
$(error linux/ は RPI=$(BUILT_RPI) でビルドされています。RPI=$(RPI) を使う場合は先に 'make RPI=$(RPI) pilinux' を実行してください（意図的に混在させる場合は ALLOW_RPI_MISMATCH=1 を指定）)
endif
ifdef SBOM_URL
ifneq ($(SBOM_URL),$(BUILT_SBOM_URL))
$(warning SBOM_URL が pilinux 実行時（$(BUILT_SBOM_URL)）と異なります。意図しない場合は 'make RPI=$(RPI) SBOM_URL=... pilinux' を再実行してください)
endif
endif
endif
endif

# 明示的に無視したい場合の逃げ道
ifdef ALLOW_RPI_MISMATCH
override BUILT_RPI := $(RPI)
endif

# Raspberry Pi kernel package suffix
RPI_SUFFIX_5 = 2712
RPI_SUFFIX_4 = v7l
RPI_SUFFIX_3 = v7
RPI_SUFFIX_2 = v6
ARCH_SUFFIX = $(RPI_SUFFIX_$(RPI))

SBOM_NAME = pool/$(shell basename $(SBOM_URL))
LINUX_HEADER_PACKAGE_FILE=linux/.header_package
LINUXVER_FILE=linux/.linux_ver
LINUX_LIBCDEV=linux/.libc6_dev
LINUX_LIBC=linux/.libc6
LINUX_LINUX_LIBCDEV=linux/.linux_libc_dev
LINUX_HEADERNAME=linux/.headername
LINUX_COMMIT=linux/.commit
GDBSERVER_FILE=linux/.gdbserver
PYTHON_VER=linux/.python_ver
PYTHON_DEB=linux/.python_deb


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
EABI=
else
ARCH=arm
GNU_ARCH=arm
EABI=eabihf
endif

ifneq (,$(findstring $(RASPIOS_TARG),$(SBOM_URL)))
else
$(error SBOM_URL ($(SBOM_URL)) does not look like a $(RASPIOS_TARG) image for RPI=$(RPI) — update SBOM_URL in Makefile.h)
endif

SYS=$(shell uname -o)
ifeq ($(SYS),Cygwin)
	OS=Cygwin
else
OS := $(shell . /etc/os-release && echo $$ID)
endif

PKGWD=$(shell pwd)

MANDB=$(PKGWD)/share/man_db.conf
MANPATH=$(PKGWD)/share/man

ifneq (,$(filter ubuntu debian,$(OS)))
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

DEPEND := $(LINUX_LIBC) $(LINUX_LIBCDEV) $(LINUX_LINUX_LIBCDEV)
SYSROOT := $(BUILD_DIR)
MULTIARCH = $(GNU_ARCH)-linux-gnu$(EABI)
CFLAGS  += --sysroot=$(SYSROOT) \
	   -B$(SYSROOT)/usr/lib/$(MULTIARCH) \
	   -B$(SYSROOT)/lib/$(MULTIARCH) \
	   -I$(SYSROOT)/usr/include/$(MULTIARCH) \

LDFLAGS += -L$(SYSROOT)/usr/lib/$(MULTIARCH) \
           -Wl,-dynamic-linker=/lib/$(MULTIARCH)/ld-linux-$(RASPIOS_TARG).so.3

