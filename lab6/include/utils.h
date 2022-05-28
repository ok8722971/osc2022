#ifndef UTILS_H
#define UTILS_H

int ceil(float num);
void memzero(void *start, unsigned long long n);
void memcpy(void *dest, const void *src, unsigned long long n);
int swap_endian_32(int n);
unsigned int swap_endian_32u(unsigned int n);
long long swap_endian_64(long long n);
void delay(unsigned long long time);

#endif
