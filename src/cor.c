#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROGRAM_NAME "cor"
#define PROGRAM_USAGE \
	"Usage: " PROGRAM_NAME " [-c] [-h] [-n] [FILE]...\n\n" \
	"  -c   disable colors\n" \
	"  -h   show usage\n" \
	"  -n   number lines\n" \
	"\n" \
	"When FILE is -, read standard input.\n" \
	"\n" \
	"A double dash (--) is used to signal that\n" \
	"any remaining arguments are not options.\n"
#define INVALID_OPTION_FMT \
	PROGRAM_NAME ": invalid option -- '%s'.\n" \
	"Try '" PROGRAM_NAME " -h' for more information.\n"

typedef struct options {
	bool no_color;
	bool numbering;
	int end_of_options;
} Options;

typedef struct rgb {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} RGB;

static bool
is_color_dark(RGB rgb) {
	float hsp; /* HSP Equation from alienryderflex.com/hsp.html */
	hsp = sqrtf(0.299f * (rgb.r * rgb.r) +
			0.587f * (rgb.g * rgb.g) +
			0.114f * (rgb.b * rgb.b));
	return hsp > 127.5f;
}

static void
int_to_rgb(RGB *rgb, int i) {
	rgb->r = i >> 16;
	rgb->g = i >> 8 & 0xff;
	rgb->b = i & 0xff;
}

static void
print_six_digit_hex_color(char *buf) {
	RGB rgb;
	char color[7];
	strncpy(color, &buf[1], 6);
	color[6] = '\0';
	int_to_rgb(&rgb, strtol(color, NULL, 16));
	if (is_color_dark(rgb))
		fputs("\033[38;2;0;0;0m", stdout);
	else
		fputs("\033[38;2;255;255;255m", stdout);
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
	RGB rgb;
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
		fputs("\033[38;2;0;0;0m", stdout);
	else
		fputs("\033[38;2;255;255;255m", stdout);
	printf("\033[48;2;%d;%d;%dm", rgb.r, rgb.g, rgb.b);
	putchar(*buf);
	putchar(buf[1]);
	putchar(buf[2]);
	putchar(buf[3]);
}

static int
print_file(FILE *fp, Options o) {
	static unsigned int numbering = 1;
	const int bufsize = BUFSIZ;
	char buf[bufsize];
	size_t n, i;
	while ((n = fread(buf, sizeof (char), bufsize, fp))) {
		for (i = 0; i < n; i++) {
			if (o.numbering &&
					(i == 0 ||
					 (i > 0 && buf[i-1] == '\n')))
				printf("%6d\t", numbering++);
			if (o.no_color) {
				putchar(buf[i]);
				continue;
			}
			if (i < n-7 && buf[i] == '#' &&
					(isxdigit(buf[i+1]) &&
					 isxdigit(buf[i+2]) &&
					 isxdigit(buf[i+3]) &&
					 isxdigit(buf[i+4]) &&
					 isxdigit(buf[i+5]) &&
					 isxdigit(buf[i+6]) &&
					 !isalnum(buf[i+7]))) {
				print_six_digit_hex_color(&buf[i]);
				fputs("\033[0m", stdout);
				i += 6;
			} else if (i < n-9 && buf[i] == '#' &&
					(isxdigit(buf[i+1]) &&
					 isxdigit(buf[i+2]) &&
					 isxdigit(buf[i+3]) &&
					 isxdigit(buf[i+4]) &&
					 isxdigit(buf[i+5]) &&
					 isxdigit(buf[i+6]) &&
					 isxdigit(buf[i+7]) &&
					 isxdigit(buf[i+8]) &&
					 !isalnum(buf[i+9]))) {
				print_six_digit_hex_color(&buf[i]);
				printf("%c%c\033[0m", buf[i+7],
						buf[i+8]);
				i += 8;
			} else if (i < n-4 && buf[i] == '#' &&
					(isxdigit(buf[i+1]) &&
					 isxdigit(buf[i+2]) &&
					 isxdigit(buf[i+3]) &&
					 !isalnum(buf[i+4]))) {
				print_three_digit_hex_color(&buf[i]);
				fputs("\033[0m", stdout);
				i += 3;
			} else
				putchar(buf[i]);
		}
	}
	return 0;
}

int
main(int argc, char **argv) {
	FILE *fp;
	Options o = {0};
	char *no_color = getenv("NO_COLOR");
	int status = 0, i;
	if (no_color && *no_color != '\0')
		o.no_color = true;
	for (i = 1; i < argc; i++) {
		if (argv[i][0] != '\0' && (argv[i][0] != '-' ||
				argv[i][1] == '\0'))
			continue;
		else if (argv[i][2] != '\0') {
			fprintf(stderr, INVALID_OPTION_FMT, argv[i]);
			return 1;
		}
		switch (argv[i][1]) {
		case '-':
			o.end_of_options = i;
			break;
		case 'c':
			o.no_color = true;
			break;
		case 'h':
			fputs(PROGRAM_USAGE, stdout);
			return 0;
		case 'n':
			o.numbering = true;
			break;
		default:
			fprintf(stderr, INVALID_OPTION_FMT, argv[i]);
			return 1;
		}
		if (o.end_of_options)
			break;
	}
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-' && argv[i][1] == '\0') {
			status |= print_file(stdin, o);
			continue;
		} else if ((!o.end_of_options || i < o.end_of_options)
				&&
				argv[i][0] == '-')
			continue;
		else if ((i == o.end_of_options) &&
				argv[i][0] == '-' && argv[i][0] == '-')
			continue;
		fp = fopen(argv[i], "r");
		if (!fp) {
			fprintf(stderr, PROGRAM_NAME ": '%s': %s.\n",
					argv[i], strerror(errno));
			status |= errno;
			continue;
		}
		status |= print_file(fp, o);
		fclose(fp);
	}
	return status;
}
