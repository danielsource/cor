cor: cor.c
	c99 -o $@ $< -std=c99 -Wall -Wextra -Wpedantic
