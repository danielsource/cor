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
int_to_rgb(struct RGB *rgb, int i) {
	rgb->r = i >> 16;
	rgb->g = i >> 8 & 0xff;
	rgb->b = i & 0xff;
}

static void
print_six_digit_hex_color(char *buf) {
	struct RGB rgb;
	char color[7];
	strncpy(color, &buf[1], 6);
	color[6] = '\0';
	int_to_rgb(&rgb, strtol(color, NULL, 16));
	if (is_color_dark(rgb))
		printf("\033[38;2;0;0;0m");
	else
		printf("\033[38;2;255;255;255m");
	printf("\033[48;2;%d;%d;%dm", rgb.r, rgb.g, rgb.b);
	putchar(*buf);
	putchar(buf[1]);
	putchar(buf[2]);
	putchar(buf[3]);
	putchar(buf[4]);
	putchar(buf[5]);
	putchar(buf[6]);
}

static void
print_three_digit_hex_color(char *buf) {
	struct RGB rgb;
	char color[7];
	color[0] = buf[1];
	color[1] = buf[1];
	color[2] = buf[2];
	color[3] = buf[2];
	color[4] = buf[3];
	color[5] = buf[3];
	color[6] = '\0';
	int_to_rgb(&rgb, strtol(color, NULL, 16));
	if (is_color_dark(rgb))
		printf("\033[38;2;0;0;0m");
	else
		printf("\033[38;2;255;255;255m");
	printf("\033[48;2;%d;%d;%dm", rgb.r, rgb.g, rgb.b);
	putchar(*buf);
	putchar(buf[1]);
	putchar(buf[2]);
	putchar(buf[3]);
}

static void
print_file(FILE *fp) {
	const int bufsize = 4096;
	char buf[bufsize];
	int i;
	while (fgets(buf, bufsize, fp) != NULL) {
		for (i = 0; i < bufsize && buf[i]; i++) {
			if (i < bufsize-7 && buf[i] == '#' &&
					(isxdigit(buf[i+1]) &&
					 isxdigit(buf[i+2]) &&
					 isxdigit(buf[i+3]) &&
					 isxdigit(buf[i+4]) &&
					 isxdigit(buf[i+5]) &&
					 isxdigit(buf[i+6]) &&
					 !isxdigit(buf[i+7]))) {
				print_six_digit_hex_color(&buf[i]);
				printf("\033[0m");
				i += 6;
			} else if (i < bufsize-9 && buf[i] == '#' &&
					(isxdigit(buf[i+1]) &&
					 isxdigit(buf[i+2]) &&
					 isxdigit(buf[i+3]) &&
					 isxdigit(buf[i+4]) &&
					 isxdigit(buf[i+5]) &&
					 isxdigit(buf[i+6]) &&
					 isxdigit(buf[i+7]) &&
					 isxdigit(buf[i+8]) &&
					 !isxdigit(buf[i+9]))) {
				print_six_digit_hex_color(&buf[i]);
				printf("%c%c\033[0m", buf[i+7], buf[i+8]);
				i += 8;
			} else if (i < bufsize-4 && buf[i] == '#' &&
					(isxdigit(buf[i+1]) &&
					 isxdigit(buf[i+2]) &&
					 isxdigit(buf[i+3]) &&
					 !isxdigit(buf[i+4]))) {
				print_three_digit_hex_color(&buf[i]);
				printf("\033[0m");
				i += 3;
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
		if (strcmp(argv[i], "-") == 0) {
			print_file(stdin);
			continue;
		}
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
