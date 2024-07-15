#ifndef SYSCALLZ_H
#define SYSCALLZ_H

#include <fcntl.h>
#include <stddef.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

long syscall_z(long number, ...);

pid_t getpid_z(void);
pid_t gettid_z(void);

ssize_t read_z(int fd, void *buf, size_t count);

int open_z(const char *pathname, int flags, mode_t mode);

int openat_z(int dirfd, const char *pathname, int flags, mode_t mode);

int close_z(int fd);

int mkdir_z(const char *pathname, mode_t mode);

off_t lseek_z(int fd, off_t offset, int whence);


int fstat_z(int fd, struct stat *statbuf);

ssize_t write_z(int fd, const void *buf, size_t count);

void *mmap_z(void *addr, size_t length, int prot, int flags, int fd,
             off_t offset);
int munmap_z(void *addr, size_t length);

#endif
