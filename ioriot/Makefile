#DEBUG=-g3 -ggdb3 -pg
NAME=ioriot
LIBS=-pthread
CFLAGS=-Wall -std=gnu99 -pedantic
STATIC=#-static
DESTDIR=/opt/ioriot/bin
SRCS=$(wildcard src/*.c src/*/*.c)
HDRS=$(SRCS:.c=.h)
OBJS=$(SRCS:.c=.o)
all: compile
c: clean all
quick: clean ctags compile sudo_install
cshell: compile
	gdb -ex='break main; run' --args ./$(NAME)
gdbtest: compile
	gdb -ex=run --args ./$(NAME) -U
compile: $(OBJS)
	$(CC) $(STATIC) $(DEBUG) $(LIBS) $(OBJS) -o $(NAME)
%.o: %.c %.h
	$(CC) $(STATIC) $(DEBUG) $(LIBS) -c $(CFLAGS) $< -o $@
clean:
	rm -v ioriot ./src/*.o ./src/*/*.o 2>/dev/null || exit 0
install:
	test ! -d $(DESTDIR) && mkdir -p $(DESTDIR) || exit 0
	cp -v $(NAME) $(DESTDIR)
	@echo "Don't forget to add $(DESTDIR) to your PATH as follows:"
	@echo "	export PATH=\$$PATH:$(DESTDIR)"
uninstall:
	test ! -z "$(DESTDIR)" && test -f $(DESTDIR)/$(NAME) && rm -v $(DESTDIR)/$(NAME) || exit 0
deinstall: uninstall
astyle:
	astyle -n --style=linux src/*.h src/*/*.h
	astyle -n --style=linux src/*.c src/*/*.c
todo:
	fgrep ../TODO ./src/*
ctags:
	ctags ./src/*.{h,c} ./src/*/*.{h,c}
test: clean all
	./ioriot -U
