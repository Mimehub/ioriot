DESTDIR=/opt/ioreplay
all:
	$(MAKE) -C systemtap
	$(MAKE) -C ioreplay
install:
	$(MAKE) -C systemtap install
	$(MAKE) -C ioreplay install
uninstall:
	test ! -z $(DESTDIR) && test -d $(DESTDIR) && rm -Rfv $(DESTDIR) || exit 0
deinstall: uninstall
clean:
	$(MAKE) -C ioreplay clean
	$(MAKE) -C systemtap clean
astyle:
	$(MAKE) -C ioreplay astyle
loc:
	wc -l ./systemtap/src/*.stp ./ioreplay/src/*.{h,c} ./ioreplay/src/*/*.{h,c} | tail -n 1
doxygen:
	doxygen ./docs/doxygen.conf
