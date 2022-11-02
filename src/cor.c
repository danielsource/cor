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

static const Option options[] = {
    {'c', "no-color", NO_COLOR,     "Disable color in output"},
    {'n', "number",   NUMBER_LINES, "Number output lines"},
    {'h', "help",     SHOW_HELP,    "Display this help"},
    {'v', "version",  SHOW_VERSION, "Display version information"},
};

static const char *usage = "Usage: cor [OPTION]... [--] [FILE]...\n";

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

int
main(int argc, char **argv) {
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
            /* TODO: o = handle_char_option(argv[i] + 1, o); */
        } else if (a[i] & IS_STR_OPTION) {
            trace_log(LOG_DEBUG, "Got the string option '%s'", argv[i]);
            /* TODO: o = handle_str_option(argv[i] + 2, o); */
        }
    }

    if (o & INVALID_OPTION) {
        exitcode = EXIT_FAILURE;
        fputs(usage, stdout);
    } else if (o & (SHOW_HELP|INVALID_OPTION)) {
        fputs(usage, stdout);
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
                IS_CHAR_OPTION | IS_STR_OPTION))
            continue;
        else if (a[i] & IS_STDIN)
            trace_log(LOG_DEBUG, "Reading stdin");
        else if (a[i] & IS_FILE)
            trace_log(LOG_DEBUG, "Reading '%s'", argv[i]);
        /* TODO: read files and print them */
    }

finish:
    free(a);
    return exitcode;
}
