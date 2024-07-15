#include "lib_d.h"
#include "basic_z.h"
#include "log.h"
#include "print.h"
#include "syscall_z.h"
#include <elf.h>
#include <fcntl.h>

static int line_contains_lib(
    const char *line,
    const char **lib_name_patterns,
    size_t lib_name_patterns_count
) {
    size_t i;
    for(i = 0; i < lib_name_patterns_count; i++) {
        if(strstr_z(line, lib_name_patterns[i])) {
            return 1;
        }
    }
    return 0;
}

static char* line_extract_filepath(const char* line) {
    return strstr_z(line, "/");
}

static long line_extract_base_address(const char* line) {
    long base_addr = 0;
    int i;
    char *end_base_addr = strstr_z(line, "-");
    if(!end_base_addr) {
        return 0;
    }
    end_base_addr--;
    for(i = 0; end_base_addr >= line; end_base_addr--, i++) {
        base_addr += hex_to_number(*end_base_addr) * lpow(16,i);
    }

    return base_addr;
}


// It is not possible to map /proc/self/maps to memory with mmap
// so we will read it.
static int search_lib_address_and_path(
    const char** lib_name_patterns,
    size_t lib_name_patterns_count,
    char* lib_path,
    void** base_address
) {
    int fd = 0;
    // this buffer size should be enough to contain any line in /proc/self/maps
    // since maximum path size is 4096
    char buff[5120];
    ssize_t read_count;
    int ok = -1;

    fd = open_z("/proc/self/maps", O_RDONLY, 0);
    if(fd < 0) {
        PRINTF("Error opening proc files: %d\n", fd);
        goto close;
    }

    while(1) {
        // LESSON: It can read less data than the maximum
        read_count = read_z(fd, buff, sizeof(buff) - 1);
        if(read_count < 0){
            PRINTF("Error reading maps: %ld\n", read_count);
            goto close;
        }
        if(read_count == 0) {
            goto close;
        }
        buff[read_count] = 0;
        LOG_WRITE("Reading maps line\n");
        // PRINTF("Read(%ld): %s\n\n", read_count, buff);

        ssize_t i;
        ssize_t startline_i = 0;
        for (i = 0; i < read_count; i++) {
            if (buff[i] != '\n') {
                continue;
            }

            buff[i] = 0;
            // PRINTF("i: %ld\n", i);
            PRINTF("%s\n", buff + startline_i);
            LOG_WRITE(buff + startline_i);
            LOG_WRITE("\n");
            if (line_contains_lib(
                    buff + startline_i,
                    lib_name_patterns,
                    lib_name_patterns_count
               )) {
              strcpy_z(lib_path, line_extract_filepath(buff + startline_i));
              *base_address =
                  (void *)line_extract_base_address(buff + startline_i);
              goto close_ok;
            } else {
              startline_i = i + 1;
            }
        }

        if(lseek_z(fd, startline_i - i, SEEK_CUR) < 0){
          LOG_WRITE("Error in lseek\n");
          PRINTF("Error in lseek\n");
          goto close;
        }
    }

close_ok:
    ok = 0;
 close:
    if(fd >= 0) {
        close_z(fd);
    }

    return ok;
}

const char dynsym[] = ".dynsym";
const char dynstr[] = ".dynstr";

static int search_lib_symbols_tables(
    const char* lib_path,
    long* symbolt_vaddr,
    long* symbolt_count,
    long* symbolstrt_vaddr
) {
    int fd = 0;
    off_t filesize = 0;
    Elf64_Ehdr* elf_h;
    Elf64_Shdr* section_h;
    Elf64_Shdr* string_table_section_h;
    Elf64_Shdr *symbol_table_section_h;
    char *string_table;
    char* section_name;
    void* base_address = NULL;
    void* end_address;
    int i;
    size_t dynsym_size;
    size_t dynstr_size;
    long symbol_string_table_vaddr;
    int ok = -1;

    fd = open_z(lib_path, O_RDONLY, 0);
    if(fd < 0) {
        goto close;
    }

    filesize = lseek_z(fd, 0 , SEEK_END);
    if(filesize < 0) {
        PRINTF("Error seeking: %ld\n", filesize);
        goto close;
    }

    base_address = mmap_z(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
    if(base_address < 0) {
        PRINTF("Error mapping file: %lx\n", (long)base_address);
        goto close;
    }
    end_address = base_address + filesize;

    elf_h = (Elf64_Ehdr*)base_address;
    if(((void*)elf_h + sizeof(*elf_h)) > end_address) {
        goto close;
    }

    if(elf_h->e_ident[EI_CLASS] != ELFCLASS64) {
        goto close;
    }

    if (elf_h->e_shstrndx == SHN_UNDEF) {
      goto close;
    }

    section_h = base_address + elf_h->e_shoff;
    if((void*)(section_h + elf_h->e_shnum) > end_address) {
        goto close;
    }

    string_table_section_h = section_h + elf_h->e_shstrndx;
    if(((void*)string_table_section_h + sizeof(Elf64_Shdr)) > end_address) {
        goto close;
    }
    string_table = base_address + string_table_section_h->sh_offset;

    dynsym_size = strlen_z(dynsym) + 1;
    dynstr_size = strlen_z(dynstr) + 1;
    symbol_table_section_h = NULL;
    symbol_string_table_vaddr = 0;
    for(i = 0; i < elf_h->e_shnum; i++) {
        section_name = string_table + section_h[i].sh_name;
        if ((void*)(section_name + dynsym_size) > end_address) {
            continue;
        }
        if (memcmp_z(section_name, dynsym, dynstr_size) == 0) {
            symbol_table_section_h = section_h + i;
            continue;
        }
        if((void*)(section_name + dynstr_size) > end_address) {
            continue;
        }
        if(memcmp_z(section_name, dynstr, dynstr_size) == 0) {
          symbol_string_table_vaddr = section_h[i].sh_addr;
          continue;
        }
    }

    if(!symbol_string_table_vaddr || !symbol_table_section_h) {
        goto close;
    }
    *symbolstrt_vaddr = symbol_string_table_vaddr;
    *symbolt_count = symbol_table_section_h->sh_size / symbol_table_section_h->sh_entsize;
    *symbolt_vaddr = symbol_table_section_h->sh_addr;

    ok = 0;
close:
    if (base_address >= 0) {
      munmap_z(base_address, filesize);
    }
    if(fd >= 0) {
        close_z(fd);
    }

    return ok;
}

int lib_search_symbols_tables(
    const char **lib_name_patterns,
    size_t lib_name_patterns_count,
    libsyms_t* lib_symbols
) {
    // max path is 4096
    char lib_path[4104];
    void *base_address;
    long symbolt_vaddr;
    long symbolt_count;
    long symbolstrt_vaddr;

    if (search_lib_address_and_path(
            lib_name_patterns,
            lib_name_patterns_count,
            lib_path,
            &base_address
            )) {
        PRINTF("Unable to find library path\n");
        LOG_WRITE("Unable to find library path\n");
        return -1;
    }
    LOG_WRITE("Found lib path: ");
    LOG_WRITE(lib_path);
    LOG_WRITE("\n");

    PRINTF("Library path: %s\n", lib_path);
    PRINTF("Base address: %p\n", base_address);

    if(search_lib_symbols_tables(
           lib_path,
           &symbolt_vaddr,
           &symbolt_count,
           &symbolstrt_vaddr
           )) {
        PRINTF("Error searching symbol table\n");
        LOG_WRITE("Error searching symbol table");
        return -1;
    }
    LOG_WRITE("Found symbols table\n");

    PRINTF("symbol table vaddr: %ld\n", symbolt_vaddr);
    PRINTF("symbol count: %ld\n", symbolt_count);
    PRINTF("symbol string table vaddr: %ld\n", symbolstrt_vaddr);
    lib_symbols->base_address = base_address;
    lib_symbols->symbol_str_t = base_address + symbolstrt_vaddr;
    lib_symbols->symbol_t = base_address + symbolt_vaddr;
    lib_symbols->symbol_c = symbolt_count;

    return 0;
}
int lib_search_symbol_value(
    libsyms_t *lib_symbols,
    char *symname,
    char symtype,
    Elf64_Word *symvalue
) {
    long i;
    char *symbol_name;

    for (i = 0; i < lib_symbols->symbol_c; i++) {
        if (ELF64_ST_TYPE(lib_symbols->symbol_t[i].st_info) != symtype) {
            continue;
        }
        if (lib_symbols->symbol_t[i].st_name == 0) {
            continue;
        }

          symbol_name =
              lib_symbols->symbol_str_t + lib_symbols->symbol_t[i].st_name;
        if (strcmp_z(symbol_name, symname) == 0) {
            *symvalue = lib_symbols->symbol_t[i].st_value;
            return 0;
        }
    }
    return -1;
}

void* lib_search_function(libsyms_t *lib_symbols, char* name) {
    Elf64_Word func_vaddr;
    LOG_WRITE("Lib search function init\n");
    if(lib_search_symbol_value(lib_symbols, name, STT_FUNC, &func_vaddr)) {
      if (lib_search_symbol_value(lib_symbols, name, STT_GNU_IFUNC,
                                  &func_vaddr)) {
        return NULL;
      }
    }

    return lib_symbols->base_address + func_vaddr;
}
