#ifndef BASICZ_H
#define BASICZ_H

#include <stdio.h>

int memcmp_z(const void *s1, const void *s2, size_t n);

int strcmp_z(const char *s1, const char *s2);

char *strcpy_z(char *dest, const char *src);

size_t strlen_z(const char *s);

char *strstr_z(const char *haystack, const char *needle);

void puts_z(const char* msg);

long lpow(long base, long exp);

char hex_to_number(char hex);

#endif
