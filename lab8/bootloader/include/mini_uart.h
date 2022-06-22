#ifndef MINI_UART
#define MINI_UART

void uart_init();
void uart_write(unsigned int c);
char uart_read_raw();
char uart_read();
void uart_puts(char* str);

#endif
