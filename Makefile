cor: cor.c
	cc -o $@ $< -std=c89 -Wall -Wextra -Wpedantic
