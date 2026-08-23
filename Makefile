include Makefile.h

POOL_DIR := pool
sbom: $(SBOM_NAME)

$(SBOM_NAME):
	wget -nc  -P $(POOL_DIR) $(SBOM_URL)

install: 
	$(MAKE) RPI=$(RPI) pilinux
	$(MAKE) install_python_dev

$(PYTHON_DEB): $(SBOM_NAME)
	mkdir -p linux $(POOL_DIR)
	PYVER=python$$(xzcat $< | \
	jq -r ' .packages[]? | select(.name == "python3") | .externalRefs[]? | select(.referenceType == "purl") | .referenceLocator | capture("@(?<version>[^?]+)\\?arch=(?<arch>[^&]+)") | (.version | split(".")[0:2] | join(".")) ') ; \
	echo "$${PYVER}" > $(PYTHON_VER) ; \
	xzcat $< | \
	jq -r --arg pyver "$${PYVER}" ' .packages[]? | select(.name == $$pyver) | .externalRefs[]? | select(.referenceType == "purl") | .referenceLocator | capture("@(?<version>[^?]+)\\?arch=(?<arch>[^&]+)") | .version |= gsub("%2B"; "+") | "\($$pyver)-dev_\(.version)_\(.arch).deb", "lib\($$pyver)-dev_\(.version)_\(.arch).deb", "lib\($$pyver)-minimal_\(.version)_\(.arch).deb"' > $@ ; \
	DATE_STAMP=$$(echo "$<" | grep -oE '[0-9]{4}-[0-9]{2}-[0-9]{2}' | tr -d '-') ; \
	URL="https://snapshot.debian.org/archive/debian/$${DATE_STAMP}T000000Z/pool/main/p/python3.11/" ; \
	echo "Downloading from: $${URL}" ; \
	for f in $$(cat $@); do \
		wget --no-iri -nc -P $(POOL_DIR) "$${URL}$${f}" ; \
	done

install_python_dev: $(PYTHON_DEB)
	for i in $$(cat $<); do \
	  echo "DEB=$${i}" ; \
	  ar p $(POOL_DIR)/$${i}  data.tar.xz| tar -xJ -C linux ; \
	done
	touch $@


$(LINUX_HEADER_PACKAGE_FILE):  $(SBOM_NAME)
	mkdir -p linux
	xzcat $< | jq -r --arg suffix "$(ARCH_SUFFIX)" \
	 '.packages[] | select(.name | (test("^linux-headers-[0-9]+\\.[0-9]+\\.[0-9]+\\+")  and endswith($$suffix)) ) | .name' > $@

$(GDBSERVER_FILE):	$(SBOM_NAME)
	mkdir -p linux
	xzcat $< |  \
	jq -r ' .packages[]?  | select(.name == "gdb") | .externalRefs[]?  | select(.referenceType == "purl") | .referenceLocator | capture("@(?<version>[^?]+)\\?arch=(?<arch>[^&]+)") | "gdbserver_\(.version)_\(.arch).deb" ' > $@
	wget -nc -P $(POOL_DIR) \
	    "https://ftp.debian.org/debian/pool/main/g/gdb/$$(cat $@)"

$(LINUXVER_FILE): $(LINUX_HEADER_PACKAGE_FILE)
	sed -E 's/^linux-headers-([0-9]+\.[0-9]+\.[0-9]+).*/\1/' \
	  $< > $@

$(LINUX_LIBC):  $(LINUXVER_FILE)
	mkdir -p linux
	VER=$$(xz -dc $(SBOM_NAME) | jq -r '.packages[]? | select(.name == "libc6") | .versionInfo') ;\
	VER_CLEAN=$${VER#*:} ;\
	echo $$VER_CLEAN > $@ ; \
	wget -nc -P $(POOL_DIR) https://archive.raspberrypi.org/debian/pool/main/g/glibc/libc6_$${VER_CLEAN}_$(RASPIOS_TARG).deb ; \
	ar p $(POOL_DIR)/libc6_$${VER_CLEAN}_$(RASPIOS_TARG).deb data.tar.gz| tar -xz -C linux ;

$(LINUX_LIBCDEV):  $(LINUXVER_FILE)
	mkdir -p linux
	VER=$$(xz -dc $(SBOM_NAME) | jq -r '.packages[]? | select(.name == "libc6-dev") | .versionInfo') ;\
	VER_CLEAN=$${VER#*:} ;\
	echo $$VER_CLEAN > $@ ; \
	wget -nc -P $(POOL_DIR) https://archive.raspberrypi.org/debian/pool/main/g/glibc/libc6-dev_$${VER_CLEAN}_$(RASPIOS_TARG).deb ; \
	ar p $(POOL_DIR)/libc6-dev_$${VER_CLEAN}_$(RASPIOS_TARG).deb data.tar.gz| tar -xz -C linux ;


$(LINUX_LINUX_LIBCDEV):  $(LINUXVER_FILE)
	mkdir -p linux
	VER=$$(xz -dc $(SBOM_NAME) | jq -r '.packages[]? | select(.name == "linux-libc-dev") | .versionInfo') ;\
	VER_CLEAN=$${VER#*:} ;\
	echo $$VER_CLEAN > $@ ; \
	wget -nc -P $(POOL_DIR) https://archive.raspberrypi.org/debian/pool/main/l/linux/linux-libc-dev_$${VER_CLEAN}_all.deb ; \
	ar p $(POOL_DIR)/linux-libc-dev_$${VER_CLEAN}_all.deb data.tar.xz| tar -xJ -C linux ;

$(LINUX_HEADERNAME):  $(LINUXVER_FILE)
	mkdir -p linux
	VER=$$(xz -dc $(SBOM_NAME) | jq -r '.packages[]? | select(.name == "linux-headers-$(shell cat $(LINUXVER_FILE))+rpt-common-rpi") | .versionInfo') ;\
	VER_CLEAN=$${VER#*:} ;\
	echo $$VER_CLEAN > $@ ; \
	wget -nc -P $(POOL_DIR) https://archive.raspberrypi.org/debian/pool/main/l/linux/linux-headers-$(shell cat $(LINUXVER_FILE))+rpt-rpi-$(ARCH_SUFFIX)_$${VER_CLEAN}_$(RASPIOS_TARG).deb ; \
	ar p $(POOL_DIR)/linux-headers-$(shell cat $(LINUXVER_FILE))+rpt-rpi-$(ARCH_SUFFIX)_$${VER_CLEAN}_$(RASPIOS_TARG).deb data.tar.xz| tar -xJ -C linux ;\
	wget -nc -P $(POOL_DIR) https://archive.raspberrypi.org/debian/pool/main/l/linux/linux-headers-$(shell cat $(LINUXVER_FILE))+rpt-common-rpi_$${VER_CLEAN}_all.deb ;\
	ar p $(POOL_DIR)/linux-headers-$(shell cat $(LINUXVER_FILE))+rpt-common-rpi_$${VER_CLEAN}_all.deb data.tar.xz | tar -xJ -C linux
	

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
	$(MAKE) RPI=5  FROM_PI_TARGET=1 install
	touch $@

pi4:
	$(MAKE) RPI=4  FROM_PI_TARGET=1 install
	touch $@

pi3:
	$(MAKE) RPI=3  FROM_PI_TARGET=1 install
	touch $@

SYMVERS=linux/usr/src/linux-headers-$(shell cat $(LINUXVER_FILE))+rpt-rpi-$(ARCH_SUFFIX)/Module.symvers
DOT_CONFIG=linux/usr/src/linux-headers-$(shell cat $(LINUXVER_FILE))+rpt-rpi-$(ARCH_SUFFIX)/.config

pilinux: $(DEPEND) $(TOOLDIR)$(CROSS_COMPILE)gcc linux/.linux_src $(LINUX_HEADERNAME) $(GDBSERVER_FILE)
	echo OS=$(OS) $(SYMVERS) $(DOT_CONFIG) DEPEND=$(DEPEND)
	cp $(SYMVERS) linux/usr/src/linux
	cp $(DOT_CONFIG) linux/usr/src/linux
	( cd linux/usr/src/linux ;\
	KERNEL=$(KERNEL_NAME) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) $(MAKE) olddefconfig prepare modules_prepare; \
	)
	$(MAKE) RPI=$(RPI) SBOM_URL=$(SBOM_URL) $(BUILD_CONFIG)
	$(MAKE) exboard.dtbo

$(BUILD_CONFIG):
	mkdir -p $(BUILD_DIR)
	echo "BUILT_RPI := $(RPI)" > $(BUILD_CONFIG)
	echo "BUILT_SBOM_URL := $(SBOM_URL)" >> $(BUILD_CONFIG)


exboard.dtbo: dts/exboard.dtso
	cpp -nostdinc -undef -D__DTS__ -x assembler-with-cpp \
	-Ilinux/usr/src/linux/scripts/dtc/include-prefixes  dts/exboard.dtso | \
	linux/usr/src/linux/scripts/dtc/dtc -O dtb -o exboard.dtbo

veryclean:
	$(MAKE) clean
	rm -fr exboard.dtbo
	rm  -fr $(POOL_DIR) 
	rm -fr linux
	rm -f install_python_dev
	rm -f pi3 pi4 pi5
clean:
	(cd modules; $(MAKE) clean)
	(cd C; $(MAKE) clean)
	(cd Python; $(MAKE) clean)

################################
# suffix rules
################################
.SUFFIXES:  .man

%.man:
	man -C $(MANDB) $*
	# MANPATH=$(PKGWD)/share/man man -C $(MANDB) $*
