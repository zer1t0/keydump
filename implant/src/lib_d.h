#ifndef LIBD_H
#define LIBD_H
#include <elf.h>
#include <stdlib.h>

typedef struct {
  void *base_address;
  long symbol_c;
  Elf64_Sym *symbol_t;
  char *symbol_str_t;
} libsyms_t;

int lib_search_symbols_tables(
    const char **lib_name_patterns,
    size_t lib_name_patterns_count,
    libsyms_t *lib_symbols
    );

int lib_search_symbol_value(
    libsyms_t *lib_symbols,
    char *symname,
    char symtype,
    Elf64_Word *symvalue
    );

void *lib_search_function(libsyms_t *lib_symbols, char *name);

#endif
