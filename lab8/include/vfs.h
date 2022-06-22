#ifndef __VFS__
#define __VFS__

#include "type.h"
#include "list.h"

#define O_CREAT 00000100
#define REGULAR_FILE 0
#define DIRECTORY    1
#define NR_OPEN_DEFAULT 16

struct mount {
    struct dentry *root;  // root directory
    struct filesystem *fs;
};

struct filesystem {
    char *name;
    int (*setup_mount)(struct filesystem *fs, struct mount *mount);
};

struct vnode {
    struct vnode_operations *v_ops;
    struct file_operations *f_ops;
    struct dentry *dentry;  // TODO: list of dentry (link)
    void *internal;         // internal representation of a vnode
};

struct dentry {
    char name[32];
    struct dentry *parent;
    struct list_head list;
    struct list_head childs;
    struct vnode *vnode;
    int type;
    struct mount *mountpoint;
};

struct file {
    struct vnode *vnode;
    uint64_t f_pos;  // The next read/write position of this file descriptor
    struct file_operations *f_ops;
    int flags;
};

struct file_operations {
    int (*write)(struct file *file, const void *buf, uint64_t len);
    int (*read)(struct file *file, void *buf, uint64_t len);
	uint64_t (*get_file_size)(struct file *file);
};

struct vnode_operations {
    int (*lookup)(struct vnode *dir, struct vnode **target, const char *component_name);
    int (*create)(struct vnode *dir, struct vnode **target, const char *component_name);
    int (*mkdir)(struct vnode *dir, struct vnode **target, const char *component_name);
	int (*load_dentry)(struct dentry* dir, char *component_name);
};

struct files_struct {
    struct file *fd[NR_OPEN_DEFAULT]; // default array of file objects
};

extern struct mount *rootfs;

void init_rootfs();
int register_filesystem(struct filesystem *fs);
struct file *vfs_open(const char *pathname, int flags);
int vfs_close(struct file *file);
int vfs_write(struct file *file, const void *buf, uint64_t len);
int vfs_read(struct file *file, void *buf, uint64_t len);
uint64_t vfs_get_file_size(struct file *file);
int vfs_mkdir(const char *pathname);
int vfs_chdir(const char *pathname);
int vfs_mount(const char *mountpoint, const char *filesystem);
void traversal(const char *pathname, struct vnode **target_node, char *target_path);

#endif // __VFS__
