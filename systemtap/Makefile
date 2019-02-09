KERNEL ?= $(shell uname -r)
PROCESSOR ?= $(shell uname -p)
DESTDIR ?= /opt/ioriot/systemtap/$(KERNEL)
STP = $(wildcard src/*.stp)
MODULES = $(subst src/,,$(subst .stp,,$(STP)))
all: compile
compile: $(MODULES)
prepare:
	sed 's/execname() != "stapio"/pid() == target()/' ./src/ioriot.stp > ./src/targetedioriot.stp
	sed 's/execname() != "stapio"/execname() == "java"/' ./src/ioriot.stp > ./src/javaioriot.stp
$(MODULES): prepare
	stap -v ./src/$@.stp -p 4 -r $(KERNEL) -m $@ \
		-D MAXSTRINGLEN=255 -D MAXACTION=10000 -D MAXSKIPPED=10000 \
		-g --suppress-time-limits --suppress-handler-errors
testsystemtap:
	stap -v -e 'probe vfs.read {printf("read performed\n"); exit()}'
clean:
	@echo Cleaning modules
	find . -name \*.ko -delete || exit 0
install:
	test -d $(DESTDIR) || mkdir -p $(DESTDIR)
	cp -vp *.ko $(DESTDIR)/
uninstall:
	test ! -z "$(DESTDIR)" && test -d $(DESTDIR)/ && find $(DESTDIR) -name \*.ko -delete || exit 0
deinstall: uninstall
todo:
	fgrep TODO ./src/*
