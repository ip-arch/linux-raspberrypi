include ../Makefile.h
PKGWD=$(HOME)/linux-raspberrypi
CC=$(CROSS_COMPILE)gcc
GDB=$(CROSS_COMPILE)gdb
RANLIB=$(CROSS_COMPILE)ranlib
# LDFLAGS="-lrt"
MANDB=$(PKGWD)/share/man_db.conf

clean:
	rm -rf *.exe *.o *.s *~ {API,RPI,mem}/*.exe *.so __pycache__

pygpio.so: PyGPIO/pygpio.c
	make PyGPIO/$@
	mv PyGPIO/$@ $@

PYTHON_VER_FILE := ../$(PYTHON_VER)

%.so: %.c | check_python
	$(CC) -g -O2 -shared -fPIC $< -o $@ $(ADDFLAGS) $(CFLAGS) $(LDFLAGS) -I$(SYSROOT)/usr/include/$(shell cat $(PYTHON_VER_FILE))

# ファイルが存在しない場合のみ、親ディレクトリ等の Make を実行する
check_python:
ifeq ($(wildcard $(PYTHON_VER_FILE)),)
	@echo "Python environment not found. Building Python environment first..."
	$(MAKE) -C .. install_python_dev
endif


%.man:
	man -C $(MANDB) $*

