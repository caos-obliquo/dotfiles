# wclipmenu — wmenu-based clipboard picker over kaprica (Wayland)
# Zero-footprint, on-demand binary. Depends on libc + external wmenu + kaprica.

VERSION = 0.1.0

CC      = cc
CFLAGS  = -std=c11 -Wall -Wextra -O2 -g
LDFLAGS =

PREFIX  = /usr/local
BINDIR  = $(PREFIX)/bin
MANPREFIX = $(PREFIX)/share/man

SRC = src/wclipmenu.c src/kapc.c src/entries.c src/image.c src/wmenu.c src/commands.c
BIN = bin/wclipmenu
MAN = wclipmenu.1

all: $(BIN)

$(BIN): $(SRC)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $(SRC) $(LDFLAGS)

clean:
	rm -rf bin

install: $(BIN)
	install -Dm755 $(BIN) $(DESTDIR)$(BINDIR)/wclipmenu
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	cp -f $(MAN) $(DESTDIR)$(MANPREFIX)/man1/$(MAN)
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/$(MAN)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/wclipmenu
	rm -f $(DESTDIR)$(MANPREFIX)/man1/$(MAN)

.PHONY: all clean install uninstall
