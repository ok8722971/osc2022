#include "mini_uart.h"
extern unsigned char __boot_loader;

void loadimg() {
	uart_puts("Send image via UART now!\n");
    unsigned char *kernel = (unsigned char *)0x80000;

    // big endian
    int img_size = 0, i;
    for (i = 0; i < 4; i++) {
        img_size <<= 8;
        img_size |= (int)uart_read_raw();
    }

    // big endian
    int img_checksum = 0;
    for (i = 0; i < 4; i++) {
        img_checksum <<= 8;
        img_checksum |= (int)uart_read_raw();
    }

    for (i = 0; i < img_size; i++) {
        char b = uart_read_raw();
        *(kernel + i) = b;
        img_checksum -= (int)b;
    }

    if (img_checksum != 0) {
        uart_puts("Failed!");
    }
    else {
        void (*start_os)(void) = (void *)kernel;
        start_os();
    }
}

int main() {
	uart_init();
	uart_puts("hi\n");
	loadimg();
	return 0;
}

