#ifndef MINI_UART
#define MINI_UART

void init_uart();
void uart_write(unsigned int c);
char uart_read();
void uart_puts(char *str);
void uart_puts_sync(char *s);
void uart_Wputs(char *str, int len);
void uart_printf(char *fmt, ...);
void uart_printf_sync(char *fmt, ...);
void enable_interrupt();
void disable_interrupt();
void enable_uart_interrupt();
void disable_uart_interrupt();
void trigger_tx_interrupt();
void clear_tx_interrupt();
void trigger_rx_interrupt();
void clear_rx_interrupt();

#endif
