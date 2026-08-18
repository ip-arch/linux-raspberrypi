include ../Makefile.h
PKGWD=$(shell pwd)/..
BUILD=$(PKGWD)/linux/usr/src/linux
M=`pwd`
CC=$(CROSS_COMPILE)gcc
MULTIARCH = $(GNU_ARCH)-linux-gnu$(EABI)


ifeq ($(OS), almalinux)
CFLAGS  += --sysroot=$(SYSROOT) \
	   -B$(SYSROOT)/usr/lib/$(MULTIARCH) \
	   -B$(SYSROOT)/lib/$(MULTIARCH) \
	   -I$(SYSROOT)/usr/include/$(MULTIARCH) \

LDFLAGS += -L$(SYSROOT)/usr/lib/$(MULTIARCH) \
           -Wl,-dynamic-linker=/lib/$(MULTIARCH)/ld-linux-$(RASPIOS_TARG).so.3

endif

clean:
	rm -rf ./.[a-z]* *.ko *.mod.c *.o *.mod modules.order Module.symvers *~ *.exe


$(BUILD)/drivers/gpio/gpio-sim.ko:
	make $@

gpio-sim.ko: $(BUILD)/drivers/gpio/gpio-sim.ko
	cp $< $@

################################
# suffix rules
################################
.SUFFIXES:  .c .ko .exe

.c.ko:
	ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) make -C $(BUILD) M=$(M) V=1 NAME=$*

.c.exe:
	$(CC) -g -O2 $< -o $*.exe $(ADDFLAGS)  $(CFLAGS) $(LDFLAGS)

