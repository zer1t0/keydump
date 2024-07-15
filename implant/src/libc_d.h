#ifndef LIBCD_H
#define LIBCD_H

#define _GNU_SOURCE
#include <dirent.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

void *libc_search_func(char *name);

#define DECLARE_LIBC_FUNC_SEARCHER(func) typedef __typeof__(func) *func##_f;\
    func##_f func##_s();

#ifdef _GNU_SOURCE
DECLARE_LIBC_FUNC_SEARCHER(clone);
#endif

DECLARE_LIBC_FUNC_SEARCHER(calloc);
DECLARE_LIBC_FUNC_SEARCHER(closedir);
DECLARE_LIBC_FUNC_SEARCHER(dirfd);
DECLARE_LIBC_FUNC_SEARCHER(fclose);
DECLARE_LIBC_FUNC_SEARCHER(free);
DECLARE_LIBC_FUNC_SEARCHER(fopen);
DECLARE_LIBC_FUNC_SEARCHER(fdopen);
DECLARE_LIBC_FUNC_SEARCHER(fwrite);
DECLARE_LIBC_FUNC_SEARCHER(getchar);
DECLARE_LIBC_FUNC_SEARCHER(getline);
DECLARE_LIBC_FUNC_SEARCHER(malloc);
DECLARE_LIBC_FUNC_SEARCHER(mprotect);
DECLARE_LIBC_FUNC_SEARCHER(munmap);
DECLARE_LIBC_FUNC_SEARCHER(opendir);
DECLARE_LIBC_FUNC_SEARCHER(printf);
DECLARE_LIBC_FUNC_SEARCHER(sleep);
DECLARE_LIBC_FUNC_SEARCHER(sprintf);
DECLARE_LIBC_FUNC_SEARCHER(sscanf);
DECLARE_LIBC_FUNC_SEARCHER(time);
DECLARE_LIBC_FUNC_SEARCHER(vfprintf);

#endif
