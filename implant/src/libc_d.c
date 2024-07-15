#define _GNU_SOURCE
#include "lib_d.h"
#include "linker.h"
#include "libc_d.h"
#include "log.h"

STORE_IN_DATA libsyms_t libc_symbols = {0};

const char libc_pattern1[] = "/libc-";
const char libc_pattern2[] = "/libc.";

static int init_glibc_symbols_tables() {
  const char *lib_patterns[] = {libc_pattern1, libc_pattern2};
  size_t lib_patterns_count = 2;
  LOG_WRITE("Init glibc symbol tables\n");

  return lib_search_symbols_tables(lib_patterns, lib_patterns_count, &libc_symbols);
}

void *libc_search_func(char *name) {
    LOG_WRITE("Libc searching function:");
    LOG_WRITE(name);
    LOG_WRITE("\n");
    if (!libc_symbols.base_address) {
        if (init_glibc_symbols_tables()) {
            return NULL;
        }
    }

    return lib_search_function(&libc_symbols, name);
}

#define IMPLEMENT_LIBC_FUNC_SEARCHER(func)                              \
    STORE_IN_DATA func##_f func##_v = NULL;                             \
    func##_f func##_s() {                                               \
        if (!func##_v) {                                                \
            func##_v = libc_search_func(#func);                         \
        }                                                               \
        return func##_v;                                                \
    }

#ifdef _GNU_SOURCE
IMPLEMENT_LIBC_FUNC_SEARCHER(clone);
#endif

IMPLEMENT_LIBC_FUNC_SEARCHER(calloc);
IMPLEMENT_LIBC_FUNC_SEARCHER(closedir);
IMPLEMENT_LIBC_FUNC_SEARCHER(dirfd);
IMPLEMENT_LIBC_FUNC_SEARCHER(fclose);
IMPLEMENT_LIBC_FUNC_SEARCHER(free);
IMPLEMENT_LIBC_FUNC_SEARCHER(fopen);
IMPLEMENT_LIBC_FUNC_SEARCHER(fdopen);
IMPLEMENT_LIBC_FUNC_SEARCHER(fwrite);
IMPLEMENT_LIBC_FUNC_SEARCHER(vfprintf);
IMPLEMENT_LIBC_FUNC_SEARCHER(getchar);
IMPLEMENT_LIBC_FUNC_SEARCHER(getline);
IMPLEMENT_LIBC_FUNC_SEARCHER(malloc);
IMPLEMENT_LIBC_FUNC_SEARCHER(mprotect);
IMPLEMENT_LIBC_FUNC_SEARCHER(munmap);
IMPLEMENT_LIBC_FUNC_SEARCHER(opendir);
IMPLEMENT_LIBC_FUNC_SEARCHER(printf);
IMPLEMENT_LIBC_FUNC_SEARCHER(sleep);
IMPLEMENT_LIBC_FUNC_SEARCHER(sprintf);
IMPLEMENT_LIBC_FUNC_SEARCHER(sscanf);
IMPLEMENT_LIBC_FUNC_SEARCHER(time);
