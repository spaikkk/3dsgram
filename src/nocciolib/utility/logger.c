#include "utility/logger.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

char *type[LOG_LEVEL_LEN] = {"DEBUG", "INFO", "WARN", "ERR"};

char *colors[LOG_LEVEL_LEN] = {"\x1b[0m", "\x1b[32m", "\x1b[1;33m", "\x1b[31m"};

time_t current_time;
struct tm *m_time;

void log_get_time() {
  time(&current_time);
  m_time = localtime(&current_time);
}

void ulogger_log(LOG_LEVEL level, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  log_get_time();
#ifndef NO_ANSI
  printf("%s", colors[level]);
#endif

  printf("[%d:%d:%d][%s] ", m_time->tm_hour, m_time->tm_min, m_time->tm_sec,
         type[level]);
  vfprintf(stdout, fmt, args);
  printf("\n%s", colors[LOG_TRACE]);
  va_end(args);
}