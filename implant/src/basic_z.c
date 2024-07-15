#include "basic_z.h"
#include "syscall_z.h"
#include <unistd.h>

int memcmp_z(const void *s1, const void *s2, size_t n) {
    size_t i;
    int diff = 0;
    for(i = 0; i < n; i++) {
        diff = ((char*)s1)[i] - ((char*)s2)[i];
        if(diff) {
            break;
        }
    }
    return diff;
}

size_t strlen_z(const char *s) {
    size_t i;
    for(i = 0; s[i]; i++);
    return i;
}

int strcmp_z(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return (int)(*s1 - *s2);
}

char* strcpy_z(char* dest, const char *src) {
    char* dest_ptr = dest;
    for(;*src; src++, dest_ptr++) {
        *dest_ptr = *src;
    }
    *dest_ptr = 0;
    return dest;
}

char* strstr_z(const char *haystack, const char *needle) {
    size_t needle_len = strlen_z(needle);
    size_t haystack_len = strlen_z(haystack);
    for(;*haystack && (haystack_len >= needle_len); haystack++, haystack_len--){
        if(memcmp_z(haystack, needle, needle_len) == 0) {
            return (char*)haystack;
        }
    }

    return NULL;
}

void puts_z(const char* msg) {
    write_z(STDOUT_FILENO,  msg, strlen_z(msg));
}

// https://stackoverflow.com/questions/101439/the-most-efficient-way-to-implement-an-integer-based-power-function-powint-int#answer-101613
long lpow(long base, long exp) {
  long result = 1;
  for (;;) {
    if (exp & 1)
      result *= base;
    exp >>= 1;
    if (!exp)
      break;
    base *= base;
  }

  return result;
}

char hex_to_number(char hex) {
  if (hex >= 0x30 && hex <= 0x39) {
    return hex - 0x30;
  }
  if (hex >= 0x61) {
    hex -= 0x20;
  }
  return hex - 0x37;
}
