VERSION	= 0.9

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

