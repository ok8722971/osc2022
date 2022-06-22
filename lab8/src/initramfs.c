#include "initramfs.h"
#include "list.h"
#include "mm.h"
#include "string.h"
#include "mini_uart.h"
#include "ramdisk.h"

struct vnode_operations *initramfs_v_ops;
struct file_operations *initramfs_f_ops;

struct vnode *initramfs_create_vnode(struct dentry *dentry) {
    struct vnode *vnode = (struct vnode*)kmalloc(sizeof(struct vnode));
    vnode->dentry = dentry;
    vnode->f_ops = initramfs_f_ops;
    vnode->v_ops = initramfs_v_ops;
    return vnode;
}

struct dentry *initramfs_create_dentry(struct dentry *parent, const char *name, int type) {
    struct dentry *dentry = (struct dentry*)kmalloc(sizeof(struct dentry));
    strcpy(name, dentry->name);
    dentry->parent = parent;
    init_list_head(&dentry->list);
    init_list_head(&dentry->childs);
    if(parent != 0) {
        list_add(&dentry->list, &parent->childs);
    }
    dentry->vnode = initramfs_create_vnode(dentry);
    dentry->mountpoint = 0;
    dentry->type = type;
    return dentry;
}

int initramfs_register() {
    initramfs_v_ops = (struct vnode_operations*)kmalloc(sizeof(struct vnode_operations));
    initramfs_v_ops->create = initramfs_create;
    initramfs_v_ops->lookup = initramfs_lookup;
    initramfs_v_ops->mkdir = initramfs_mkdir;
	initramfs_v_ops->load_dentry = initramfs_load_dentry;
    initramfs_f_ops = (struct file_operations*)kmalloc(sizeof(struct file_operations));
    initramfs_f_ops->read = initramfs_read;
    initramfs_f_ops->write = initramfs_write;
	initramfs_f_ops->get_file_size = initramfs_get_file_size;
    return 0;
}

int initramfs_setup_mount(struct filesystem *fs, struct mount *mount) {
    mount->fs = fs;
    mount->root = initramfs_create_dentry(0, "/", DIRECTORY);
    return 0;
}

// vnode operations
int initramfs_lookup(struct vnode *dir, struct vnode **target, const char *component_name) {
    // component_name is empty, return dir vnode
    if(strcmp(component_name, "")) {
        *target = dir;
        return 0;
    }
    // search component_name in dir
    struct list_head *p;
    list_for_each(p, &dir->dentry->childs) {
        struct dentry *dentry = list_entry(p, struct dentry, list);
        if (strcmp(dentry->name, component_name)) {
            *target = dentry->vnode;
            return 0;
        }
    }
    *target = 0;
    return -1;
}

int initramfs_create(struct vnode *dir, struct vnode **target, const char *component_name) {
    uart_printf("create on initramfs\n");
    return -1;
}

// file operations
int initramfs_read(struct file *file, void *buf, uint64_t len) {
    struct initramfs_internal *file_node = (struct initramfs_internal*)file->vnode->internal;

    char *dest = (char*)buf;
    char *src = &file_node->p[file->f_pos];
	uint64_t size = file_node->size;
    uint64_t i = 0;
    for(; i < len && file->f_pos+i < size; i++) {
    	//uart_printf("%d\n", EOF);
        dest[i] = src[i];
    }
    return i;
}

int initramfs_write(struct file *file, const void *buf, uint64_t len) {
    uart_printf("write on initramfs\n");
    return 0;
}

int initramfs_mkdir(struct vnode *dir, struct vnode **target, const char *component_name) {
    uart_printf("mkdir on initramfs\n");
    return -1;
}

int initramfs_load_dentry(struct dentry* dir, char* component_name) {
	struct dentry* dentry = initramfs_create_dentry(dir, component_name, REGULAR_FILE);
	struct initramfs_internal* child_internal = (struct initramfs_internal*)kmalloc(sizeof(struct initramfs_internal));
	child_internal->size = cpio_get_fsize(component_name);
	child_internal->p = cpio_get_addr(component_name);
	dentry->vnode->internal = child_internal;
	return 0;
}

uint64_t initramfs_get_file_size(struct file *file) {
    struct initramfs_internal *file_node = (struct initramfs_internal*)file->vnode->internal;
	return file_node->size;
}
