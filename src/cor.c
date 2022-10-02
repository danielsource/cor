#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROGRAM_NAME "cor"

struct RGB {
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

static bool
is_color_dark(struct RGB rgb) {
	float hsp; /* HSP Equation from alienryderflex.com/hsp.html */
	hsp = sqrtf(0.299f * (rgb.r * rgb.r) +
			0.587f * (rgb.g * rgb.g) +
			0.114f * (rgb.b * rgb.b));
	return hsp > 127.5f;
}

static void
print_six_digit_hex_color(struct RGB rgb, char *buf, int index) {
	char color[7] = {0};
	int i = index, rgbint;
	strncpy(color, &buf[i+1], 6);
	rgbint = strtol(color, NULL, 16);
	rgb.r = rgbint >> 16;
	rgb.g = rgbint >> 8 & 0xff;
	rgb.b = rgbint & 0xff;
	if (is_color_dark(rgb))
		printf("\033[38;2;0;0;0m");
	else
		printf("\033[38;2;255;255;255m");
	printf("\033[48;2;%d;%d;%dm", rgb.r, rgb.g, rgb.b);
	putchar(buf[i]);
	putchar(buf[i+1]);
	putchar(buf[i+2]);
	putchar(buf[i+3]);
	putchar(buf[i+4]);
	putchar(buf[i+5]);
	putchar(buf[i+6]);
	printf("\033[0m");
}

static void
print_file(FILE *fp) {
	struct RGB rgb;
	const int bufsize = 4096;
	char buf[bufsize];
	int i;
	while (fgets(buf, bufsize, fp) != NULL) {
		for (i = 0; i < bufsize-7 && buf[i]; i++) {
			if (buf[i] == '#' && (isxdigit(buf[i+1]) &&
					isxdigit(buf[i+2]) &&
					isxdigit(buf[i+3]) &&
					isxdigit(buf[i+4]) &&
					isxdigit(buf[i+5]) &&
					isxdigit(buf[i+6]) &&
					!isxdigit(buf[i+7]))) {
				print_six_digit_hex_color(rgb, buf, i);
				i += 6;
			} else {
				putchar(buf[i]);
			}
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
		print_file(fp);
		fclose(fp);
	}
	return status;
}
