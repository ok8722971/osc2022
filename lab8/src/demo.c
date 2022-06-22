#include "mmio.h"
#include "schedule.h"
#include "mini_uart.h"
#include "sys.h"
#include "mm.h"
#include "vfs.h"


void demo_lab7_req1() {
    struct file* a = vfs_open("hello", 0);
    uart_printf("addr: %X\n", a);
    a = vfs_open("hello", O_CREAT);
    uart_printf("addr: %X\n", a);
    vfs_close(a);
    struct file* b = vfs_open("hello", 0);
    uart_printf("addr: %X\n", b);
    vfs_close(b);
    while (1);
}

void demo_lab7_req2() {
    struct file* a = vfs_open("hello", O_CREAT);
    struct file* b = vfs_open("world", O_CREAT);
    vfs_write(a, "Hello ", 6);
    vfs_write(b, "World!", 6);
    vfs_close(a);
    vfs_close(b);

    char buf[512];
    b = vfs_open("hello", 0);
    a = vfs_open("world", 0);
    int sz;
    sz = vfs_read(b, buf, 100);
    sz += vfs_read(a, buf + sz, 100);
    buf[sz] = '\0';
    uart_printf("%s\n", buf); // should be Hello World!
    while (1);
}

void demo_lab7_req3() {
	struct file* a = vfs_open("hello", O_CREAT);
	vfs_write(a, "hello ", 6);
	vfs_write(a, "world!", 6);
	vfs_close(a);

	a= vfs_open("hello", 0);
	char buf[512];

	int sz = vfs_read(a, buf, 100);
	buf[sz] = '\0';
	uart_printf("%s\n", buf);
	while(1);
}
