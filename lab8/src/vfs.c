#include "vfs.h"
#include "mm.h"
#include "string.h"
#include "tmpfs.h"
#include "mini_uart.h"
#include "utils.h"
#include "schedule.h"
#include "initramfs.h"
#include "sd_driver.h"
#include "fat32.h"

struct mount *rootfs;

void init_rootfs() {
    struct filesystem *tmpfs = (struct filesystem*)kmalloc(sizeof(struct filesystem));
    tmpfs->name = (char*)kmalloc(sizeof(char)*6);
    strcpy("tmpfs", tmpfs->name);
    tmpfs->setup_mount = tmpfs_setup_mount;
    register_filesystem(tmpfs);

    rootfs = (struct mount*)kmalloc(sizeof(struct mount));
    tmpfs->setup_mount(tmpfs, rootfs);
    	
	// initramfs
    struct filesystem *initramfs = (struct filesystem*)kmalloc(sizeof(struct filesystem)); 
    initramfs->name = (char*)kmalloc(sizeof(char)*11);
    strcpy("initramfs", initramfs->name);
	register_filesystem(initramfs);
	
	char mountpoint[11] = "/initramfs";
    vfs_mkdir(mountpoint);
    vfs_mount(mountpoint, "initramfs");

	// sd card
	sd_mount();
}

int register_filesystem(struct filesystem *fs) {
    // register the file system to the kernel.
    // you can also initialize memory pool of the file system here.
    if(strcmp(fs->name, "tmpfs")) {
        uart_printf("Register tmpfs\n");
        return tmpfs_register();
    }
	else if(strcmp(fs->name, "initramfs")) {
        uart_printf("Register initramfs\n");
        return initramfs_register();
	}
	else if(strcmp(fs->name, "fat32")) {
		uart_printf("Register fat32\n");
		return fat32_register();
	}
    return -1;
}

struct file *create_fd(struct vnode *target) {
    struct file *fd = (struct file*)kmalloc(sizeof(struct file));
    fd->f_ops = target->f_ops;
    fd->vnode = target;
    fd->f_pos = 0;
    return fd;
}

void traversal_recursive(struct dentry *node, const char *path, struct vnode **target_node, char *target_path) {
    // find next /
    int i = 0;
    while(path[i]) {
        if (path[i] == '/') break;
        target_path[i] = path[i];
        i++;
    }
    target_path[i++] = '\0';
    *target_node = node->vnode;
    // edge cases check
    if(strcmp(target_path, "")) {
        return;
    }
    else if(strcmp(target_path, ".")) {
        traversal_recursive(node, path + i, target_node, target_path);
        return;
    }
    else if(strcmp(target_path, "..")) {
        if(node->parent == 0) { 
			uart_printf("traverse: no parent\n");
            return;
        }
        traversal_recursive(node->parent, path + i, target_node, target_path);
        return;
    }
    // find in node's child
    struct list_head *p;
	int found = 0;
    list_for_each(p, &node->childs) {
        struct dentry *dent = list_entry(p, struct dentry, list);
        if(strcmp(dent->name, target_path)) {
            if(dent->mountpoint != 0) {
                traversal_recursive(dent->mountpoint->root, path + i, target_node, target_path);
            }
            else if(dent->type == DIRECTORY) {
                traversal_recursive(dent, path + i, target_node, target_path);
            }
			found = 1;
			return;
            //break;
        }
    }
	
	// not found in vnode tree, then try to load dentry
    if (!found) {
		uart_printf_sync("need to load %s\n", target_path);
        int ret = node->vnode->v_ops->load_dentry(node, target_path);
        if (ret == 0) { // load success, traversal again
            traversal_recursive(node, path, target_node, target_path);
			uart_printf_sync("load success traversal done!\n");
        }
    }
}

void traversal(const char *pathname, struct vnode **target_node, char *target_path) {
    struct task_t *current_task = get_current_task();
    if(pathname[0] == '/') {  // absolute path
        struct vnode *rootnode = rootfs->root->vnode;
        traversal_recursive(rootnode->dentry, pathname + 1, target_node, target_path);
    }
    else {  // relative path
        struct vnode *rootnode = current_task->pwd->vnode;
        traversal_recursive(rootnode->dentry, pathname, target_node, target_path);
    }
}

struct file *vfs_open(const char *pathname, int flags) {
    // 1. Find target_dir node and target_path based on pathname
    struct vnode *target_dir;
    char target_path[255];
    traversal(pathname, &target_dir, target_path);
    // 2. Create a new file descriptor for this vnode if found.
    struct vnode *target_file;
    if(target_dir->v_ops->lookup(target_dir, &target_file, target_path) == 0) {
        return create_fd(target_file);
    }
    // 3. Create a new file if O_CREAT is specified in flags.
    else {
        if(flags & O_CREAT) {
			uart_printf_sync("need create!\n");
            int res = target_dir->v_ops->create(target_dir, &target_file, target_path);
            if (res < 0) return 0; // error
            target_file->dentry->type = REGULAR_FILE;
            return create_fd(target_file);
        }
        else {
            return 0;
        }
    }
}

int vfs_close(struct file *file) {
    // 1. release the file descriptor
    kfree((void*)file);
    return 0;
}

int vfs_write(struct file *file, const void *buf, uint64_t len) {
    if(file->vnode->dentry->type != REGULAR_FILE) {
        uart_printf("Write to non regular file\n");
        return -1;
    }
    // 1. write len byte from buf to the opened file.
    // 2. return written size or error code if an error occurs.
    
    int res = file->f_ops->write(file, buf, len);
    file->f_pos += res;
    
    return res;
}

int vfs_read(struct file *file, void *buf, uint64_t len) {
    if(file->vnode->dentry->type != REGULAR_FILE) {
        uart_printf("Read on non regular file\n");
        return -1;
    }
    // 1. read min(len, readable file data size) byte to buf from the opened file.
    // 2. return read size or error code if an error occurs.
  
    int res = file->f_ops->read(file, buf, len);
    
    file->f_pos += res;
    return res;
}

uint64_t vfs_get_file_size(struct file *file) {
	return file->f_ops->get_file_size(file);
}

int vfs_mkdir(const char *pathname) {
    struct vnode *target_dir;
    char child_name[255];
    traversal(pathname, &target_dir, child_name);
    struct vnode *child_dir;
    int res = target_dir->v_ops->mkdir(target_dir, &child_dir, child_name);
    if (res < 0) return res; // error
    child_dir->dentry->type = DIRECTORY;
    return 0;
}

int vfs_chdir(const char *pathname) {
    struct vnode *target_dir;
    struct task_t *current_task = get_current_task();
    char path_remain[255];
    traversal(pathname, &target_dir, path_remain);
    if(strcmp(path_remain, "")) { // found
        current_task->pwd = target_dir->dentry;
        return 0;
    }
    else {
		uart_printf("vfs_chdir: path not found\n");
        return -1;
    }
}

// error: -1: not directory, -2: not found -3: unknown filesystem
int vfs_mount(const char *mountpoint, const char *filesystem) {
    // check mountpoint is valid
    struct vnode *mount_dir;
    char path_remain[255];
    traversal(mountpoint, &mount_dir, path_remain);
    if(strcmp(path_remain, "")) {  // found
        if(mount_dir->dentry->type != DIRECTORY) {
			uart_printf("vfs_mount: not dir\n");
            return -1;
        }
    }
    else {
		uart_printf("vfs_mount: mountpoint not found\n");
        return -2;
    }

    // mount fs on mountpoint
    struct mount *mt = (struct mount*)kmalloc(sizeof(struct mount));
    if(strcmp(filesystem, "tmpfs")) {
        struct filesystem *tmpfs = (struct filesystem*)kmalloc(sizeof(struct filesystem));
        tmpfs->name = (char*)kmalloc(sizeof(char)*strlen(filesystem));
        strcpy(filesystem, tmpfs->name);
        tmpfs->setup_mount = tmpfs_setup_mount;
        tmpfs->setup_mount(tmpfs, mt);
        mount_dir->dentry->mountpoint = mt;
		mt->root->parent = mount_dir->dentry->parent;
    }
    else if(strcmp(filesystem, "initramfs")) {
        struct filesystem *initramfs = (struct filesystem*)kmalloc(sizeof(struct filesystem));
        initramfs->name = (char*)kmalloc(sizeof(char)*strlen(filesystem));
        strcpy(filesystem, initramfs->name);
        initramfs->setup_mount = initramfs_setup_mount;
        initramfs->setup_mount(initramfs, mt);
        mount_dir->dentry->mountpoint = mt;
		mt->root->parent = mount_dir->dentry->parent;
    }
	else if(strcmp(filesystem, "fat32")) {	
		struct filesystem *fat32 = (struct filesystem*)kmalloc(sizeof(struct filesystem));
        fat32->name = (char*)kmalloc(sizeof(char)*strlen(filesystem));
        strcpy(filesystem, fat32->name);
		register_filesystem(fat32);
        fat32->setup_mount = fat32_setup_mount;
        fat32->setup_mount(fat32, mt);
        mount_dir->dentry->mountpoint = mt;
		mt->root->parent = mount_dir->dentry->parent;	
	}
	else {
		uart_printf("vfs mount:unknown filesystem\n");
		return -3;
	}

    return 0;
}

