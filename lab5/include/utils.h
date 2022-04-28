#ifndef UTILS_H
#define UTILS_H

void memcpy(void *dest, const void *src, int n);
int swap_endian_32(int n);
unsigned int swap_endian_32u(unsigned int n);
long long swap_endian_64(long long n);
void delay(unsigned long long time);

#endif
