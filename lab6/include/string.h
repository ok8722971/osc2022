#ifndef STRING_H
#define STRING_H

#include "type.h"

int strcmp (char *s1, char *s2);
int strncmp (char *s1, char *s2, int len);
void strcpy(char *s1, char *s2);
int strlen (char *s);
void reverse_str(char *s);
int atoi(char *s);
int itoa(int x, char str[], int d);
int uitoa(unsigned int x, char str[], int d);
void ltoa(long long x, char str[]);
void ftoa(float n, char* res, int afterpoint);
void itohex_str(uint64_t d, int size, char *s);
void uitoxstr(unsigned int x, char str[]);
void ltoxstr(unsigned long long x, char str[]);
void itoxstr(int x, char str[]);
unsigned int vsprintf(char *dst, char *fmt, __builtin_va_list args);

#endif