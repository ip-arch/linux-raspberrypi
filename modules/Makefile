include ../Makefile.h
PKGWD=$(shell pwd)/..
BUILD=$(PKGWD)/linux/usr/src/linux
BUILD_REL=../linux/usr/src/linux
M=`pwd`
CC=$(CROSS_COMPILE)gcc

clean:
	rm -rf ./\.[\.a-zA-Z]?* *.ko *.mod.c *.o *.mod modules.order Module.symvers *~ *.exe


gpio-sim.ko: $(BUILD_REL)/drivers/gpio/gpio-sim.ko
	cp $< $@

################################
# suffix rules
################################
.SUFFIXES:  .c .ko .exe

.c.ko:
	ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) $(MAKE) -C $(BUILD) M=$(M) V=1 NAME=$*

.c.exe:
	$(CC) -g -O2 $< -o $*.exe $(ADDFLAGS)  $(CFLAGS) $(LDFLAGS)

