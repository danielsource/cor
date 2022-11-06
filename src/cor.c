#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

/* Argument flags */
#define IS_OPTION_END  0x01
#define IS_FILE        0x02
#define IS_STDIN       0x04
#define IS_CHAR_OPTION 0x08
#define IS_STR_OPTION  0x10

/* Option flags */
#define READ_FILES      0x01
#define NO_COLOR        0x02
#define NUMBER_LINES    0x04
#define SHOW_HELP       0x08
#define SHOW_VERSION    0x10
#define INVALID_OPTION  0x20
#define NEGATIVE_OPTION 0x40

typedef uint8_t ArgFlags;
typedef uint8_t OptionFlags;

typedef struct {
    char c;
    char *str;
    OptionFlags flag;
    char *description;
} Option;

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} RGB;

static const Option g_options[] = {
    {'c', "no-color", NO_COLOR,     "Disable color in output"},
    {'n', "number",   NUMBER_LINES, "Number output lines"},
    {'h', "help",     SHOW_HELP,    "Display this help"},
    {'v', "version",  SHOW_VERSION, "Display version information"},
};

void show_help(FILE *f) {
    int i;
    fputs("Usage: cor [OPTION]... [--] [FILE]...\n"
            "Preview colors when outputting files in the terminal.\n"
            "\n"
            "With no FILE, or when FILE is -, read standard input.\n"
            "\n", f);
    for (i = 0; i < LENGTH(g_options); i++)
        fprintf(f, "  -%c, --%-*s%s\n",
                g_options[i].c,
                14,
                g_options[i].str,
                g_options[i].description);
    fputs("\n"
            "Inspired by the 'cat' Unix utility.\n"
            "Program page: https://github.com/danielsource/cor\n", f);
}

ArgFlags
identify_arg(const char *arg) {
    size_t len = strnlen(arg, 3);
    if (len == 1 &&
            arg[0] == '-')
        return IS_STDIN;
    else if (len == 2 &&
            arg[0] == '-' &&
            arg[1] == '-')
        return IS_OPTION_END;
    else if (len > 1 && arg[0] == '-' &&
            arg[1] != '-')
        return IS_CHAR_OPTION;
    else if (len > 2 && arg[0] == '-' &&
            arg[1] == '-')
        return IS_STR_OPTION;
    return IS_FILE;
}

OptionFlags
handle_char_option(char *options, OptionFlags o) {
    int i, j;
    bool valid;
    for (i = 0; options[i]; i++) {
        valid = false;
        for (j = 0; j < LENGTH(g_options); j++) {
            if (options[i] == g_options[j].c) {
                if (g_options[j].flag & NEGATIVE_OPTION)
                    o ^= g_options[j].flag;
                else
                    o |= g_options[j].flag;
                valid = true;
            }
        }
        if (!valid) {
            trace_log(LOG_ERROR, "cor: Invalid option '-%c'", options[i]);
            return INVALID_OPTION;
        }
    }
    return o;
}

OptionFlags
handle_str_option(char *option, OptionFlags o) {
    int i;
    bool valid;
    for (i = 0; i < LENGTH(g_options); i++) {
        if (!strcmp(option, g_options[i].str)) {
            if (g_options[i].flag & NEGATIVE_OPTION)
                o ^= g_options[i].flag;
            else
                o |= g_options[i].flag;
            valid = true;
        }
    }
    if (!valid) {
        trace_log(LOG_ERROR, "cor: Invalid option '--%s'", option);
        return INVALID_OPTION;
    }
    return o;
}

bool
is_color_dark(RGB rgb) {
    float hsp; /* HSP Equation from https://alienryderflex.com/hsp.html */
    hsp = sqrtf(0.299f * (rgb.r * rgb.r) +
            0.587f * (rgb.g * rgb.g) +
            0.114f * (rgb.b * rgb.b));
    return hsp > 127.5f;
}

void
print_file(FILE *f, OptionFlags o) {
    int c;
    while ((c = fgetc(f)) != EOF) {
        putchar(c);
    }
}

int
main(int argc, char **argv) {
    FILE *f = NULL;
    ArgFlags *a;
    OptionFlags o = 0;
    char *no_color;
    int i, exitcode = EXIT_SUCCESS;

#ifdef DEBUG
    set_trace_log_level(LOG_DEBUG);
#endif

    a = malloc(argc * sizeof(ArgFlags));
    no_color = getenv("NO_COLOR");
    if (no_color != NULL && no_color[0] != '\0') {
        o |= NO_COLOR;
    }

    /* Handle [OPTION]... */
    for (i = 1; i < argc; i++) {
        a[i] = identify_arg(argv[i]);
        if (a[i] & IS_OPTION_END) {
            if (argc - i - 1 > 0) {
                memset(a + i + 1, IS_FILE, argc - i - 1);
                o |= READ_FILES;
            }
            break;
        } else if (a[i] & (IS_FILE | IS_STDIN)) {
            o |= READ_FILES;
            continue;
        } else if (a[i] & IS_CHAR_OPTION) {
            trace_log(LOG_DEBUG, "Got the char option '%s'", argv[i]);
            o = handle_char_option(argv[i] + 1, o);
        } else if (a[i] & IS_STR_OPTION) {
            trace_log(LOG_DEBUG, "Got the string option '%s'", argv[i]);
            o = handle_str_option(argv[i] + 2, o);
        }
    }

    if (o & INVALID_OPTION) {
        exitcode = EXIT_FAILURE;
        fputs("Try 'cor --help' for more information.\n", stderr);
    } else if (o & (SHOW_HELP|INVALID_OPTION)) {
        show_help(stdout);
        goto finish;
    } else if (o & SHOW_VERSION) {
        puts("cor-prerelease");
        goto finish;
    }

    /* Handle [FILE]... */
    if (o ^ READ_FILES)
        trace_log(LOG_DEBUG,
                "Reading stdin because no file was provided");
    else for (i = 1; i < argc; i++) {
        if (a[i] & (IS_OPTION_END |
                IS_CHAR_OPTION | IS_STR_OPTION)) {
            continue;
        } else if (a[i] & IS_STDIN) {
            trace_log(LOG_DEBUG, "Reading stdin");
            f = stdin;
        } else if (a[i] & IS_FILE) {
            trace_log(LOG_DEBUG, "Reading '%s'", argv[i]);
            f = fopen(argv[i], "r");
        }
        if (!f) {
            trace_log(LOG_ERROR, "cor: %s:", argv[i]);
            exitcode = EXIT_FAILURE;
            continue;
        }
        print_file(f, o);
    }

finish:
    free(a);
    return exitcode;
}
