CC      ?= gcc
CFLAGS  ?= -Os -ffast-math -std=c99 -Wall -Wextra -Wpedantic
LDFLAGS ?= -s -lm
PREFIX = /usr/local

program = $(notdir $(patsubst %/,%,$(CURDIR)))
sources = $(wildcard src/*.c)
headers = $(wildcard src/*.h)
objects = $(sources:.c=.o)

all: $(program)

clean:
	rm -f $(program) $(objects)

install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(program) $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/$(program)

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/dwm

$(program): $(objects)
	$(CC) -o $@ $(objects) $(LDFLAGS)

src/%.o: src/%.c $(headers)
	$(CC) -o $@ -c $(CFLAGS) $<

.PHONY: all clean install uninstall
