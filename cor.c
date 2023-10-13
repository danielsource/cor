#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Returns whether a RGB color is bright based on HSP
 * (http://alienryderflex.com/hsp.html) */
int is_color_bright(uint8_t r, uint8_t g, uint8_t b) {
  return (r * r * .299f) + (g * g * .587f) + (b * b * 0.114f) > 127.5f * 127.5f;
}

/* Outputs the file contents with colored hex triplets (e.g., #f00 with a red
 * background). Returns non-zero in case of error/warning. */
int cor(FILE *fp) {
  char line[BUFSIZ];
  int status = 0;

  line[sizeof(line) - 2] = '\0';
  clearerr(fp);
  while (fgets(line, sizeof(line), fp)) {
    if (ferror(fp))
      status |= 4;
    if (line[sizeof(line) - 2] != '\0') {
      fprintf(
          stderr,
          "WARNING: line is as long or longer than the buffer capacity (%zu)\n",
          sizeof(line) - 1);
      status |= 2;
      line[sizeof(line) - 2] = '\0';
    }
    fputs(line, stdout);
  }
  return 0;
}

void test_colors(void) {
  int i, j, n;

  /* 0-9 */
  for (n = 0; n < 10; ++n)
    printf("\033[%dm %3d\033[m", n, n);
  putchar('\n');

  /* 10 is default font and 11-19 is alternative font (not widely supported) */

  /* 20-108 */
  for (i = 2; i < 11; i++) {
    for (j = 0; j < 10; j++) {
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
  int i, status = 0;

  if (argc == 1)
    return cor(stdin);

  if (strcmp(argv[1], "--test") == 0) {
    test_colors();
    return 0;
  }

  for (i = 1; i < argc; ++i) {
    if (!(fp = fopen(argv[i], "r"))) {
      status |= 1;
      continue;
    }
    status |= cor(fp);
    fclose(fp);
  }
  return status;
}
