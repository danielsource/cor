#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CFG_MAX_LINE_LENGTH BUFSIZ

/* Unsigned char (8 bits) */
typedef unsigned char u8;

typedef enum {
  ERROR_FOPEN = 1,
  ERROR_INVALID_OPT = 2,
  ERROR_LINE_LENGTH = 4,
  ERROR_FGETS = 8
} exit_status;

/* Hexadecimal triplet color types.
 * The enum constants use the length as values (e.g. "#0f0" -> length 4). */
typedef enum {
  HEX_NONE = 0,
  HEX_RGB = 4,     /* Ex: #0f0 */
  HEX_RGBA = 5,    /* Ex: #00f7 */
  HEX_RRGGBB = 7,  /* Ex: #225577 */
  HEX_RRGGBBAA = 9 /* Ex: #deadbeef */
} hex_color_type;

/* Red, Green and Blue color */
typedef struct {
  u8 r, g, b;
} Rgb;

/* Inserts values into RGB color struct if a hex triplet is
 * found in the string. Returns the type of the hex triplet. */
hex_color_type get_hex_color(const char *s, Rgb *clr) {
  hex_color_type t = HEX_NONE;
  char rgb_str[3][3];

  if (!isxdigit((u8)s[0]) || !isxdigit((u8)s[1]) || !isxdigit((u8)s[2]))
    return HEX_NONE;
  else if (!isalnum((u8)s[3]))
    t = HEX_RGB;
  else if (!isxdigit((u8)s[3]))
    return HEX_NONE;
  else if (!isalnum((u8)s[4]))
    t = HEX_RGBA;
  else if (!isxdigit((u8)s[4]) || !isxdigit((u8)s[5]))
    t = HEX_NONE;
  else if (!isalnum((u8)s[6]))
    t = HEX_RRGGBB;
  else if (!isxdigit((u8)s[6]) || !isxdigit((u8)s[7]))
    return HEX_NONE;
  else if (!isalnum((u8)s[8]))
    t = HEX_RRGGBBAA;
  else
    return HEX_NONE;

  if (t == HEX_RGB || t == HEX_RGBA) {
    rgb_str[0][0] = s[0];
    rgb_str[0][1] = s[0];
    rgb_str[0][2] = '\0';

    rgb_str[1][0] = s[1];
    rgb_str[1][1] = s[1];
    rgb_str[1][2] = '\0';

    rgb_str[2][0] = s[2];
    rgb_str[2][1] = s[2];
    rgb_str[2][2] = '\0';
  } else if (t == HEX_RRGGBB || t == HEX_RRGGBBAA) {
    rgb_str[0][0] = s[0];
    rgb_str[0][1] = s[1];
    rgb_str[0][2] = '\0';

    rgb_str[1][0] = s[2];
    rgb_str[1][1] = s[3];
    rgb_str[1][2] = '\0';

    rgb_str[2][0] = s[4];
    rgb_str[2][1] = s[5];
    rgb_str[2][2] = '\0';
  }

  clr->r = strtol(rgb_str[0], NULL, 16);
  clr->g = strtol(rgb_str[1], NULL, 16);
  clr->b = strtol(rgb_str[2], NULL, 16);
  return t;
}

/* Returns whether a RGB color is bright based on HSP
 * (https://alienryderflex.com/hsp.html) */
int is_color_bright(Rgb clr) {
  return (clr.r * clr.r * .299f) + (clr.g * clr.g * .587f) +
             (clr.b * clr.b * .114f) >
         127.5f * 127.5f;
}

/* Outputs the hex triplet with a colored background using "true color"
 * terminal escape codes. Returns the number of chars outputted. */
int print_hex_color(const char *s, hex_color_type t, Rgb bg) {
  if (is_color_bright(bg))
    printf("\033[38;2;0;0;0m\033[48;2;%d;%d;%dm", bg.r, bg.g, bg.b);
  else
    printf("\033[38;2;255;255;255m\033[48;2;%d;%d;%dm", bg.r, bg.g, bg.b);

  printf("#%.*s", t - 1, s);
  fputs("\033[0m", stdout);
  return t;
}

/* Outputs the file contents with colored hex triplets (e.g. #f00 with a
 * red background). If 'extract' is non-zero only display the colors
 * instead of the whole file. Returns non-zero in case of error/warning. */
exit_status cor(FILE *fp, int extract) {
  char line[CFG_MAX_LINE_LENGTH + 2];
  Rgb clr;
  int i;
  hex_color_type t;
  exit_status sts = 0;

  line[sizeof(line) - 2] = '\0';
  clearerr(fp);
  while (fgets(line, sizeof(line), fp)) {
    if (ferror(fp)) {
      perror("cor: fgets");
      return sts |= ERROR_FGETS;
    }

    if (line[sizeof(line) - 2] != '\0') {
      fprintf(
          stderr,
          "cor: WARNING: line is as long or longer than the buffer capacity "
          "(%d)\n",
          CFG_MAX_LINE_LENGTH);
      sts |= ERROR_LINE_LENGTH;
      line[sizeof(line) - 2] = '\0';
    }

    if (line[0] == '#' && (t = get_hex_color(line + 1, &clr))) {
      i = print_hex_color(line + 1, t, clr);
      if (extract)
        putchar('\n');
    } else {
      if (!extract)
        putchar(line[0]);
      i = 1;
    }

    while (line[i]) {
      if (!isalnum((u8)line[i - 1]) && line[i] == '#' &&
          (t = get_hex_color(line + i + 1, &clr))) {
        i += print_hex_color(line + i + 1, t, clr);
        if (extract)
          putchar('\n');
        continue;
      }
      if (!extract)
        putchar(line[i]);
      ++i;
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

  puts("\033[38;2;255;100;0mThis is orange if true color is "
       "supported.\033[0m");
}

int main(int argc, char **argv) {
  FILE *fp;
  int i, file_given = 0, use_opts = 1, extract = 0, show_filename = 0;
  exit_status sts = 0;

  for (i = 1; i < argc; ++i) {
    if (use_opts && argv[i][0] == '-') {
      if (argv[i][1] == '\0') {
        sts |= cor(stdin, extract);
        continue;
      } else if (argv[i][1] == '-' && argv[i][2] == '\0') {
        use_opts = 0;
      } else if (strcmp(argv[i] + 2, "extract") == 0) {
        extract = 1;
      } else if (strcmp(argv[i] + 2, "show-filename") == 0) {
        show_filename = 1;
      } else if (strcmp(argv[i] + 2, "test") == 0) {
        test_colors();
        return sts;
      } else {
        puts("Usage: cor [--extract | --show-filename | --test] [FILE...]");
        if (strcmp(argv[i] + 2, "help") != 0)
          sts |= ERROR_INVALID_OPT;
        return sts;
      }
      continue;
    }

    file_given = 1;
    if (!(fp = fopen(argv[i], "r"))) {
      fputs("cor: ", stderr);
      perror(argv[i]);
      sts |= ERROR_FOPEN;
      continue;
    }
    if (show_filename)
      printf("%s:\n", argv[i]);
    sts |= cor(fp, extract);
    fclose(fp);
  }

  if (!file_given)
    return cor(stdin, extract);
  return sts;
}
