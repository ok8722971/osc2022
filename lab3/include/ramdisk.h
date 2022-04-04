#ifndef RAMDISK
#define RAMDISK

void init_cpio();
void cpio_ls();
void cpio_cat(char *filename);
void cpio_exec(char *progname);

#endif
