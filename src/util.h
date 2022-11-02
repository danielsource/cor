#define LENGTH(x) (sizeof (x) / sizeof (x[0]))

typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_ERROR,
    LOG_FATAL,
    LOG_QUIET_FATAL,
    LOG_NONE
} LogType;

void die(const char *fmt, ...);
size_t strnlen(const char *s, size_t maxlen);
void set_trace_log_level(LogType t);
void trace_log(LogType t, const char *fmt, ...);
