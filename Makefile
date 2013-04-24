VERSION	= 0.9

PREFIX	= /usr

CC	= gcc
CFLAGS	= -W -Wall -Wextra -Werror \
	-DVERSION=\"$(VERSION)\" \
	-g
LDFLAGS =

all: atinout atinout.1

atinout: atinout.c
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $^

atinout.1 atinout.1.html: atinout.1.ronn
	ronn $^

clean:
	rm -f atinout atinout.1 atinout.1.html

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp atinout $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/share/man/man1
	cp atinout.1 $(DESTDIR)$(PREFIX)/share/man/man1/atinout.1
