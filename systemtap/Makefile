KERNEL ?= $(shell uname -r)
PROCESSOR ?= $(shell uname -p)
DESTDIR ?= /opt/ioriot/systemtap/$(KERNEL)
UPDATEURI = http://debuginfo.centos.org/7/$(PROCESSOR)
DOWNLOADIR ?= ./downloads
all: compile
prepare:
	sed 's/execname() != "stapio"/pid() == target()/' ./src/ioriot.stp > ./src/targetedioriot.stp
	sed 's/execname() != "stapio"/execname() == "java"/' ./src/ioriot.stp > ./src/javaioriot.stp
	sed 's/execname() != "stapio"/pid() == target()/' ./src/traceio.stp > ./src/targetedtraceio.stp
	sed 's/execname() != "stapio"/execname() == "java"/' ./src/traceio.stp > ./src/javatraceio.stp
compile: prepare
	@echo Crosscompiling for Kernel version $(KERNEL)
	for stp in ioriot javaioriot targetedioriot; do \
		stap -v ./src/$$stp.stp -p 4 -r $(KERNEL) -m $$stp \
		-D MAXSTRINGLEN=255 -D MAXACTION=10000 -D MAXSKIPPED=10000\
		-g --suppress-time-limits --suppress-handler-errors; \
	done
traceio_compile: prepare
	@echo Crosscompiling for Kernel version $(KERNEL)
	for stp in traceio javatraceio targetedtraceio; do \
		stap -v ./src/$$stp.stp -p 4 -r $(KERNEL) -m $$stp \
		-D MAXSTRINGLEN=255 -D MAXACTION=10000 -D MAXSKIPPED=10000\
		-g --suppress-time-limits --suppress-handler-errors; \
	done
testsystemtap:
	stap -v -e 'probe vfs.read {printf("read performed\n"); exit()}'
clean:
	@echo Cleaning modules
	test -f ioriot.ko && rm -v ioriot.ko || exit 0
	test -f javaioriot.ko && rm -v javaioriot.ko || exit 0
	test -f targetedioriot.ko && rm -v targetedioriot.ko || exit 0
install:
	test -d $(DESTDIR) || mkdir -p $(DESTDIR)
	test -f ioriot.ko && cp -v ioriot.ko $(DESTDIR)/ || exit 0
	test -f javaioriot.ko && cp -v javaioriot.ko $(DESTDIR)/ || exit 0
	test -f targetedioriot.ko && cp -v targetedioriot.ko $(DESTDIR)/ || exit 0
	test -f traceio.ko && cp -v traceio.ko $(DESTDIR)/ || exit 0
	test -f javatraceio.ko && cp -v javatraceio.ko $(DESTDIR)/ || exit 0
	test -f targetedtraceio.ko && cp -v targetedtraceio.ko $(DESTDIR)/ || exit 0
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
	yum remove kernel-debuginfo kernel-debuginfo-common-$(PROCESSOR)
	yum localinstall $(DOWNLOADIR)/kernel-debuginfo-common-$(PROCESSOR)-$(KERNEL).rpm \
		$(DOWNLOADIR)/kernel-debuginfo-$(KERNEL).rpm
todo:
	fgrep TODO ./src/*
