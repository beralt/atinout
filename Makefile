
CC	= gcc
CFLAGS	= -W -Wall -Wextra -Werror \
	-g
LDFLAGS =

all: atinout

atinout: atinout.c
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $^

clean:
	rm -f atinout

