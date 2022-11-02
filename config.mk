CC      = cc
CFLAGS  = -Os -ffast-math -std=c99 -Wall -Wextra -Wpedantic
LDFLAGS = -s -lm
PREFIX  ?= /usr/local

cflags_debug  = -g -O0 -std=c99 -Wall -Wextra -Wpedantic -DDEBUG
ldflags_debug = -lm

program = cor
version = prerelease
