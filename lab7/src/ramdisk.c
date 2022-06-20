#include "ramdisk.h"
#include "mini_uart.h"
#include "string.h"
#include "dtb.h"

//#define CPIO_ADDRESS 0x20000000
extern unsigned char __user_program_start; //from linker

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

void read_prog(char *address, unsigned long long fs) {
	long long kernel_size = fs;
    unsigned char *new_bl = (unsigned char *)&__user_program_start;
    unsigned char *bl = (unsigned char *)address;
	//uart_printf("start transfer.\n");
	//uart_printf("nb:%x\t b:%x\t ks:%x\n", new_bl, bl, kernel_size);    
	while (kernel_size--) {
        *new_bl++ = *bl++;
    }
	//uart_printf("nb:%x\t b:%x\t ks:%x\n", new_bl, bl, kernel_size);
	//uart_printf("transfer done.\n");
}

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
unsigned long long hex2dec(char* s, int n)
{
    unsigned long long r = 0;
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
        unsigned long long ns = hex2dec(header->namesize, 8);
        unsigned long long fs = hex2dec(header->filesize, 8);

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
        unsigned long long ns = hex2dec(header->namesize, 8);
        unsigned long long fs = hex2dec(header->filesize, 8);

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
	uart_puts(": No such file or directory\n");

}

void cpio_exec(char *progname) {	
	char *address = CPIO_ADDRESS;
    while (!memcmp(address, "070701", 6) && 
		    memcmp(address + sizeof(cpio_t), "TRAILER!!", 9)) {
        cpio_t* header = (cpio_t*)address;
        unsigned long long ns = hex2dec(header->namesize, 8);
        unsigned long long fs = hex2dec(header->filesize, 8);

		if(ns-1 == strlen(progname) && 
		   strncmp(progname, address + sizeof(cpio_t), ns-1)) {
			if ((sizeof(cpio_t) + ns) % 4) {
				address += (((sizeof(cpio_t) + ns) / 4) + 1) * 4;
			} else {
				address += sizeof(cpio_t) + ns;
			}
			//uart_Wputs(address, fs);
			read_prog(address, fs);
			//register char *x8 __asm("x8");
			//x8 = address;
			//uart_printf("%x\n", x8);
			//from_el1_to_el0();
			/*asm volatile(
				"mov x1, 0\n\t"
				// 0b0000000000 enable interrupt and set to el0
				// 0b1111000000 disable interrupt and set to el0
				"msr spsr_el1, x1\n\t"
				"msr elr_el1, x8\n\t"
				"eret"
			);*/
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
	uart_puts("exec: ");
	uart_puts(progname);
	uart_puts(": No such file or directory\n");
}

char* cpio_get_addr(char *progname) {	
	char *address = CPIO_ADDRESS;
    while (!memcmp(address, "070701", 6) && 
		    memcmp(address + sizeof(cpio_t), "TRAILER!!", 9)) {
        cpio_t* header = (cpio_t*)address;
        unsigned long long ns = hex2dec(header->namesize, 8);
        unsigned long long fs = hex2dec(header->filesize, 8);

		if(ns-1 == strlen(progname) && 
		   strncmp(progname, address + sizeof(cpio_t), ns-1)) {
			if ((sizeof(cpio_t) + ns) % 4) {
				address += (((sizeof(cpio_t) + ns) / 4) + 1) * 4;
			} else {
				address += sizeof(cpio_t) + ns;
			}
			return address;
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
	uart_puts("exec: ");
	uart_puts(progname);
	uart_puts(": No such file or directory\n");
	return 0;
}

unsigned long long cpio_get_fsize(char *progname) {	
	char *address = CPIO_ADDRESS;
    while (!memcmp(address, "070701", 6) && 
		    memcmp(address + sizeof(cpio_t), "TRAILER!!", 9)) {
        cpio_t* header = (cpio_t*)address;
        unsigned long long ns = hex2dec(header->namesize, 8);
        unsigned long long fs = hex2dec(header->filesize, 8);

		if(ns-1 == strlen(progname) && 
		   strncmp(progname, address + sizeof(cpio_t), ns-1)) {
			return fs;
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
	uart_puts("exec: ");
	uart_puts(progname);
	uart_puts(": No such file or directory\n");
	return 0;
}
