#include "dtb.h"
#include "mini_uart.h"
#include "utils.h"
#include "string.h"

#define FDT_BEGIN_NODE	0x1		/* Start node: full name */
#define FDT_END_NODE	0x2		/* End node */
#define FDT_PROP	    0x3		/* Property: name off, size, content */
#define FDT_NOP		    0x4		/* nop */
#define FDT_END		    0x9

typedef struct {
	unsigned int magic;
	unsigned int totalsize;
	unsigned int off_dt_struct;
	unsigned int off_dt_strings;
	unsigned int off_mem_rsvmap;
	unsigned int version;
	unsigned int last_comp_version;
	unsigned int boot_cpuid_phys;
	unsigned int size_dt_strings;
	unsigned int size_dt_struct;
} fdt_header;

typedef struct {
	unsigned int len;
	unsigned int nameoff;
} fdt_prop;

register long long *X11 asm("x11");

char *dtb_base_address;
char *dtb_struct_address;
char *dtb_string_address;
unsigned int dtb_struct_size;
unsigned int dtb_string_size;

void init_dtb() {
	dtb_base_address = X11;
	fdt_header *header = (fdt_header*) dtb_base_address;
	dtb_struct_address = dtb_base_address + 
						(int)swap_endian_32u(header->off_dt_struct);
	dtb_string_address = dtb_base_address +
						(int)swap_endian_32u(header->off_dt_strings);
	
	dtb_struct_size = swap_endian_32u(header->size_dt_struct);
	dtb_string_size = swap_endian_32u(header->size_dt_strings);

	return ;
}


char* dtb_get_initrd_address(){
	long long res = 0;
	char *struct_address = dtb_struct_address;
	unsigned int size = dtb_struct_size;
	unsigned int tag;

	tag = *((unsigned int*)struct_address);
	tag = swap_endian_32u(tag);
	
	while(tag != FDT_END) {

		//uart_print_uint(tag);
		//uart_puts("\n");	
		
    	/*int temp = 1500000;
    	while(temp--) {
        	asm volatile("nop");
    	}*/

		struct_address += 4;
			
		if(tag == FDT_BEGIN_NODE) {
			while(*struct_address != '\0') struct_address++;
			struct_address++;
			while((long long)struct_address % 4 != 0) struct_address++;
		}
		else if(tag == FDT_END_NODE) {}
		else if(tag == FDT_PROP) {
			fdt_prop *p = (fdt_prop*)struct_address;
			unsigned int len = swap_endian_32u(p->len);
			unsigned int nameoff = swap_endian_32u(p->nameoff);
			
			char *string_address = dtb_string_address + nameoff;
			char str[128] = {0};
			int i = 0;
			while(*string_address != '\0') {
				str[i++] = *string_address++;
			}
			str[i] = '\0';
			//uart_puts(str);
			//uart_write('\n');
			if(strcmp(str, "linux,initrd-start")){
				struct_address += sizeof(fdt_prop);
				int i;
				for(i = 0; i < len; ++i) {
					res <<= 8;
					res |= *struct_address++;
				}
				// uart_print_long(res);
				// uart_puts("\n");
				return res;
			}

			struct_address += sizeof(fdt_prop) + len;
			while((long long)struct_address % 4 != 0) struct_address++;
		}
		else if(tag == FDT_NOP) {}
		else {
			uart_print_long(struct_address-4);
			uart_puts("\nsomething wrong here\n");
			return 0;
		}
		//uart_print_long(struct_address);
		//uart_write('\n');
		tag = *((unsigned int*)struct_address);
		tag = swap_endian_32u(tag);
	}

	return 0;
}
