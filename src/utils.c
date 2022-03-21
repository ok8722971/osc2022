#include "utils.h"



int swap_endian_32(int n) {
	int b0, b1, b2, b3;
	int res;

	b0 = (n & 0x000000ff) << 24;
	b1 = (n & 0x0000ff00) << 8;
	b2 = (n & 0x00ff0000) >> 8;
	b3 = (n & 0xff000000) >> 24;
	
	res = b0 | b1 | b2 | b3;
	
	return res;
}

unsigned int swap_endian_32u(unsigned int n) {
	unsigned int b0, b1, b2, b3;
	unsigned int res;

	b0 = (n & 0x000000ff) << 24u;
	b1 = (n & 0x0000ff00) << 8u;
	b2 = (n & 0x00ff0000) >> 8u;
	b3 = (n & 0xff000000) >> 24u;
	
	res = b0 | b1 | b2 | b3;
	
	return res;
}

long long swap_endian_64(long long n) {
	long long b0, b1, b2, b3, b4, b5, b6, b7;
	long long res;
	
	b0 = (n & 0x00000000000000ff) << 56u;	
	b1 = (n & 0x000000000000ff00) << 40u;
	b2 = (n & 0x0000000000ff0000) << 24u;
	b3 = (n & 0x00000000ff000000) << 8u;
	b4 = (n & 0x000000ff00000000) >> 8u;
	b5 = (n & 0x0000ff0000000000) >> 24U;
	b6 = (n & 0x00ff000000000000) >> 40u;
	b7 = (n & 0xff00000000000000) >> 56u;

	res = b0 | b1 | b2 | b3 | b4 | b5 | b6 | b7;
	return res;
}
