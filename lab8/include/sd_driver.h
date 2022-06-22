#ifndef SD_DRIVER
#define SD_DRIVER

void sd_init();
int sd_mount();
void writeblock(int block_idx, void* buf);
void readblock(int block_idx, void* buf);

#endif
