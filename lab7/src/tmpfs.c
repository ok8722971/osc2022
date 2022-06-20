#include "tmpfs.h"
#include "list.h"
#include "mm.h"
#include "string.h"
#include "mini_uart.h"

struct vnode_operations *tmpfs_v_ops;
struct file_operations *tmpfs_f_ops;

struct vnode *tmpfs_create_vnode(struct dentry *dentry) {
    struct vnode *vnode = (struct vnode*)kmalloc(sizeof(struct vnode));
    vnode->dentry = dentry;
    vnode->f_ops = tmpfs_f_ops;
    vnode->v_ops = tmpfs_v_ops;
    return vnode;
}

struct dentry *tmpfs_create_dentry(struct dentry *parent, const char *name, int type) {
    struct dentry *dentry = (struct dentry*)kmalloc(sizeof(struct dentry));
    strcpy(name, dentry->name);
    dentry->parent = parent;
    init_list_head(&dentry->list);
    init_list_head(&dentry->childs);
    if(parent != 0) {
        list_add(&dentry->list, &parent->childs);
    }
    dentry->vnode = tmpfs_create_vnode(dentry);
    dentry->mountpoint = 0;
    dentry->type = type;
    return dentry;
}

int tmpfs_register() {
    tmpfs_v_ops = (struct vnode_operations*)kmalloc(sizeof(struct vnode_operations));
    tmpfs_v_ops->create = tmpfs_create;
    tmpfs_v_ops->lookup = tmpfs_lookup;
    tmpfs_v_ops->mkdir = tmpfs_mkdir;
	tmpfs_v_ops->load_dentry = tmpfs_load_dentry;
    tmpfs_f_ops = (struct file_operations*)kmalloc(sizeof(struct file_operations));
    tmpfs_f_ops->read = tmpfs_read;
    tmpfs_f_ops->write = tmpfs_write;
    tmpfs_f_ops->get_file_size = tmpfs_get_file_size;
	return 0;
}

int tmpfs_setup_mount(struct filesystem *fs, struct mount *mount) {
    mount->fs = fs;
    mount->root = tmpfs_create_dentry(0, "/", DIRECTORY);
    return 0;
}

// vnode operations
int tmpfs_lookup(struct vnode *dir, struct vnode **target, const char *component_name) {
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

int tmpfs_create(struct vnode *dir, struct vnode **target, const char *component_name) {
    // create tmpfs internal structure
    struct tmpfs_internal *file_node = (struct tmpfs_internal*)kmalloc(sizeof(struct tmpfs_internal));

    // create child dentry
    struct dentry *d_child = tmpfs_create_dentry(dir->dentry, component_name, REGULAR_FILE);
    d_child->vnode->internal = (void*)file_node;

    *target = d_child->vnode;
    return 0;
}

// file operations
int tmpfs_read(struct file *file, void *buf, uint64_t len) {
    struct tmpfs_internal *file_node = (struct tmpfs_internal*)file->vnode->internal;

    char *dest = (char*)buf;
    char *src = &file_node->buf[file->f_pos];
    uint64_t i = 0;
    for(; i < len && src[i] != (char)EOF; i++) {
    	//uart_printf("%d\n", EOF);
        dest[i] = src[i];
    }
    return i;
}

int tmpfs_write(struct file *file, const void *buf, uint64_t len) {
    struct tmpfs_internal *file_node = (struct tmpfs_internal*)file->vnode->internal;

    char *dest = &file_node->buf[file->f_pos];
    char *src = (char*)buf;
    uint64_t i = 0;
    for(; i < len && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = (char)EOF;
    //uart_printf("%d %d\n", dest[i], EOF);
    return i;
}

int tmpfs_mkdir(struct vnode *dir, struct vnode **target, const char *component_name) {
    // create tmpfs internal structure
    struct tmpfs_internal *dir_node = (struct tmpfs_internal*)kmalloc(sizeof(struct tmpfs_internal));

    // create child dentry
    struct dentry *d_child = tmpfs_create_dentry(dir->dentry, component_name, DIRECTORY);
    d_child->vnode->internal = (void*)dir_node;

    *target = d_child->vnode;
    return 0;
}

int tmpfs_load_dentry(struct dentry* dir, char* component_name) {
    // dummy for compiler
	//uart_printf("tmpfs_load_entry been called!\n");
    return -1;
}

uint64_t tmpfs_get_file_size(struct file *file) {
    struct tmpfs_internal *file_node = (struct tmpfs_internal*)file->vnode->internal;
	return file_node->size;
}
