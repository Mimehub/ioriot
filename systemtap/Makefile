KERNEL ?= $(shell uname -r)
PROCESSOR ?= $(shell uname -p)
DESTDIR ?= /opt/ioreplay/systemtap/$(KERNEL)
UPDATEURI = http://debuginfo.centos.org/7/$(PROCESSOR)
DOWNLOADIR ?= ./downloads
all: prepare compile
prepare:
	sed 's/execname() != "stapio"/pid() == target()/' ./src/ioreplay.stp > ./src/targetedioreplay.stp
	sed 's/execname() != "stapio"/execname() == "java"/' ./src/ioreplay.stp > ./src/javaioreplay.stp
compile:
	@echo Crosscompiling for Kernel version $(KERNEL)
	for stp in ioreplay javaioreplay targetedioreplay; do \
		stap -v ./src/$$stp.stp -p 4 -r $(KERNEL) -m $$stp \
		-D MAXSTRINGLEN=255 -D MAXACTION=10000 -D MAXSKIPPED=10000\
		-g --suppress-time-limits --suppress-handler-errors; \
	done
testsystemtap:
	stap -v -e 'probe vfs.read {printf("read performed\n"); exit()}'
clean:
	@echo Cleaning modules
	test -f ioreplay.ko && rm -v ioreplay.ko || exit 0
	test -f javaioreplay.ko && rm -v javaioreplay.ko || exit 0
	test -f targetedioreplay.ko && rm -v targetedioreplay.ko || exit 0
install:
	test -d $(DESTDIR) || mkdir -p $(DESTDIR)
	test -f ioreplay.ko && cp -v ioreplay.ko $(DESTDIR)/ || exit 0
	test -f javaioreplay.ko && cp -v javaioreplay.ko $(DESTDIR)/ || exit 0
	test -f targetedioreplay.ko && cp -v targetedioreplay.ko $(DESTDIR)/ || exit 0
uninstall:
	test ! -z "$(DESTDIR)" && test -d $(DESTDIR)/ && find $(DESTDIR) -name \*.ko -delete || exit 0
deinstall: uninstall
debuginfodownload:
	test -d $(DOWNLOADIR) || mkdir -p $(DOWNLOADIR)
	test -f $(DOWNLOADIR)/kernel-debuginfo-$(KERNEL).rpm || \
		wget -P $(DOWNLOADIR) $(UPDATEURI)/kernel-debuginfo-$(KERNEL).rpm
	test -f $(DOWNLOADIR)/kernel-debuginfo-common-$(PROCESSOR)-$(KERNEL).rpm || \
		wget -P $(DOWNLOADIR) $(UPDATEURI)/kernel-debuginfo-common-$(PROCESSOR)-$(KERNEL).rpm
debuginfolocalinstall:
	#yum remove kernel-debuginfo kernel-debuginfo-common-$(PROCESSOR)
	yum localinstall $(DOWNLOADIR)/kernel-debuginfo-common-$(PROCESSOR)-$(KERNEL).rpm \
		$(DOWNLOADIR)/kernel-debuginfo-$(KERNEL).rpm
todo:
	fgrep TODO ./src/*
