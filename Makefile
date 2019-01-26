KERNEL ?= $(shell uname -r)
DESTDIR=/opt/ioriot
PWD=$(shell pwd)
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
man:
	ronn ./docs/ioriot.1.ronn
test:
	$(MAKE) -C ioriot test
dockerbuild:
	bash -c 'test ! -d $(PWD)/docker/opt/ && mkdir -p $(PWD)/docker/opt/; exit 0'
	bash -c 'test -f /etc/fedora-release && sudo chcon -Rt svirt_sandbox_file_t $(PWD)/docker/opt; exit 0'
	bash -c 'test -f /etc/centos-release && sudo chcon -Rt svirt_sandbox_file_t $(PWD)/docker/opt; exit 0'
	bash -c 'test -f /etc/redhat-release && sudo chcon -Rt svirt_sandbox_file_t $(PWD)/docker/opt; exit 0'
	sed s/KERNEL/$(KERNEL)/ Dockerfile.in > Dockerfile
	docker build . -t ioriot:$(KERNEL)
	docker run -v $(PWD)/docker/opt:/opt -e 'KERNEL=$(KERNEL)' -it ioriot:$(KERNEL) make all test install
dockerclean:
	bash -c 'test -d $(PWD)/docker && rm -Rfv $(PWD)/docker; exit 0'
jenkins:
	bash -c 'test ! -d $(PWD)/docker/opt/ && mkdir -p $(PWD)/docker/opt/; exit 0'
	bash -c 'test -f /etc/fedora-release && sudo chcon -Rt svirt_sandbox_file_t $(PWD)/docker/opt; exit 0'
	bash -c 'test -f /etc/centos-release && sudo chcon -Rt svirt_sandbox_file_t $(PWD)/docker/opt; exit 0'
	bash -c 'test -f /etc/redhat-release && sudo chcon -Rt svirt_sandbox_file_t $(PWD)/docker/opt; exit 0'
	sed s/KERNEL/$(KERNEL)/ Dockerfile.in > Dockerfile
	docker build . -t ioriot:latest
	docker run -v $(PWD)/docker/opt:/opt -e 'KERNEL=$(KERNEL)' -it ioriot:latest make all test install
