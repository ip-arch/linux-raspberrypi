include Makefile.h

sbom: $(RASPIOS_DATE)-raspios-$(DEBIAN_CODENAME)-$(RASPIOS_TARG)-$(RASPIOS_SZ).sbom.xz

$(RASPIOS_DATE)-raspios-$(DEBIAN_CODENAME)-$(RASPIOS_TARG)-$(RASPIOS_SZ).sbom.xz:
	wget https://downloads.raspberrypi.org/raspios_$(RASPIOS_SZ)_$(RASPIOS_TARG)/images/raspios_$(RASPIOS_SZ)_$(RASPIOS_TARG)-$(RASPIOS_DATE)/$@

linux/.headername: $(RASPIOS_DATE)-raspios-$(DEBIAN_CODENAME)-$(RASPIOS_TARG)-$(RASPIOS_SZ).sbom.xz
	mkdir -p linux
	VER=$$(xz -dc $< | jq -r '.packages[]? | select(.name == "linux-headers-$(LINUXVER)+rpt-common-rpi") | .versionInfo') ;\
	VER_CLEAN=$${VER#*:} ;\
	echo $$VER_CLEAN > $@ ; \
	wget https://archive.raspberrypi.org/debian/pool/main/l/linux/linux-headers-$(LINUXVER)+rpt-rpi-$(ARCH_SUFFIX)_$${VER_CLEAN}_$(RASPIOS_TARG).deb ; \
	ar p linux-headers-$(LINUXVER)+rpt-rpi-$(ARCH_SUFFIX)_$${VER_CLEAN}_$(RASPIOS_TARG).deb data.tar.xz| tar -xJ -C linux
	
CHANGE_LOG:=linux/usr/share/doc/linux-headers-$(LINUXVER)+rpt-rpi-$(ARCH_SUFFIX)/changelog.Debian.gz 

linux/.linux_commit: linux/.headername
	echo $(CHANGE_LOG) ; \
	gunzip -dcf $(CHANGE_LOG) | \
	sed -n 's/^  \* Linux commit: //p' | \
	head -n1 > $@


linux/usr/src/linux/.git: linux/.linux_commit
	(cd linux/usr/src; \
	git clone --filter=blob:none --single-branch --branch $(LINUX_BRANCH) \
	    https://github.com/raspberrypi/linux.git linux \
	)
linux/.linux_src: linux/usr/src/linux/.git linux/.linux_commit
	cd linux/usr/src/linux && git fetch
	cd linux/usr/src/linux && git checkout $$(cat ../../../../linux/.linux_commit)
	touch $@

pi5:
	$(MAKE) RPI=5 pilinux

pi4:
	$(MAKE) RPI=4 pilinux

pi3:
	$(MAKE) RPI=3 pilinux

SYMVERS=linux/usr/src/linux-headers-$(LINUXVER)+rpt-rpi-$(ARCH_SUFFIX)/Module.symvers
DOT_CONFIG=linux/usr/src/linux-headers-$(LINUXVER)+rpt-rpi-$(ARCH_SUFFIX)/.config

pilinux:	$(TOOLDIR)$(CROSS_COMPILE)gcc linux/.linux_src linux/.headername
	echo $(SYMVERS) $(DOT_CONFIG)
	cp $(SYMVERS) linux/usr/src/linux
	cp $(DOT_CONFIG) linux/usr/src/linux
	( cd linux/usr/src/linux ;\
	KERNEL=$(KERNEL_NAME) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) make oldconfig prepare modules_prepare; \
	)
	make exboard.dtbo

exboard.dtbo: dts/exboard.dtso
	cpp -nostdinc -undef -D__DTS__ -x assembler-with-cpp \
	-Ilinux/usr/src/linux/scripts/dtc/include-prefixes  dts/exboard.dtso | \
	linux/usr/src/linux/scripts/dtc/dtc -O dtb -o exboard.dtbo

pi5-veryclean:
	$(MAKE) RPI=5 veryclean

pi4-veryclean:
	$(MAKE) RPI=4 veryclean

pi3-veryclean:
	$(MAKE) RPI=3 veryclean

veryclean:
	make clean
	rm -fr exboard.dtbo
	rm  -f $(RASPIOS_DATE)-raspios-$(DEBIAN_CODENAME)-$(RASPIOS_TARG)-$(RASPIOS_SZ).sbom.xz linux-headers-$(LINUXVER)+rpt-rpi-$(ARCH_SUFFIX)_$(shell cat linux/.headername)_$(RASPIOS_TARG).deb 
	rm -fr linux
clean:
	(cd modules; make clean)
	(cd apps/C; make clean)
	(cd apps/Python; make clean)

################################
# suffix rules
################################
.SUFFIXES:  .man

%.man:
	man -C $(MANDB) $*
	# MANPATH=$(PKGWD)/share/man man -C $(MANDB) $*
