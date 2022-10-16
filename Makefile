include config.mk

sources = $(wildcard src/*.c)
headers = $(wildcard src/*.h)
objects = $(sources:.c=.o)

all: $(program)

clean:
	rm -f $(program) $(objects)

debug: CFLAGS  = -g -O0 -std=c99 -Wall -Wextra -Wpedantic -DDEBUG
debug: LDFLAGS =
debug: version := $(version)+debug
debug: all

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(program) $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/$(program)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(program)

$(program): $(objects)
	$(CC) -o $@ $(objects) $(LDFLAGS)

src/%.o: src/%.c $(headers)
	$(CC) -o $@ -c $(CFLAGS) \
		-DPROGRAM_NAME=\"$(program)\" \
		-DPROGRAM_VERSION=\"$(version)\" $<

.PHONY: all clean debug install uninstall
