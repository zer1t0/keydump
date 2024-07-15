#include "syscall_z.h"
#include <sys/syscall.h>

long syscall_z(long number, ...);

pid_t getpid_z(void) { return syscall_z(SYS_getpid); }
pid_t gettid_z(void) { return syscall_z(SYS_gettid); }

ssize_t read_z(int fd, void *buf, size_t count) {
    return syscall_z(
        SYS_read,
        (long)fd,
        (long)buf,
        (long)count
        );
}

int open_z(const char *pathname, int flags, mode_t mode) {
    return syscall_z(
        SYS_open,
        (long)pathname,
        (long)flags,
        (long)mode
        );
}

int openat_z(
    int dirfd,
    const char *pathname,
    int flags,
    mode_t mode
    ) {
    return syscall_z(
        SYS_openat,
        (long)dirfd,
        (long)pathname,
        (long)flags,
        (long)mode
   );
}

int close_z(int fd) {
    return syscall_z(SYS_close, (long)fd);
}

int mkdir_z(const char *pathname, mode_t mode) {
    return syscall_z(SYS_mkdir, (long)pathname, (long) mode);
}

off_t lseek_z(int fd, off_t offset, int whence) {
  return syscall_z(SYS_lseek, (long)fd, (long)offset, (long)whence);
}

int fstat_z(int fd, struct stat *statbuf) {
    return syscall_z(SYS_fstat, (long)statbuf);
}

ssize_t write_z(int fd, const void *buf, size_t count) {
    return syscall_z(SYS_write, (long)fd, (long)buf, (long)count);
}

void *mmap_z(void *addr, size_t length, int prot, int flags, int fd,
             off_t offset) {
    return (void *)syscall_z(
        SYS_mmap,
        (long)addr,
        (long)length,
        (long)prot,
        (long)flags,
        (long)fd,
        (long)offset
        );
}

int munmap_z(void* addr, size_t length) {
    return syscall_z(SYS_munmap, (long)addr, (long)length);
}
