include ../Makefile.h
PKGWD=$(HOME)/rpi4
CC=$(CROSS_COMPILE)gcc
GDB=$(CROSS_COMPILE)gdb
RANLIB=$(CROSS_COMPILE)ranlib
# LDFLAGS="-lrt"
MANDB=$(PKGWD)/share/man_db.conf

clean:
	rm -rf *.exe *.o *.s *~ {API,RPI,mem}/*.exe

################################
# suffix rules
################################
.SUFFIXES:  .c .s .o .wpi .exe .debug .man

%.man:
	man -C $(MANDB) $*
.c.s:
	$(CC) -S -O2 $< -o $*.s

.c.exe:
	$(CC) -g -O2 $< -o $*.exe $(ADDFLAGS) $(CFLAGS) $(LDFLAGS)

%.debug:
	$(GDB) $*

