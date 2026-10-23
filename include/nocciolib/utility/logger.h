#ifndef LOGGER_HEADER
#define LOGGER_HEADER
#include <stdarg.h>

#define l_trace(...) ulogger_log(LOG_TRACE, __VA_ARGS__);
#define l_info(...) ulogger_log(LOG_INFO, __VA_ARGS__);
#define l_error(...) ulogger_log(LOG_ERR, __VA_ARGS__);

typedef enum {
	LOG_TRACE,
	LOG_INFO,
	LOG_WARN,
	LOG_ERR,
	LOG_LEVEL_LEN
} LOG_LEVEL;

#ifdef __EMSCRIPTEN
#define NO_ANSI
#endif

void ulogger_log(LOG_LEVEL level, const char *fmt, ...);

#endif