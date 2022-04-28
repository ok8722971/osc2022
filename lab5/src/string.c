#include "string.h"
#include "type.h"
#include "math.h"
#include "mini_uart.h"

int strlen(char *s) {
	int i = 0;
	while(*(s+i) != '\0'){
		++i;
	}
	return i;
}

void strcpy(char *s1, char *s2) {
	int i;
	for(i = 0; i < strlen(s1); ++i){
		s2[i] = s1[i];
	}
	s2[i] = '\0';
}


int strcmp(char *s1, char *s2) {
	int l1 = strlen(s1);
	int l2 = strlen(s2);
	if(l1 != l2) return 0;
	int i;
	for(i = 0; i < l1; ++i) {
		if(s1[i] != s2[i]) return 0;
	}

	return 1;
}

int strncmp(char *s1, char *s2, int len) {
	int i;
	for(i = 0; i < len; ++i) {
		if(s1[i] != s2[i]) return 0;
	}
	return 1;
}

void reverse_str(char *s) {
	int i;
	char temp;
	for(i = 0; i < strlen(s)/2; ++i) {
		temp = s[strlen(s)-1-i];
		s[strlen(s)-1-i] = s[i];
		s[i] = temp;
	}
}

int atoi(char *s) {
	int n = 0;
	while(*s){
		n *= 10;
		n += *s - '0';
		s++;
	}
	return n;
}

// Converts a given integer x to string str[]. 
// d is the number of digits required in the output. 
// If d is more than the number of digits in x, 
// then 0s are added at the beginning.
int itoa(int x, char str[], int d){
	int i = 0;
	// take care of negtive number
	if(x < 0) {
		x *= -1;
		str[i++] = '-';
	}
    while(x) {
        str[i++] = (x % 10) + '0';
        x = x / 10;
    }
  
    // If number of digits required is more, then
    // add 0s at the beginning
    while(i < d)
        str[i++] = '0';
  
    reverse_str(str);
    str[i] = '\0';
    return i;
}

int uitoa(unsigned int x, char str[], int d){
	int i = 0;

    while(x) {
        str[i++] = (x % 10) + '0';
        x = x / 10;
    }
  
    // If number of digits required is more, then
    // add 0s at the beginning
    while(i < d)
        str[i++] = '0';
  
    reverse_str(str);
    str[i] = '\0';
    return i;
}

void ltoa(long long x, char str[]) {
	if(x == 0) {
		str[0] = '0';
		str[1] = '\0';
		return;
	}

	int i = 0;
	// take care of negtive number
	if(x < 0) {
		x *= -1;
		str[i++] = '-';
	}
    while(x) {
        str[i++] = (x % 10) + '0';
        x = x / 10;
    }
  
    reverse_str(str);
    str[i] = '\0';
    return ;
}

void itoxstr(int x, char str[]) {
	int i = 0;
	while(x) {
		int temp = x % 16;
		if(temp > 9) {
			str[i++] = temp - 10 + 'A';
		}
		else {
			str[i++] = temp + '0';
		}
		x /= 16;
	}
	str[i++] = 'x';
	str[i++] = '0';
	reverse_str(str);
	str[i] = '\0';
	return;
}

void uitoxstr(unsigned int x, char str[]) {
	int i = 0;
	while(x) {
		int temp = x % 16;
		if(temp > 9) {
			str[i++] = temp - 10 + 'A';
		}
		else {
			str[i++] = temp + '0';
		}
		x /= 16;
	}
	str[i++] = 'x';
	str[i++] = '0';
	reverse_str(str);
	str[i] = '\0';
	return;
}

void ltoxstr(unsigned long long x, char str[]) {
	int i = 0;
	while(x) {
		int temp = x % 16;
		if(temp > 9) {
			str[i++] = temp - 10 + 'A';
		}
		else {
			str[i++] = temp + '0';
		}
		x /= 16;
	}
	str[i++] = 'x';
	str[i++] = '0';
	reverse_str(str);
	str[i] = '\0';
	return;
}


// Converts a floating-point/double number to a string.
void ftoa(float n, char* res, int afterpoint){
    // Extract integer part
    int ipart = (int)n;

    // Extract floating part
    float fpart = n - (float)ipart;

    // convert integer part to string
    int i = itoa(ipart, res, 0);

    // check for display option after point
    if(afterpoint != 0) {
        res[i] = '.'; // add dot

        // Get the value of fraction part upto given no.
        // of points after dot. The third parameter
        // is needed to handle cases like 233.007
        fpart = fpart * pow(10, afterpoint);

        itoa((int)fpart, res + i + 1, afterpoint);
    }
}

void itohex_str(uint64_t d, int size, char *s){
    int i = 0;
    unsigned int n;
    int c;

    c = size * 8;
    s[0] = '0';
    s[1] = 'x';

    for(c = c-4, i = 2; c >= 0; c -= 4, i++){
        // get highest tetrad
        n = (d >> c) & 0xF;

        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        if (n > 9 && n < 16)
            n += ('A' - 10);
        else
            n += '0';

        s[i] = n;
    }

    s[i] = '\0';
}

unsigned int vsprintf(char *dst, char *fmt, __builtin_va_list args) {
    char *dst_orig = dst;

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            // escape %
            if (*fmt == '%') {
                goto put;
            }
            // string
            if (*fmt == 's') {
                char *p = __builtin_va_arg(args, char *);
                while (*p) {
                    *dst++ = *p++;
                }
            }
            // int/long number
            if (*fmt == 'd') {
                int arg = __builtin_va_arg(args, int);
                char p[128] = {0};
                itoa(arg, p, 1);
				for(int i = 0; p[i] != '\0'; ++i) {
					*dst++ = p[i];
				}
            }
			// long long number
            if (*fmt == 'D') {
                long long arg = __builtin_va_arg(args, long long);
                char p[128] = {0};
                ltoa(arg, p);
				for(int i = 0; p[i] != '\0'; ++i) {
					*dst++ = p[i];
				}
            }
            // long/int hex
            if (*fmt == 'x') {
                unsigned int arg = __builtin_va_arg(args, unsigned int);
				// 2("0x") + 16(hex=4byte 8*4=32byte) + 1('\0')
                char p[128] = {0};
                uitoxstr(arg, p); 
				for(int i = 0; p[i] != '\0'; ++i) {
					*dst++ = p[i];
				}
            }
			// long long hex
            if (*fmt == 'X') {
                long long arg = __builtin_va_arg(args, long long);
				// 2("0x") + 16(hex=4byte 8*4=32byte) + 1('\0')
                char p[128] = {0};
                ltoxstr(arg, p); 
				for(int i = 0; p[i] != '\0'; ++i) {
					*dst++ = p[i];
				}
            }
            // float
            if (*fmt == 'f') {
                float arg = (float)__builtin_va_arg(args, double);
                char p[128] = {0};
                ftoa(arg, p, 6);
				for(int i = 0; p[i] != '\0'; ++i) {
					*dst++ = p[i];
				}
            }
        }
        else {
        put:
            *dst++ = *fmt;
        }
        fmt++;
    }
    *dst = '\0';

    return dst - dst_orig;  // return written bytes
}
