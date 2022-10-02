#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROGRAM_NAME "cor"

static void
printfile(FILE *fp) {
	const int bufsize = 4096;
	char buf[bufsize], color[7] = {0};
	int i;
	float hsp; /* HSP Equation from alienryderflex.com/hsp.html */
	unsigned int rgb;
	unsigned char r, g, b;
	while (fgets(buf, bufsize, fp) != NULL) {
		for (i = 0; i < bufsize-7 && buf[i]; i++) {
			if (buf[i] != '#' || !(isxdigit(buf[i+1]) &&
					isxdigit(buf[i+2]) &&
					isxdigit(buf[i+3]) &&
					isxdigit(buf[i+4]) &&
					isxdigit(buf[i+5]) &&
					isxdigit(buf[i+6]) &&
					!isxdigit(buf[i+7]))) {
				putchar(buf[i]);
				continue;
			}
			strncpy(color, &buf[i+1], 6);
			rgb = strtol(color, NULL, 16);
			r = rgb >> 16;
			g = rgb >> 8 & 0xff;
			b = rgb & 0xff;
			hsp = sqrtf(0.299f * (r * r) +
					0.587f * (g * g) +
					0.114f * (b * b));
			if (hsp > 127.5f)
				printf("\033[38;2;0;0;0m");
			else
				printf("\033[38;2;255;255;255m");
			printf("\033[48;2;%d;%d;%dm", r, g, b);
			putchar(buf[i]);
			putchar(buf[i+1]);
			putchar(buf[i+2]);
			putchar(buf[i+3]);
			putchar(buf[i+4]);
			putchar(buf[i+5]);
			putchar(buf[i+6]);
			printf("\033[0m");
			i += 6;
		}
	}
}

int
main(int argc, char **argv) {
	FILE *fp;
	int status = 0, i;
	for (i = 1; i < argc; i++) {
		fp = fopen(argv[i], "r+");
		if (!fp) {
			fprintf(stderr, PROGRAM_NAME ": %s: %s.\n",
					argv[i], strerror(errno));
			status = errno;
			continue;
		}
		printfile(fp);
		fclose(fp);
	}
	return status;
}
