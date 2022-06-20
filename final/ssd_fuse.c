/*
  FUSE ssd: FUSE ioctl example
  Copyright (C) 2008       SUSE Linux Products GmbH
  Copyright (C) 2008       Tejun Heo <teheo@suse.de>
  This program can be distributed under the terms of the GNU GPLv2.
  See the file COPYING.
*/
#define FUSE_USE_VERSION 35
#include <fuse.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include "ssd_fuse_header.h"
#define SSD_NAME       "ssd_file"
#define get_page_idx(pca) (pca.fields.lba + PAGE_PER_BLOCK*pca.fields.nand)


enum
{
    SSD_NONE,
    SSD_ROOT,
    SSD_FILE,
};
FILE *outfp;

static size_t physic_size;
static size_t logic_size;
static size_t host_write_size;
static size_t nand_write_size;

typedef union pca_rule PCA_RULE;
union pca_rule
{
    unsigned int pca;
    struct
    {
        unsigned int lba : 16;
        unsigned int nand: 16;
    } fields;
};

PCA_RULE curr_pca;
static unsigned int get_next_pca();
static void gc();

unsigned int* L2P,* P2L,* valid_count, free_block_number;

static int ssd_resize(size_t new_size)
{
    //set logic size to new_size
    if (new_size > NAND_SIZE_KB * 1024)
    {
        return -ENOMEM;
    }
    else
    {
        logic_size = new_size;
        return 0;
    }

}

static int ssd_expand(size_t new_size)
{
    //logic must less logic limit

    if (new_size > logic_size)
    {
        return ssd_resize(new_size);
    }

    return 0;
}

static int ftl_set_stale(unsigned int p)
{
    PCA_RULE pca;

    if (p == INVALID_PCA) {
        printf("pca:%p is invalid in ftl_set_stale\n", p);
        return -1;
    }

    pca.pca = p;

    P2L[get_page_idx(pca)] = INVALID_LBA;

    return 0;
}

static int nand_read(char* buf, int pca)
{
    char nand_name[100];
    FILE* fptr;

    PCA_RULE my_pca;
    my_pca.pca = pca;
    snprintf(nand_name, 100, "%s/nand_%d", NAND_LOCATION, my_pca.fields.nand);

    //read
    if ( (fptr = fopen(nand_name, "r") ))
    {
        fseek( fptr, my_pca.fields.lba * 512, SEEK_SET );
        fread(buf, 1, 512, fptr);
        fclose(fptr);
    }
    else
    {
        printf("open file fail at nand read pca = %d\n", pca);
        return -EINVAL;
    }
    return 512;
}
static int nand_write(const char* buf, int pca)
{
    char nand_name[100];
    FILE* fptr;

    PCA_RULE my_pca;
    my_pca.pca = pca;
    snprintf(nand_name, 100, "%s/nand_%d", NAND_LOCATION, my_pca.fields.nand);

    //write
    if ( (fptr = fopen(nand_name, "r+")))
    {
        fseek( fptr, my_pca.fields.lba * 512, SEEK_SET );
        fwrite(buf, 1, 512, fptr);
        fclose(fptr);
        physic_size ++;
        valid_count[my_pca.fields.nand]++;
    }
    else
    {
        printf("open file fail at nand (%s) write pca = %d, return %d\n", nand_name, pca, -EINVAL);
        return -EINVAL;
    }

    nand_write_size += 512;
    return 512;
}

static int nand_erase(int block_index)
{
    char nand_name[100];
    FILE* fptr;
    snprintf(nand_name, 100, "%s/nand_%d", NAND_LOCATION, block_index);
    fptr = fopen(nand_name, "w");
    if (fptr == NULL)
    {
        printf("erase nand_%d fail", block_index);
        return 0;
    }
    fclose(fptr);
    
	free_block_number++;
    
	valid_count[block_index] = FREE_BLOCK;
    return 1;
}

static unsigned int get_cur_nand() {
	if(curr_pca.fields.nand < 9) {
		return curr_pca.fields.nand;
	}
	for (int i = 0; i < PHYSICAL_NAND_NUM; i++) {
		if (valid_count[(curr_pca.fields.nand + i) % PHYSICAL_NAND_NUM] == FREE_BLOCK) {
			return (curr_pca.fields.nand + i) % PHYSICAL_NAND_NUM;
		}
	}
}

static unsigned int get_next_block()
{
    for (int i = 0; i < PHYSICAL_NAND_NUM; i++)
    {
        if (valid_count[(curr_pca.fields.nand + i) % PHYSICAL_NAND_NUM] == FREE_BLOCK)
        {
            curr_pca.fields.nand = (curr_pca.fields.nand + i) % PHYSICAL_NAND_NUM;
            curr_pca.fields.lba = 0;
            free_block_number--;
            valid_count[curr_pca.fields.nand] = 0;
            return curr_pca.pca;
        }
    }
    return OUT_OF_BLOCK;
}
static unsigned int get_next_pca()
{
    if (curr_pca.pca == INVALID_PCA)
    {
        //init
        curr_pca.pca = 0;
        valid_count[0] = 0;
        free_block_number--;
        return curr_pca.pca;
    }

    if(curr_pca.fields.lba == 9)
    {
        int temp = get_next_block();
        if (temp == OUT_OF_BLOCK)
        {
			printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
            return OUT_OF_BLOCK;
        }
        else if(temp == -EINVAL)
        {
            return -EINVAL;
        }
        else
        {
            return temp;
        }
    }
    else
    {
        curr_pca.fields.lba += 1;
    }
    return curr_pca.pca;

}

static int ftl_read(char* buf, size_t lba)
{
    // TODO
    if(L2P[lba] == INVALID_PCA) {
		memset(buf, 0, 512);
		return 512;
    }
    return nand_read(buf, L2P[lba]);
}

static int ftl_write(const char* buf, size_t lba_range, size_t lba)
{
    // TODO
	// if the pca(L2P[lba] already in use it means that it is a stale data)
	// doing gc now;
    if(L2P[lba] != INVALID_PCA) {
		gc(lba);
	}
	unsigned int addr = get_next_pca();
    if(addr == INVALID_PCA) return 0;
    int ret = nand_write(buf, addr);
    L2P[lba] = addr;
    PCA_RULE pca;
    pca.pca = addr;
    P2L[get_page_idx(pca)] = lba;
    return ret;
}

static void gc(size_t lba) {
	// 0. record which lba is stale
	PCA_RULE stale_data;
	stale_data.pca = L2P[lba];
	
	ftl_set_stale(stale_data.pca);
	char buf[512];

	// initial to zero for loop
	stale_data.fields.lba = 0;
	
	if(stale_data.fields.nand == get_cur_nand()/*curr_pca.fields.nand*/) {
		int oldlba = curr_pca.fields.lba;
		unsigned int temp =	get_next_block();
		for(int i = 0; i <= oldlba; ++i) {
			if(P2L[get_page_idx(stale_data)] != INVALID_LBA){
				memset(buf, 0, 512);
				nand_read(buf, stale_data.pca);
				unsigned int addr;
				if(i == 0) addr = temp;
				else addr = get_next_pca();
				nand_write(buf, addr);
				
				// maintain L2P and P2L
				size_t l = P2L[get_page_idx(stale_data)];
				L2P[l] = addr;
				PCA_RULE pca;
				pca.pca = addr;
				P2L[get_page_idx(pca)] = l; 

				ftl_set_stale(stale_data.pca);
			}
			stale_data.fields.lba++;
		}
		nand_erase(stale_data.fields.nand);
		
		return;
	}
	
	// gc
	// transfer valid data and maintain L2P and P2L table

	//printf("gc!\n lba:%d\n stale_data.nand:%d\n stale_data.lba:%d\n", lba, stale_data.fields.nand, stale_data.fields.lba);

	
	for(int i = 0; i < PAGE_PER_BLOCK; ++i) {
		if(P2L[get_page_idx(stale_data)] != INVALID_LBA) {

			//printf("valid data!\n lba:%d\n stale_data.nand:%d\n stale_data.lba:%d\n", P2L[get_page_idx(stale_data)], stale_data.fields.nand, stale_data.fields.lba);
			// transfer valid data		
			memset(buf, 0, 512);
			nand_read(buf, stale_data.pca);
			unsigned int addr = get_next_pca();
			nand_write(buf, addr);

			// maintain L2P and P2L
			//printf("%d %d\n", stale_data.fields.nand, stale_data.fields.lba);
			size_t l = P2L[get_page_idx(stale_data)];
			//printf("valid data l:%d\n", l);
			L2P[l] = addr;
			PCA_RULE pca;
			pca.pca = addr;
			//printf("%d\n", get_page_idx(pca));
			P2L[get_page_idx(pca)] = l; 

			ftl_set_stale(stale_data.pca);
		}
		stale_data.fields.lba++;
	}
	//printf("block:%d been erase!\n", stale_data.fields.nand);
	nand_erase(stale_data.fields.nand);
}

static int ssd_file_type(const char* path)
{
    if (strcmp(path, "/") == 0)
    {
        return SSD_ROOT;
    }
    if (strcmp(path, "/" SSD_NAME) == 0)
    {
        return SSD_FILE;
    }
    return SSD_NONE;
}
static int ssd_getattr(const char* path, struct stat* stbuf,
                       struct fuse_file_info* fi)
{
    (void) fi;
    stbuf->st_uid = getuid();
    stbuf->st_gid = getgid();
    stbuf->st_atime = stbuf->st_mtime = time(NULL);
    switch (ssd_file_type(path))
    {
        case SSD_ROOT:
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;
            break;
        case SSD_FILE:
            stbuf->st_mode = S_IFREG | 0644;
            stbuf->st_nlink = 1;
            stbuf->st_size = logic_size;
            break;
        case SSD_NONE:
            return -ENOENT;
    }
    return 0;
}
static int ssd_open(const char* path, struct fuse_file_info* fi)
{
    (void) fi;
    if (ssd_file_type(path) != SSD_NONE)
    {
        return 0;
    }
    return -ENOENT;
}
static int ssd_do_read(char* buf, size_t size, off_t offset)
{
    int tmp_lba, tmp_lba_range, rst ;
    char* tmp_buf;

    //off limit
    if ((offset ) >= logic_size)
    {
        return 0;
    }
    if ( size > logic_size - offset)
    {
        //is valid data section
        size = logic_size - offset;
    }

    tmp_lba = offset / 512;
    tmp_lba_range = (offset + size - 1) / 512 - (tmp_lba) + 1;
    tmp_buf = calloc(tmp_lba_range * 512, sizeof(char));

    for (int i = 0; i < tmp_lba_range; i++) {
        // TODO
        ftl_read(tmp_buf + i*512, tmp_lba + i);
    }

    memcpy(buf, tmp_buf + offset % 512, size);

    
    free(tmp_buf);
    return size;
}
static int ssd_read(const char* path, char* buf, size_t size,
                    off_t offset, struct fuse_file_info* fi)
{
    (void) fi;
    if (ssd_file_type(path) != SSD_FILE)
    {
        return -EINVAL;
    }
	fprintf(outfp,"ssd_read- size: %d offset:%d \n", size, offset); 
    return ssd_do_read(buf, size, offset);
}
static int ssd_do_write(const char* buf, size_t size, off_t offset)
{
    int tmp_lba, tmp_lba_range, process_size;
    int idx, curr_size, remain_size, rst;
    char* tmp_buf;

    host_write_size += size;
    if (ssd_expand(offset + size) != 0)
    {
        return -ENOMEM;
    }

    tmp_lba = offset / 512;
    tmp_lba_range = (offset + size - 1) / 512 - (tmp_lba) + 1;
	
	//printf("ssd_do_write!\n offset:%d\n size:%d\n tmp_lba:%d\n, tmp_lba_range:%d\n", offset, size, tmp_lba, tmp_lba_range);

    process_size = 0;
    remain_size = size;
    curr_size = 0;

    for (idx = 0; idx < tmp_lba_range; idx++)
    {
        // TODO
        // if the idx is the first block or the last
        // we need to check if it is align to 512
        // if not, we need to read it first to prevent some info lost
        if((idx == tmp_lba_range-1 && (offset+size)%512 != 0) || (idx == 0 && offset%512 != 0)){
            char *tmp_buf = calloc(512, sizeof(char));
            int start = 0,end = 512;
            ftl_read(tmp_buf, tmp_lba + idx);
            
            if(idx == tmp_lba_range-1 && (offset+size)%512 != 0) end = (offset+size)%512;
            if(idx == 0 && offset%512 != 0) start = offset%512;
            for(int i = start; i < end; i++) {
                tmp_buf[i] = buf[process_size++];
            }
            
            ftl_write(tmp_buf, 512, tmp_lba + idx);
     		free(tmp_buf);   
		}else {
            //ftl_write(buf + 512*idx, 512, tmp_lba + idx);
            //process_size += 512;
			char *tmp_buf = calloc(512, sizeof(char));
			for(int i = 0; i < 512; ++i) {
				tmp_buf[i] = buf[process_size++];
			}
			ftl_write(tmp_buf, 512, tmp_lba + idx);
			free(tmp_buf);
        }
    }
    return size;
}
static int ssd_write(const char* path, const char* buf, size_t size,
                     off_t offset, struct fuse_file_info* fi)
{

    (void) fi;
    if (ssd_file_type(path) != SSD_FILE)
    {
        return -EINVAL;
    }
	fprintf(outfp,"ssd_write- size: %d offset:%d \n", size, offset); 
    return ssd_do_write(buf, size, offset);
}
static int ssd_truncate(const char* path, off_t size,
                        struct fuse_file_info* fi)
{
    (void) fi;
    memset(L2P, INVALID_PCA, sizeof(int) * LOGICAL_NAND_NUM * PAGE_PER_BLOCK);
    memset(P2L, INVALID_LBA, sizeof(int) * PHYSICAL_NAND_NUM * PAGE_PER_BLOCK);
    memset(valid_count, FREE_BLOCK, sizeof(int) * PHYSICAL_NAND_NUM);
    curr_pca.pca = INVALID_PCA;
    free_block_number = PHYSICAL_NAND_NUM;
    if (ssd_file_type(path) != SSD_FILE)
    {
        return -EINVAL;
    }
	fprintf(outfp,"ssd_truncate- size: %d\n", size);
    return ssd_resize(size);
}
static int ssd_readdir(const char* path, void* buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info* fi,
                       enum fuse_readdir_flags flags)
{
    (void) fi;
    (void) offset;
    (void) flags;
    if (ssd_file_type(path) != SSD_ROOT)
    {
        return -ENOENT;
    }
    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);
    filler(buf, SSD_NAME, NULL, 0, 0);
    return 0;
}
static int ssd_ioctl(const char* path, unsigned int cmd, void* arg,
                     struct fuse_file_info* fi, unsigned int flags, void* data)
{

    if (ssd_file_type(path) != SSD_FILE)
    {
        return -EINVAL;
    }
    if (flags & FUSE_IOCTL_COMPAT)
    {
        return -ENOSYS;
    }
    switch (cmd)
    {
        case SSD_GET_LOGIC_SIZE:
            *(size_t*)data = logic_size;
            return 0;
        case SSD_GET_PHYSIC_SIZE:
            *(size_t*)data = physic_size;
            return 0;
        case SSD_GET_WA:
            *(double*)data = (double)nand_write_size / (double)host_write_size;
            return 0;
    }
    return -EINVAL;
}
static const struct fuse_operations ssd_oper =
{
    .getattr        = ssd_getattr,
    .readdir        = ssd_readdir,
    .truncate       = ssd_truncate,
    .open           = ssd_open,
    .read           = ssd_read,
    .write          = ssd_write,
    .ioctl          = ssd_ioctl,
};
int main(int argc, char* argv[])
{
	outfp=fopen("input.txt","w");
	int idx;
    char nand_name[100];
    physic_size = 0;
    logic_size = 0;
    curr_pca.pca = INVALID_PCA;
    free_block_number = PHYSICAL_NAND_NUM;

    L2P = malloc(LOGICAL_NAND_NUM * PAGE_PER_BLOCK * sizeof(int));
    memset(L2P, INVALID_PCA, sizeof(int) * LOGICAL_NAND_NUM * PAGE_PER_BLOCK);
    P2L = malloc(PHYSICAL_NAND_NUM * PAGE_PER_BLOCK * sizeof(int));
    memset(P2L, INVALID_LBA, sizeof(int) * PHYSICAL_NAND_NUM * PAGE_PER_BLOCK);
    valid_count = malloc(PHYSICAL_NAND_NUM * sizeof(int));
    memset(valid_count, FREE_BLOCK, sizeof(int) * PHYSICAL_NAND_NUM);

    //create nand file
    for (idx = 0; idx < PHYSICAL_NAND_NUM; idx++)
    {
        FILE* fptr;
        snprintf(nand_name, 100, "%s/nand_%d", NAND_LOCATION, idx);
        fptr = fopen(nand_name, "w");
        if (fptr == NULL)
        {
            printf("open fail");
        }
        fclose(fptr);
    }
    return fuse_main(argc, argv, &ssd_oper, NULL);
}

