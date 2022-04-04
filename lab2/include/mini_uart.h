#ifndef MINI_UART
#define MINI_UART

void init_uart();
void uart_write(unsigned int c);
char uart_read();
void uart_puts(char *str);
void uart_Wputs(char *str, int len);
void uart_print_int(int i);
void uart_print_ix(int i);
void uart_print_uint(unsigned int i);
void uart_print_uix(unsigned int i);
void uart_print_long(long long l);
void uart_writeb(unsigned int c);

#endif
