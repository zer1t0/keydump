#ifndef LOG_H
#define LOG_H
#include "build.h"

#ifdef ENABLE_LOG

void log_write(const char *msg);
void log_printf(const char *format, ...);

#define LOG_WRITE(m) log_write(m)
#define LOG_PRINTF(...) log_printf(__VA_ARGS__)
#else
#define LOG_WRITE(m)
#define LOG_PRINTF(...)
#endif

#endif
