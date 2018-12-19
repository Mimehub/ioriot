KERNEL ?= $(shell uname -r)
DESTDIR=/opt/ioriot
all:
	$(MAKE) -C systemtap
	$(MAKE) -C ioriot
install:
	$(MAKE) -C systemtap install
	$(MAKE) -C ioriot install
uninstall:
	test ! -z $(DESTDIR) && test -d $(DESTDIR) && rm -Rfv $(DESTDIR) || exit 0
deinstall: uninstall
clean:
	$(MAKE) -C ioriot clean
	$(MAKE) -C systemtap clean
astyle:
	$(MAKE) -C ioriot astyle
loc:
	wc -l ./systemtap/src/*.stp ./ioriot/src/*.{h,c} ./ioriot/src/*/*.{h,c} | tail -n 1
doxygen:
	doxygen ./docs/doxygen.conf
test:
	$(MAKE) -C ioriot test
dockerbuild:
	sed s/KERNEL/$(KERNEL)/ Dockerfile.in > Dockerfile
	docker build . -t ioriot:$(KERNEL)
	bash -c 'test ! -d /tmp/docker/opt/ && mkdir -p /tmp/docker/opt/; exit 0'
	docker run -v /tmp/docker/opt:/opt -v /tmp/docker/downloads:/ioriot/systemtap/downloads -e 'KERNEL=$(KERNEL)' -it ioriot:$(KERNEL) make all install
