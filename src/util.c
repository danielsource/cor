#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

static LogType log_level = LOG_INFO;

void
die(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    trace_log(LOG_QUIET_FATAL, fmt, args);
    va_end(args);
}

size_t
strnlen(const char *s, size_t maxlen) {
    size_t len;
    for (len = 0; len < maxlen; len++)
        if (!s[len])
            break;
    return len;
}

void
set_trace_log_level(LogType t) {
    log_level = t;
}

void
trace_log(LogType t, const char *fmt, ...) {
    FILE *f = stdout;
    va_list args;
    if (t < log_level)
        return;
    va_start(args, fmt);
    switch (t) {
    case LOG_INFO:
        fputs("INFO: ", f);
        break;
    case LOG_DEBUG:
        fputs("DEBUG: ", f);
        break;
    case LOG_ERROR:
        fputs("ERROR: ", f);
        break;
    case LOG_FATAL:
        fputs("FATAL: ", f);
        break;
    default:
        break;
    }
    vfprintf(f, fmt, args);
    va_end(args);
    if (fmt[0] && fmt[strlen(fmt) - 1] == ':')
        fprintf(f, " %s", strerror(errno));
    else
        putchar('\n');
    fflush(f);
    if (t == LOG_FATAL ||
            t == LOG_QUIET_FATAL)
        exit(EXIT_FAILURE);
}
