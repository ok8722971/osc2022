#include "ramdisk.h"
#include "mini_uart.h"
#include "string.h"
#include "dtb.h"

//#define CPIO_ADDRESS 0x20000000

/* cpio hpodc format */
typedef struct {
	char magic[6];      /* Magic header '070707'. */
	char ino[8];        /* "i-node" number. */
	char mode[8];       /* Permisions. */
	char uid[8];        /* User ID. */
	char gid[8];        /* Group ID. */
	char nlink[8];      /* Number of hard links. */
	char mtime[8];      /* Modification time. */
	char filesize[8];   /* File size. */
	char devmajor[8];   /* device number. */
	char devminor[8];   /* device number. */
	char rdevmajor[8];  /* device major/minor number. */
	char rdevminor[8];  /* device major/minor number. */
	char namesize[8];   /* Length of filename in bytes. */
	char check[8];
} cpio_t;

int memcmp(void* s1, void* s2, int n)
{
    unsigned char *a = s1, *b = s2;
    while (n--) {
        if (*a != *b) {
            return *a - *b;
        }
        a++;
        b++;
    }
    return 0;
}

char *CPIO_ADDRESS;

void init_cpio() {
	CPIO_ADDRESS = dtb_get_initrd_address(); 
}

/**
 * Helper function to convert ASCII octal number into binary
 * s string
 * n number of digits
 */
int hex2dec(char* s, int n)
{
    int r = 0;
    while (n--) {
        r *= 16;
        int dif = *s++ - '0';
        if (dif < 10) {
            r += dif;
        } else {
            r += dif - 7;
        }
    }
    return r;
}

void cpio_ls() {
	char *address = CPIO_ADDRESS;
    while (!memcmp(address, "070701", 6) && memcmp(address + sizeof(cpio_t), "TRAILER!!", 9)) {
        cpio_t* header = (cpio_t*)address;
        int ns = hex2dec(header->namesize, 8);
        int fs = hex2dec(header->filesize, 8);

		uart_Wputs(address + sizeof(cpio_t), ns);

        if ((sizeof(cpio_t) + ns) % 4) {
            address += (((sizeof(cpio_t) + ns) / 4) + 1) * 4;
        } else {
            address += sizeof(cpio_t) + ns;
        }

        if ((fs) % 4) {
            address += ((fs / 4) + 1) * 4;
        } else {
            address += fs;
        }
	}
}

void cpio_cat(char *filename) {

	char *address = CPIO_ADDRESS;
    while (!memcmp(address, "070701", 6) && 
		    memcmp(address + sizeof(cpio_t), "TRAILER!!", 9)) {
        cpio_t* header = (cpio_t*)address;
        int ns = hex2dec(header->namesize, 8);
        int fs = hex2dec(header->filesize, 8);

		if(ns-1 == strlen(filename) && 
		   strncmp(filename, address + sizeof(cpio_t), ns-1)) {
			if ((sizeof(cpio_t) + ns) % 4) {
				address += (((sizeof(cpio_t) + ns) / 4) + 1) * 4;
			} else {
				address += sizeof(cpio_t) + ns;
			}
			uart_Wputs(address, fs);
			//uart_write('\n');
			return ;
		}

        if ((sizeof(cpio_t) + ns) % 4) {
            address += (((sizeof(cpio_t) + ns) / 4) + 1) * 4;
        } else {
            address += sizeof(cpio_t) + ns;
        }

        if ((fs) % 4) {
            address += ((fs / 4) + 1) * 4;
        } else {
            address += fs;
        }
	}
	uart_puts("cat: ");
	uart_puts(filename);
	uart_puts(": No suck file or directory\n");

}
