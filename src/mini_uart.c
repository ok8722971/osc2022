#include "aux.h"
#include "gpio.h"
#include "mini_uart.h"
#include "string.h"

void init_uart() {
    // GPIO alternative function selection
    // gpio14/15's alt5 is TXD1/RXD1
    // ref page 92 && 102
    unsigned int temp = *GPFSEL1;
    temp &= ~(7<<12); // clean gpio14
    temp |= 2<<12;    // set alt5 for gpio14
    temp &= ~(7<<15); // clean gpio15
    temp |= 2<<15;    // set alt5 for gpio15
    *GPFSEL1 = temp;

    // ref page 101, disable GPIO pull-up/down
    
    // 1. Write to GPPUD to set the required control signal (i.e. Pull-up or             Pull-Down or neither to remove the current Pull-up/down)
    *GPPUD = 0;
    
    // 2. Wait 150 cycles – this provides the required set-up time for the               control signal
    temp = 150;
    while(temp--) {
        asm volatile("nop");
    }
    
    // 3. Write to GPPUDCLK0/1 to clock the control signal into the GPIO pads            you wish to modify – NOTE only the pads which receive a clock will be          modified, all others will retain their previous state.
    *GPPUDCLK0 = (1 << 14) | (1 << 15);
    
    // 4. Wait 150 cycles – this provides the required hold time for the                 control signal
    temp = 150;
    while(temp--) {
        asm volatile("nop");
    }
    
    // 5. Write to GPPUDCLK0/1 to remove the clock
    *GPPUDCLK0 = 0;
    
    *AUX_ENABLES |= 1;   // Enable mini UART
    *AUX_MU_CNTL = 0;    // Disable TX, RX during configuration
    *AUX_MU_IER = 0;     // Disable interrupt
    *AUX_MU_LCR = 3;     // Set the data size to 8 bit
    *AUX_MU_MCR = 0;     // Don't need auto flow control
    *AUX_MU_BAUD = 270;  // Set baud rate to 115200
    *AUX_MU_IIR = 6;     // No FIFO
    *AUX_MU_CNTL = 3;    // Enable the transmitter and receiver.
}

// check ref 15
char uart_read() {
    // Check data ready field
    do {
        asm volatile("nop");
    } while (!(*AUX_MU_LSR & 0x01));
    // bit 0 (idx start from 0) means data ready
    // Read
    char r = (char)(*AUX_MU_IO);
    // Convert carrige return to newline
    return r == '\r' ? '\n' : r;
}

void uart_write(unsigned int c) {
    // Check transmitter idle field
    do {
        asm volatile("nop");
    } while (!(*AUX_MU_LSR & 0x20));
    // bit 5(idx start from 0) means transmitter empty
    
    // Write
    *AUX_MU_IO = c;
    // proceess special case
    if ( c == '\n' ) 
    {
        do {
            asm volatile("nop");
        } while( ! ( *AUX_MU_LSR&0x20 ));
        *AUX_MU_IO = '\r';
    }
}

void uart_Wputs(char *str, int len) {
	while(len--) {
		uart_write(*str++);
	}
	uart_write('\n');
}

void uart_puts(char* str)
{
    for (int i = 0; str[i] != '\0'; i ++) {
        uart_write((char)str[i]);
    }
}

void uart_print_int(int i) {
	char str[128] = {0};
	itoa(i, str, 0);
	uart_puts(str);
}

void uart_print_uint(unsigned int i) {
	char str[128] = {0};
	uitoa(i, str, 0);
	uart_puts(str);
}

void uart_print_ix(int i) {
	char str[128] = {0};
	itoxstr(i, str);
	uart_puts(str);
}

void uart_print_uix(unsigned int i) {
	char s[128] = {0};
	uitoxstr(i, s);
	uart_puts(s);
}

void uart_print_long(long long l) {
	char str[128] = {0};
	ltoxstr(l, str);
	uart_puts(str);
}
