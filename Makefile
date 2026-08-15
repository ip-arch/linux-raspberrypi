include Makefile.h

sbom: $(SBOM_NAME)

$(SBOM_NAME):
	wget -nc $(SBOM_URL)

install: 
	sh scripts/wsl_script.sh

$(LINUX_HEADER_PACKAGE_FILE):  $(SBOM_NAME)
	mkdir -p linux
	xzcat $< | jq -r --arg suffix "$(ARCH_SUFFIX)" \
	 '.packages[] | select(.name | (test("^linux-headers-[0-9]+\\.[0-9]+\\.[0-9]+\\+")  and endswith($$suffix)) ) | .name' > $@

$(GDBSERVER_FILE):	$(SBOM_NAME)
	mkdir -p linux
	xzcat $< |  \
	jq -r ' .packages[]?  | select(.name == "gdb") | .externalRefs[]?  | select(.referenceType == "purl") | .referenceLocator | capture("@(?<version>[^?]+)\\?arch=(?<arch>[^&]+)") | "gdbserver_\(.version)_\(.arch).deb" ' > $@
	wget -nc \
	    "https://ftp.debian.org/debian/pool/main/g/gdb/$$(cat $@)"

$(LINUXVER_FILE): $(LINUX_HEADER_PACKAGE_FILE)
	sed -E 's/^linux-headers-([0-9]+\.[0-9]+\.[0-9]+).*/\1/' \
	  $< > $@
	

$(LINUX_HEADERNAME):  $(LINUXVER_FILE)
	mkdir -p linux
	VER=$$(xz -dc $(SBOM_NAME) | jq -r '.packages[]? | select(.name == "linux-headers-$(shell cat $(LINUXVER_FILE))+rpt-common-rpi") | .versionInfo') ;\
	VER_CLEAN=$${VER#*:} ;\
	echo $$VER_CLEAN > $@ ; \
	wget -nc https://archive.raspberrypi.org/debian/pool/main/l/linux/linux-headers-$(shell cat $(LINUXVER_FILE))+rpt-rpi-$(ARCH_SUFFIX)_$${VER_CLEAN}_$(RASPIOS_TARG).deb ; \
	ar p linux-headers-$(shell cat $(LINUXVER_FILE))+rpt-rpi-$(ARCH_SUFFIX)_$${VER_CLEAN}_$(RASPIOS_TARG).deb data.tar.xz| tar -xJ -C linux ;\
	wget -nc https://archive.raspberrypi.org/debian/pool/main/l/linux/linux-headers-$(shell cat $(LINUXVER_FILE))+rpt-common-rpi_$${VER_CLEAN}_all.deb ;\
	ar p linux-headers-$(shell cat $(LINUXVER_FILE))+rpt-common-rpi_$${VER_CLEAN}_all.deb data.tar.xz | tar -xJ -C linux
	

$(LINUX_COMMIT): $(LINUX_HEADERNAME)
	CHANGE_LOG=linux/usr/share/doc/linux-headers-$(shell cat $(LINUXVER_FILE))+rpt-common-rpi/changelog.Debian.gz  ; \
	echo $${CHANGE_LOG} ; \
	gunzip -dcf $${CHANGE_LOG} | \
	sed -n 's/^  \* Linux commit: //p' | \
	head -n1 > $@
	cat $@


linux/usr/src/linux/.git: $(LINUX_COMMIT)
	LINUX_MAJOR_MINOR=$(word 1,$(subst ., ,$(shell cat $(LINUXVER_FILE)))).$(word 2,$(subst ., ,$(shell cat $(LINUXVER_FILE)))) ; \
	LINUX_BRANCH=rpi-$${LINUX_MAJOR_MINOR}.y ; \
	(cd linux/usr/src; \
	git clone --filter=blob:none --single-branch --branch $${LINUX_BRANCH} \
	    https://github.com/raspberrypi/linux.git linux \
	)
linux/.linux_src: linux/usr/src/linux/.git $(LINUX_COMMIT)
	cd linux/usr/src/linux && git fetch
	cd linux/usr/src/linux && git checkout $$(cat ../../../../$(LINUX_COMMIT))
	touch $@

pi5:
	$(MAKE) RPI=5 pilinux

pi4:
	$(MAKE) RPI=4 pilinux

pi3:
	$(MAKE) RPI=3 pilinux

SYMVERS=linux/usr/src/linux-headers-$(shell cat $(LINUXVER_FILE))+rpt-rpi-$(ARCH_SUFFIX)/Module.symvers
DOT_CONFIG=linux/usr/src/linux-headers-$(shell cat $(LINUXVER_FILE))+rpt-rpi-$(ARCH_SUFFIX)/.config

pilinux:	$(TOOLDIR)$(CROSS_COMPILE)gcc linux/.linux_src $(LINUX_HEADERNAME) $(GDBSERVER_FILE)
	echo $(SYMVERS) $(DOT_CONFIG)
	cp $(SYMVERS) linux/usr/src/linux
	cp $(DOT_CONFIG) linux/usr/src/linux
	( cd linux/usr/src/linux ;\
	KERNEL=$(KERNEL_NAME) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) make oldconfig prepare modules_prepare; \
	)
	make RPI=$(RPI) SBOM_URL=$(SBOM_URL) $(BUILD_CONFIG)
	make exboard.dtbo

$(BUILD_CONFIG):
	echo "BUILT_RPI := $(RPI)" > $(BUILD_CONFIG)
	echo "BUILT_SBOM_URL := $(SBOM_URL)" >> $(BUILD_CONFIG)


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
	rm  -f $(SBOM_NAME) linux-headers-$(shell cat $(LINUXVER_FILE))+rpt-rpi-$(ARCH_SUFFIX)_$(shell cat $(LINUX_HEADERNAME))_$(RASPIOS_TARG).deb linux-headers-$(shell cat $(LINUXVER_FILE))+rpt-common-rpi_$(shell cat $(LINUX_HEADERNAME))_all.deb
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
