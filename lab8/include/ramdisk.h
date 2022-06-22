#ifndef RAMDISK
#define RAMDISK

void init_cpio();
void cpio_ls();
void cpio_cat(char *filename);
void cpio_exec(char *progname);
char* cpio_get_addr(char *progname);
unsigned long long cpio_get_fsize(char *progname);

#endif
