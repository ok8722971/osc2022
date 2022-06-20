#ifndef __INITRAMFS__
#define __INITRAMFS__

#include "vfs.h"
#include "type.h"

#define EOF (-1)

struct initramfs_internal {
    uint64_t size;
    char *p;
};

int initramfs_register();
int initramfs_setup_mount(struct filesystem *fs, struct mount *mount);

// vnode operations
int initramfs_lookup(struct vnode *dir, struct vnode **target, const char *component_name);
int initramfs_create(struct vnode *dir, struct vnode **target, const char *component_name);
int initramfs_mkdir(struct vnode *dir, struct vnode **target, const char *component_name);
int initramfs_load_dentry(struct dentry* dir, char* component_name);

// file operations
int initramfs_read(struct file *file, void *buf, uint64_t len);
int initramfs_write(struct file *file, const void *buf, uint64_t len);
uint64_t initramfs_get_file_size(struct file *file);

#endif // __INITRAMFS__
