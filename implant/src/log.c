#include "libc_d.h"
#include "basic_z.h"
#include "syscall_z.h"
#include "log.h"
#include <stdarg.h>
#include <stdio.h>

void log_write(const char *msg) {
    int fd;

#ifdef LOG_FILE
    fd = open_z(LOG_FILE, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if(fd == -1) {
        goto close;
    }
#else
    fd = 1;
#endif

    write_z(fd, msg, strlen_z(msg));

#ifdef LOG_FILE
close:
    if (fd != -1) {
        close_z(fd);
    }
#endif
    return;
}

void log_printf(const char *format, ...) {
    FILE* fp = NULL;
    va_list args;
    vfprintf_f vfprintf_d;

#ifdef LOG_FILE
    fclose_f fclose_d;
    fopen_f fopen_d = fopen_s();
    if(!fopen_d) {
        goto close;
    }
    fp = fopen_d(LOG_FILE, "a");
#else
    fdopen_f fdopen_d = fdopen_s();
    if (!fdopen_d) {
      goto close;
    }
    fp = fdopen_d(1, "a");
#endif
    if (!fp) {
      goto close;
    }

    vfprintf_d = vfprintf_s();
    if(!vfprintf_d) {
        goto close;
    }

    va_start(args, format);
    vfprintf_d(fp, format, args);
    va_end(args);

  close:

#ifdef LOG_FILE
    if(fp) {
        fclose_d = fclose_s();
        if(fclose_d) {
            fclose_d(fp);
        }
    }
#endif
    return;
}
