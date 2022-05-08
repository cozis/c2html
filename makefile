CC = gcc
CFLAGS = -Wall -Wextra -DNDEBUG -O3
# CFLAGS = -Wall -Wextra -g # When debugging

.PHONY: all install clean

all: c2html

c2html: cli.c c2html.c c2html.h
	$(CC) cli.c c2html.c -o $@ $(CFLAGS)

install: c2html
	cp c2html /bin/c2html

clean:
	rm c2html