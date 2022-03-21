#include "string.h"
#include "type.h"
#include "math.h"

int strlen(char *s) {
	int i = 0;
	while(*(s+i) != '\0'){
		++i;
	}
	return i;
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

void reverse_str(char *s) {
	int i;
	char temp;
	for(i = 0; i < strlen(s)/2; ++i) {
		temp = s[strlen(s)-1-i];
		s[strlen(s)-1-i] = s[i];
		s[i] = temp;
	}
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
