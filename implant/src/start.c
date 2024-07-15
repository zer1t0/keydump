#include "start.h"
#include "linker.h"
#include "libc_d.h"
#include "log.h"
#include "syscall_z.h"

int main();

long __data_offset;
void *get_data_start_addr() { return &__data_offset; }

struct unmap_shc_args_t {
  void *munmap;
  void *map_addr;
  unsigned long map_size;
  void *sleep;
};

int unmap_shc(struct unmap_shc_args_t *args);

STORE_IN_DATA struct unmap_shc_args_t unmap_shc_args;

#define STACK_SIZE 4096

int delete_shc(void* start_addr, unsigned long size) {
    clone_f clone_d = clone_s();
    munmap_f munmap_d = munmap_s();
    sleep_f sleep_d = sleep_s();
    calloc_f calloc_d = calloc_s();
    void* stack_bottom = NULL;
    void* stack_top = NULL;
    int result = -1;

    if(!(clone_d && munmap_d && calloc_d && sleep_d)) {
        goto close;
    }

    stack_bottom = calloc_d(1, STACK_SIZE);
    if(!stack_bottom) {
        goto close;
    }

    stack_top = stack_bottom + STACK_SIZE;

    unmap_shc_args.munmap = (void*) munmap_d;
    unmap_shc_args.map_addr = start_addr;
    unmap_shc_args.map_size = size;
    unmap_shc_args.sleep = (void*) sleep_d;

    result = clone_d((int (*)(void *))unmap_shc, stack_top,
                                CLONE_VM | CLONE_THREAD | CLONE_SIGHAND,
                                &unmap_shc_args, NULL, NULL, NULL);

close:
    return result;
}

int premain() {
    LOG_WRITE("--- Start shellcode premain ---\n");
    int err = -1;
    void *start_addr = get_start_addr();
    void *end_addr = get_end_addr();
    void *data_addr = get_data_start_addr();
    size_t data_size = end_addr - data_addr;
    size_t code_size = data_addr - start_addr;
    mprotect_f mprotect_d = mprotect_s();
    /* pid_t pid = getpid_z(); */
    /* LOG_PRINTF("PID: %d\n", pid); */
    LOG_PRINTF("Start addr: %p\n", start_addr);
    LOG_PRINTF("Data addr: %p\n", data_addr);
    LOG_PRINTF("End addr: %p\n", end_addr);
    LOG_PRINTF("Data size: 0x%lx\n", data_size);

    if(!mprotect_d) {
        goto close;
    }
    mprotect_d(start_addr, code_size, PROT_READ | PROT_EXEC);
    mprotect_d(data_addr, data_size, PROT_READ | PROT_WRITE);

    err = main();

    delete_shc(start_addr, end_addr - start_addr);

close:
    LOG_WRITE("--- End shellcode premain ---\n");
    return err;
}
