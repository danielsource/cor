#include <ctype.h>
#include <stdio.h>
#include <string.h>

/* Comment this out to make the hex triplets uppercase: */
#define CFG_USE_LOWERCASE

#define CFG_MAX_LINE_LENGTH BUFSIZ

typedef enum {
  ERROR_FOPEN = 1,
  ERROR_LINE_LENGTH = 2,
  ERROR_FGETS = 4
} exit_status;

typedef enum {
  HEX_NONE = 0,
  HEX_RGB,
  HEX_RGBA,
  HEX_RRGGBB,
  HEX_RRGGBBAA
} hex_color_type;

/* Color */
typedef struct {
  unsigned char r, g, b;
} RGB;

/* Returns whether a RGB color is bright based on HSP
 * (http://alienryderflex.com/hsp.html) */
int is_color_bright(RGB clr) {
  return (clr.r * clr.r * .299f) + (clr.g * clr.g * .587f) +
             (clr.b * clr.b * .114f) >
         127.5f * 127.5f;
}

hex_color_type get_hex_color(char *s, RGB *clr) { return 0; }

int print_hex_color(hex_color_type t, RGB clr) { return 0; }

/* Outputs the file contents with colored hex triplets (e.g., #f00 with a
 * red background). Returns non-zero in case of error/warning. */
exit_status cor(FILE *fp) {
  char line[CFG_MAX_LINE_LENGTH + 2];
  int i;
  RGB clr;
  hex_color_type t;
  exit_status sts = 0;

  line[sizeof(line) - 2] = '\0';
  clearerr(fp);
  while (fgets(line, sizeof(line), fp)) {
    if (ferror(fp)) {
      sts |= ERROR_FGETS;
      return sts;
    }

    if (line[sizeof(line) - 2] != '\0') {
      fprintf(
          stderr,
          "WARNING: line is as long or longer than the buffer capacity (%lu)\n",
          sizeof(line) - 1);
      sts |= ERROR_LINE_LENGTH;
      line[sizeof(line) - 2] = '\0';
    }

    if (line[0] == '#' && (t = get_hex_color(&line[1], &clr))) {
      i = print_hex_color(t, clr);
    } else {
      putchar(line[0]);
      i = 1;
    }

    while (line[i]) {
      /* TODO: add behind char checking */
      if (line[i] == '#' && (t = get_hex_color(&line[i + 1], &clr))) {
        i += print_hex_color(t, clr);
        continue;
      }
      putchar(line[i++]);
    }
  }
  return sts;
}

void test_colors(void) {
  int i, j, n;

  /* 0-9 */
  for (n = 0; n < 10; ++n)
    printf("\033[%dm %3d\033[m", n, n);
  putchar('\n');

  /* 10 is default font and 11-19 is alternative font (not widely supported) */

  /* 20-108 */
  for (i = 2; i < 11; ++i) {
    for (j = 0; j < 10; ++j) {
      n = 10 * i + j;
      if (n > 107)
        break;
      printf("\033[%dm %3d\033[m", n, n);
    }
    putchar('\n');
  }

  printf("\033[38;2;255;100;0m%s\033[0m\n",
         "This is orange if true color is supported.");
}

int main(int argc, char **argv) {
  FILE *fp;
  int i, use_opts = 1;
  exit_status sts = 0;

  if (argc == 1) {
    return cor(stdin);
  }

  for (i = 1; i < argc; ++i) {
    if (use_opts) {
      if (strcmp(argv[i], "--help") == 0) {
        puts("Usage: cor [--test] [FILE...]");
        return sts;
      } else if (strcmp(argv[i], "--test") == 0) {
        test_colors();
        return sts;
      } else if (strcmp(argv[i], "--") == 0) {
        use_opts = 0;
        continue;
      }
    }

    if (!(fp = fopen(argv[i], "r"))) {
      sts |= ERROR_FOPEN;
      continue;
    }
    sts |= cor(fp);
    fclose(fp);
  }
  return sts;
}
