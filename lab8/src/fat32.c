#include "fat32.h"
#include "mm.h"
#include "string.h"
#include "sd_driver.h"
#include "mini_uart.h"
#include "vfs.h"
#include "list.h"
#include "tmpfs.h"

struct fat32_metadata fat32_metadata;

struct vnode_operations* fat32_v_ops = 0;
struct file_operations* fat32_f_ops = 0;



static uint32_t get_cluster_blk_idx(uint32_t cluster_idx) {
    return fat32_metadata.data_region_blk_idx +
           (cluster_idx - fat32_metadata.first_cluster) * fat32_metadata.sector_per_cluster;
}

static uint32_t get_fat_blk_idx(uint32_t cluster_idx) {
    return fat32_metadata.fat_region_blk_idx + (cluster_idx / FAT_ENTRY_PER_BLOCK);
}

static uint32_t find_empty_cluster() {
	//find available cluster on FAT table
	int FAT[FAT_ENTRY_PER_BLOCK];
	uint32_t iter_cluster = fat32_metadata.first_cluster;
	readblock(get_fat_blk_idx(iter_cluster), FAT);
	while(1){
		if(iter_cluster % FAT_ENTRY_PER_BLOCK == 0) readblock(get_fat_blk_idx(iter_cluster), FAT);
		//uart_printf_sync("iter_cluster:%d, fat:%X\n", get_fat_blk_idx(iter_cluster), FAT[iter_cluster%FAT_ENTRY_PER_BLOCK]);
		if(FAT[iter_cluster%FAT_ENTRY_PER_BLOCK] == 0x0){
			uart_printf_sync("found empty cluster:%d\n", iter_cluster);
			FAT[iter_cluster%FAT_ENTRY_PER_BLOCK] = EOC;
			break;
		}
		iter_cluster++;
	}
	return iter_cluster;
}

struct vnode* fat32_create_vnode(struct dentry* dentry) {
    struct vnode* vnode = (struct vnode*)kmalloc(sizeof(struct vnode));
    vnode->dentry = dentry;
    vnode->f_ops = fat32_f_ops;
    vnode->v_ops = fat32_v_ops;
    return vnode;
}

struct dentry* fat32_create_dentry(struct dentry* parent, const char* name, int type) {
    struct dentry* dentry = (struct dentry*)kmalloc(sizeof(struct dentry));
	 
	strcpy(name, dentry->name);
    dentry->parent = parent;
	init_list_head(&dentry->list);
    init_list_head(&dentry->childs);
    
	if (parent != 0) {
        list_add(&dentry->list, &parent->childs);
    }
    dentry->vnode = fat32_create_vnode(dentry);
    dentry->mountpoint = 0;
    dentry->type = type;
    return dentry;
}

// error code: -1: already register
int fat32_register() {
    if (fat32_v_ops != 0 && fat32_f_ops != 0) {
        return -1;
    }
    fat32_v_ops = (struct vnode_operations*)kmalloc(sizeof(struct vnode_operations));
    fat32_v_ops->create = fat32_create;
    fat32_v_ops->lookup = fat32_lookup;
    fat32_v_ops->mkdir = fat32_mkdir;
	fat32_v_ops->load_dentry = fat32_load_dentry;
    fat32_f_ops = (struct file_operations*)kmalloc(sizeof(struct file_operations));
    fat32_f_ops->read = fat32_read;
    fat32_f_ops->write = fat32_write;
    return 0;
}

int fat32_setup_mount(struct filesystem* fs, struct mount* mount) {
    mount->fs = fs;
    mount->root = fat32_create_dentry(0, "/", DIRECTORY);
    return 0;
}

int fat32_lookup(struct vnode* dir, struct vnode** target, const char* component_name) {
    // component_name is empty, return dir vnode
    if(strcmp(component_name, "")) {
        *target = dir;
        return 0;
    }
    // search component_name in dir
    struct list_head* p = &dir->dentry->childs;
    list_for_each(p, &dir->dentry->childs) {
        struct dentry* dentry = list_entry(p, struct dentry, list);
        if (strcmp(dentry->name, component_name)) {
            *target = dentry->vnode;
            return 0;
        }
    }
    *target = 0;
    return -1;
}

int fat32_create(struct vnode* dir, struct vnode** target, const char* component_name) {
	uint8_t sector[BLOCK_SIZE];
	struct fat32_internal *dir_internal = (struct fat32_internal *) dir->internal;  
	readblock(get_cluster_blk_idx(dir_internal->first_cluster), sector);
		
	struct fat32_dirent *sector_dirent = (struct fat32_dirent *)sector;
	
	int i = 0;
	while(sector_dirent[i].name[0] != '\0' && sector_dirent[i].name[0] != 0xE5){
		i++;
	}
	int t = 0;
	while(component_name[t]!='.'){ // file name
		sector_dirent[i].name[t] = component_name[t];
		t++;
	}
	
	int e = t + 1; // deal with '.'
	while(t < 8){ // FAT uses 8 bytes to store file name
		sector_dirent[i].name[t] = ' ';
		t++;
	}
	
	while(component_name[e] != '\0'){ // ext name
		sector_dirent[i].ext[t-8] = component_name[e];
		e++;
		t++;
	}
	
	//find available cluster on FAT table
	//int FAT[FAT_ENTRY_PER_BLOCK];
	//uint32_t iter_cluster = fat32_metadata.first_cluster;
	//uart_printf_sync("iter c:%d first fat block:%d\n", iter_cluster, get_fat_blk_idx(iter_cluster));
	//readblock(get_fat_blk_idx(iter_cluster), FAT);
	/*while(found != 1){
		if(iter_cluster % FAT_ENTRY_PER_BLOCK == 0) readblock(get_fat_blk_idx(iter_cluster), FAT);
		//uart_printf_sync("iter_cluster:%d, fat:%X\n", get_fat_blk_idx(iter_cluster), FAT[iter_cluster%FAT_ENTRY_PER_BLOCK]);
		if(FAT[iter_cluster%FAT_ENTRY_PER_BLOCK] == 0x0){
			found = 1;
			uart_printf_sync("found empty cluster:%d\n", iter_cluster);
			sector_dirent[i].cluster_high = iter_cluster >> 16;  
            sector_dirent[i].cluster_low = iter_cluster & 0xFFFF;
			FAT[iter_cluster%FAT_ENTRY_PER_BLOCK] = EOC;
			break;
		}
		iter_cluster++;
	}*/
   
   	uint32_t empty_cluster = find_empty_cluster();

	if(empty_cluster == 0) return -1;

	sector_dirent[i].cluster_high = empty_cluster >> 16;  
    sector_dirent[i].cluster_low = empty_cluster & 0xFFFF;

	sector_dirent[i].attr = 0x20;
    sector_dirent[i].size = 0;
	
    writeblock(get_cluster_blk_idx(dir_internal->first_cluster), sector_dirent);
	
    struct dentry *new_dentry = fat32_create_dentry(dir->dentry, component_name, REGULAR_FILE);
    
	struct fat32_internal* child_internal = (struct fat32_internal*)kmalloc(sizeof(struct fat32_internal));
	child_internal->size = 0;
	child_internal->first_cluster = (sector_dirent[i].cluster_high << 16) | (sector_dirent[i].cluster_low);
	uart_printf_sync("first cluster:%d\n", child_internal->first_cluster);
	child_internal->dirent_cluster = get_cluster_blk_idx(dir_internal->first_cluster);
	
	new_dentry->vnode->internal = (void*)child_internal;

	*target = new_dentry->vnode;
    return 0;
}

int fat32_load_dentry(struct dentry *dir, char *component_name) {
	

	// read first block of cluster
    struct fat32_internal* dir_internal = (struct fat32_internal*)dir->vnode->internal;
    uint8_t sector[BLOCK_SIZE];
    uint32_t dirent_cluster = get_cluster_blk_idx(dir_internal->first_cluster);
    
	readblock(dirent_cluster, sector);

    // parse
    struct fat32_dirent* sector_dirent = (struct fat32_dirent*)sector;
	

    // load all children under dentry
    int found = -1;
    for (int i = 0; sector_dirent[i].name[0] != '\0'; i++) {
        // special value (deleted
        if (sector_dirent[i].name[0] == 0xE5) {
            continue;
        }
        // get filename
        char filename[13];
        int len = 0;
        for (int j = 0; j < 8; j++) {
            char c = sector_dirent[i].name[j];
            if (c == ' ') {
                break;
            }
            filename[len++] = c;
        }
        filename[len++] = '.';
        for (int j = 0; j < 3; j++) {
            char c = sector_dirent[i].ext[j];
            if (c == ' ') {
                break;
            }
            filename[len++] = c;
        }
        filename[len++] = 0;
		if (strcmp(filename, component_name)) {
            found = 0;
        }
        // create dirent
        struct dentry* dentry;
        if (sector_dirent[i].attr == 0x10) {  // directory
            dentry = fat32_create_dentry(dir, filename, DIRECTORY);
        }
        else {  // file
            dentry = fat32_create_dentry(dir, filename, REGULAR_FILE);
        }
        // create fat32 internal
        struct fat32_internal* child_internal = (struct fat32_internal*)kmalloc(sizeof(struct fat32_internal));
		// first_cluster first 2 byte in 0x14 last 2 byte in 0x1a
        child_internal->first_cluster = ((sector_dirent[i].cluster_high) << 16) | (sector_dirent[i].cluster_low);

		uart_printf_sync("name:%s, first_c:%d\n", filename, child_internal->first_cluster);

        child_internal->dirent_cluster = dirent_cluster;
        child_internal->size = sector_dirent[i].size;
        dentry->vnode->internal = child_internal;
    }
    return found;
}

// file operations
int fat32_read(struct file* file, void* buf, uint64_t len) {
    struct fat32_internal* file_node = (struct fat32_internal*)file->vnode->internal;
    uint32_t current_cluster = file_node->first_cluster;
    int remain_len = len;
	int buf_len = 0;
    int fat[FAT_ENTRY_PER_BLOCK];
	int buf2[512];
	// traversal to target cluster using f_pos
	int remain_offset = file->f_pos;
	
	while (remain_offset > 0 && current_cluster >= fat32_metadata.first_cluster && current_cluster != EOC) {
    	readblock(get_fat_blk_idx(current_cluster), fat);
		remain_offset -= BLOCK_SIZE;
		if (remain_offset > 0) {
			readblock(get_fat_blk_idx(current_cluster), fat);
			current_cluster = fat[current_cluster % FAT_ENTRY_PER_BLOCK];
		}
	}

	// read first block	
    int f_pos_offset = file->f_pos % BLOCK_SIZE;
    readblock(get_cluster_blk_idx(current_cluster), buf2);
	int temp = (len > BLOCK_SIZE) ? BLOCK_SIZE - f_pos_offset: len - f_pos_offset;
	memcpy(buf, buf2 + f_pos_offset, temp);

	remain_len -= temp;
	current_cluster = fat[current_cluster % FAT_ENTRY_PER_BLOCK];


	// read the complete block
    while (remain_len > 0 && current_cluster >= fat32_metadata.first_cluster && current_cluster != EOC) {
		memzero(buf2, 512);
		readblock(get_cluster_blk_idx(current_cluster), buf2);
		int cpsize = (remain_len < BLOCK_SIZE) ? remain_len : BLOCK_SIZE;
		memcpy(buf + buf_len, buf2, cpsize);
        buf_len += cpsize;
        remain_len -= BLOCK_SIZE;
		
        // update cluster number from FAT
        readblock(get_fat_blk_idx(current_cluster), fat);
        current_cluster = fat[current_cluster % FAT_ENTRY_PER_BLOCK];
    }


	int res = (len > file_node->size - file->f_pos) ? file_node->size - file->f_pos : len;
	
	uart_printf_sync("buf:%s\nreturn:%d\n", buf, res);

    return res;
}

int fat32_write(struct file* file, const void* buf, uint64_t len) {
    struct fat32_internal* file_node = (struct fat32_internal*)file->vnode->internal;
    uint64_t f_pos_ori = file->f_pos;
    int fat[FAT_ENTRY_PER_BLOCK];
    char write_buf[BLOCK_SIZE];
	memzero(write_buf, BLOCK_SIZE);

    // traversal to target cluster using f_pos
    uint32_t current_cluster = file_node->first_cluster;
	uart_printf_sync("fc:%d, fcb:%d\n", current_cluster, get_cluster_blk_idx(current_cluster));
    int remain_offset = file->f_pos;
    
	while (remain_offset > 0 && current_cluster >= fat32_metadata.first_cluster && current_cluster != EOC) {
        readblock(get_fat_blk_idx(current_cluster), fat);
		remain_offset -= BLOCK_SIZE;
        if (remain_offset > 0) {
            readblock(get_fat_blk_idx(current_cluster), fat);
            current_cluster = fat[current_cluster % FAT_ENTRY_PER_BLOCK];
        }
    }

	// first target block
    // if the f_ops is in the mid of file
	// read it and copy buf to bottom falf then write it back
    int buf_idx, f_pos_offset = file->f_pos % BLOCK_SIZE;
    readblock(get_cluster_blk_idx(current_cluster), write_buf);
	uart_printf_sync("write buf has first : %s", write_buf);
    for (buf_idx = 0; buf_idx < BLOCK_SIZE - f_pos_offset && buf_idx < len; buf_idx++) {
        write_buf[buf_idx + f_pos_offset] = ((char*)buf)[buf_idx];
    }
	uart_printf_sync("write_buf:%s\n", write_buf);
    writeblock(get_cluster_blk_idx(current_cluster), write_buf);
    file->f_pos += buf_idx;

    // write complete block
    int remain_len = len - buf_idx;
    uart_printf_sync("remain_len:%d\n", remain_len);

	//readblock(get_fat_blk_idx(current_cluster), fat);
	//fat[current_cluster % FAT_ENTRY_PER_BLOCK] = EOC;
	
	//readblock(get_fat_blk_idx(current_cluster), fat);
	//current_cluster = fat[current_cluster % FAT_ENTRY_PER_BLOCK];
	//uart_printf_sync("nc:%X\n", current_cluster);
	while (remain_len > 0 && current_cluster >= fat32_metadata.first_cluster) {
        uart_printf_sync("wrong2!\n");
        // write block
        writeblock(get_cluster_blk_idx(current_cluster), buf + buf_idx);
        file->f_pos += (remain_len < BLOCK_SIZE) ? remain_len : BLOCK_SIZE;
        remain_len -= BLOCK_SIZE;
        buf_idx += BLOCK_SIZE;

        // update cluster number from FAT
        if (remain_len > 0) {
            readblock(get_fat_blk_idx(current_cluster), fat);
            if(fat[current_cluster % FAT_ENTRY_PER_BLOCK] != EOC)
				current_cluster = fat[current_cluster % FAT_ENTRY_PER_BLOCK];
			else
				fat[current_cluster % FAT_ENTRY_PER_BLOCK] = find_empty_cluster();
				current_cluster = fat[current_cluster % FAT_ENTRY_PER_BLOCK];
        }
    }

	uart_printf_sync("fpos:%d\n", file->f_pos);

    // update file size
    if (file->f_pos > file_node->size) {
        file_node->size = file->f_pos;

        // update directory entry
        uint8_t sector[BLOCK_SIZE];
        readblock(file_node->dirent_cluster, sector);
        struct fat32_dirent* sector_dirent = (struct fat32_dirent*)sector;
        for (int i = 0; sector_dirent[i].name[0] != '\0'; i++) {
            // special value
            if (sector_dirent[i].name[0] == 0xE5) {
                continue;
            }
            // find target file directory entry
            if (((sector_dirent[i].cluster_high) << 16) | (sector_dirent[i].cluster_low) == file_node->first_cluster) {
                sector_dirent[i].size = (uint32_t)file->f_pos;
            }
        }
        writeblock(file_node->dirent_cluster, sector);
    }
    return file->f_pos - f_pos_ori;
}

int fat32_mkdir(struct vnode* dir, struct vnode** target, const char* component_name) {
}