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
man:
	ronn ./docs/ioriot.1.ronn
test:
	$(MAKE) -C ioriot test
