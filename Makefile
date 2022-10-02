CC      ?= gcc
CFLAGS  ?= -std=c99 -g -Wall -Wextra -Wpedantic
LDFLAGS ?= -lm

program = $(notdir $(patsubst %/,%,$(CURDIR)))
sources = $(wildcard src/*.c)
headers = $(wildcard src/*.h)
objects = $(sources:.c=.o)

all: $(program)

clean:
	rm -f $(program) $(objects)

$(program): $(objects)
	$(CC) -o $@ $(objects) $(LDFLAGS)

src/%.o: src/%.c $(headers)
	$(CC) -o $@ -c $(CFLAGS) $<

.PHONY: all clean
