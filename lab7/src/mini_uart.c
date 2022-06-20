#include "aux.h"
#include "gpio.h"
#include "mini_uart.h"
#include "string.h"
#include "queue.h"
#include "shared_variables.h"
#include "irq.h"

void init_uart() {
    *AUX_ENABLES |= 1;   // Enable mini UART
    *AUX_MU_CNTL = 0;    // Disable TX, RX during configuration
    *AUX_MU_IER = 0;     // Disable interrupt
    *AUX_MU_LCR = 3;     // Set the data size to 8 bit
    *AUX_MU_MCR = 0;     // Don't need auto flow control
    *AUX_MU_BAUD = 270;  // Set baud rate to 115200
    *AUX_MU_IIR = 0xc6;     // No FIFO
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
    

	*AUX_MU_CNTL = 3;
	while((*AUX_MU_LSR&0x01))
		*AUX_MU_IO; //clean rx data

	enable_uart_interrupt();
	enable_interrupt();	
	trigger_rx_interrupt();
	trigger_tx_interrupt();
}

// check ref 15
char uart_read() {
	/*trigger_rx_interrupt();
    while (queue_empty(&read_buf)) asm volatile("nop");
    char c = queue_pop(&read_buf);
    return c == '\r' ? '\n' : c;*/
	do {
        asm volatile("nop");
    } while (!(*AUX_MU_LSR & 0x01));
    // Read
    char r = (char)(*AUX_MU_IO);
    // Convert carrige return to newline
    return r == '\r' ? '\n' : r;
}

void uart_write(unsigned int c) {
	do {
        asm volatile("nop");
    } while (!(*AUX_MU_LSR & 0x20));
    // bit 5(idx start from 0) means transmitter empty
    
    // Write
    *AUX_MU_IO = c;
    // proceess special case
    if (c == '\n') {
        do {
            asm volatile("nop");
        } while(!(*AUX_MU_LSR&0x20));
        *AUX_MU_IO = '\r';
    }
}

void uart_Wputs(char *str, int len) {
	while(len--) {
		uart_write(*str++);
	}
	uart_write('\n');
}

void uart_puts(char* str) {
    for (int i = 0; str[i] != '\0'; i ++) {
       	queue_push(&write_buf, str[i]);
    }
	trigger_tx_interrupt();
}

void uart_puts_sync(char *s) {
    while(*s) {
        uart_write(*s++);
    }
}

void uart_printf(char* fmt, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, fmt);

    //extern volatile unsigned char __end; // defined in linker
    //char *s = (char*)&__end;             // put temporary string after code
    char str[1024];
	vsprintf(str, fmt, args);
	char *s = &str[0];
	uart_puts(s);
    /*while (*s) {
        uart_write(*s++);
    }*/
}

void uart_printf_sync(char* fmt, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, fmt);

    //extern volatile unsigned char __end; // defined in linker
    //char *s = (char*)&__end;             // put temporary string after code
    char str[1024];
	vsprintf(str, fmt, args);
	char *s = &str[0];
	uart_puts_sync(s);
    /*while (*s) {
        uart_write(*s++);
    }*/
}

void enable_interrupt(){
    __asm__ __volatile__("msr daifclr, 0xf");
}

void disable_interrupt(){
    __asm__ __volatile__("msr daifset, 0xf");
}

void enable_uart_interrupt() {
    *IRQ_S1 |= (1 << 29);
}

void disable_uart_interrupt() {
    *IRQ_S1 &= ~(1 << 29);
}

void trigger_tx_interrupt() {
    *AUX_MU_IER |= 0x2;
}

void clear_tx_interrupt() {
    *AUX_MU_IER &= ~0x2;
}

void trigger_rx_interrupt() {
    *AUX_MU_IER |= 0x1;
}

void clear_rx_interrupt() {
    *AUX_MU_IER &= ~0x1;
}
