/*
 * The proc filesystem constants/structures
 */
#ifndef _LINUX_PROC_FS_H
#define _LINUX_PROC_FS_H

#include <linux/types.h>
#include <linux/fs.h>

struct proc_dir_entry;

#include <linux/proc_ns.h>

typedef	int (read_proc_t)(char *page, char **start, off_t off,
			  int count, int *eof, void *data);
typedef	int (write_proc_t)(struct file *file, const char __user *buffer,
			   unsigned long count, void *data);
struct proc_dir_entry {
	unsigned int low_ino;
	umode_t mode;
	nlink_t nlink;
	kuid_t uid;
	kgid_t gid;
	loff_t size;
	const struct inode_operations *proc_iops;
	const struct file_operations *proc_fops;
	struct proc_dir_entry *parent;
	struct rb_root subdir;
	struct rb_node subdir_node;
	void *data;
	read_proc_t *read_proc;
	write_proc_t *write_proc;
	int pde_users;	
	atomic_t count;		/* use count */
	atomic_t in_use;	/* number of callers into module in progress; */
			/* negative -> it's going away RSN */
	struct completion *pde_unload_completion;
	struct list_head pde_openers;	/* who did ->open, but not ->release */
	spinlock_t pde_unload_lock; /* proc_fops checks and pde_users bumps */
	u8 namelen;
	char name[];
};

union proc_op {
	int (*proc_get_link)(struct dentry *, struct path *);
	int (*proc_show)(struct seq_file *m,
		struct pid_namespace *ns, struct pid *pid,
		struct task_struct *task);
};

struct proc_inode {
	struct pid *pid;
	int fd;
	union proc_op op;
	struct proc_dir_entry *pde;
	struct ctl_table_header *sysctl;
	struct ctl_table *sysctl_entry;
	const struct proc_ns_operations *ns_ops;
	struct inode vfs_inode;
};

extern struct proc_dir_entry *create_proc_entry(const char *name, mode_t mode,
					 struct proc_dir_entry *parent);
static inline struct proc_dir_entry *create_proc_read_entry(const char *name,
	mode_t mode, struct proc_dir_entry *base, 
	read_proc_t *read_proc, void * data)
{
	struct proc_dir_entry *res=create_proc_entry(name,mode,base);
	if (res) {
		res->read_proc=read_proc;
		res->data=data;
	}
	return res;
}

#ifdef CONFIG_PROC_FS

extern void proc_root_init(void);
extern void proc_flush_task(struct task_struct *);

extern struct proc_dir_entry *proc_symlink(const char *,
		struct proc_dir_entry *, const char *);
extern struct proc_dir_entry *proc_mkdir(const char *, struct proc_dir_entry *);
extern struct proc_dir_entry *proc_mkdir_data(const char *, umode_t,
					      struct proc_dir_entry *, void *);
extern struct proc_dir_entry *proc_mkdir_mode(const char *, umode_t,
					      struct proc_dir_entry *);
 
extern struct proc_dir_entry *proc_create_data(const char *, umode_t,
					       struct proc_dir_entry *,
					       const struct file_operations *,
					       void *);

static inline struct proc_dir_entry *proc_create(
	const char *name, umode_t mode, struct proc_dir_entry *parent,
	const struct file_operations *proc_fops)
{
	return proc_create_data(name, mode, parent, proc_fops, NULL);
}

extern void proc_set_size(struct proc_dir_entry *, loff_t);
extern void proc_set_user(struct proc_dir_entry *, kuid_t, kgid_t);
extern void *PDE_DATA(const struct inode *);
extern void *proc_get_parent_data(const struct inode *);
extern void proc_remove(struct proc_dir_entry *);
extern void remove_proc_entry(const char *, struct proc_dir_entry *);
extern int remove_proc_subtree(const char *, struct proc_dir_entry *);

#else /* CONFIG_PROC_FS */

static inline void proc_root_init(void)
{
}

static inline void proc_flush_task(struct task_struct *task)
{
}

static inline struct proc_dir_entry *proc_symlink(const char *name,
		struct proc_dir_entry *parent,const char *dest) { return NULL;}
static inline struct proc_dir_entry *proc_mkdir(const char *name,
	struct proc_dir_entry *parent) {return NULL;}
static inline struct proc_dir_entry *proc_mkdir_data(const char *name,
	umode_t mode, struct proc_dir_entry *parent, void *data) { return NULL; }
static inline struct proc_dir_entry *proc_mkdir_mode(const char *name,
	umode_t mode, struct proc_dir_entry *parent) { return NULL; }
#define proc_create(name, mode, parent, proc_fops) ({NULL;})
#define proc_create_data(name, mode, parent, proc_fops, data) ({NULL;})

static inline void proc_set_size(struct proc_dir_entry *de, loff_t size) {}
static inline void proc_set_user(struct proc_dir_entry *de, kuid_t uid, kgid_t gid) {}
static inline void *PDE_DATA(const struct inode *inode) {BUG(); return NULL;}
static inline void *proc_get_parent_data(const struct inode *inode) { BUG(); return NULL; }

static inline void proc_remove(struct proc_dir_entry *de) {}
#define remove_proc_entry(name, parent) do {} while (0)
static inline int remove_proc_subtree(const char *name, struct proc_dir_entry *parent) { return 0; }

#endif /* CONFIG_PROC_FS */

struct net;

static inline struct proc_dir_entry *proc_net_mkdir(
	struct net *net, const char *name, struct proc_dir_entry *parent)
{
	return proc_mkdir_data(name, 0, parent, net);
}

#endif /* _LINUX_PROC_FS_H */
