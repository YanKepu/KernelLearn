# 文件系统在内核使用

代码路径
include/linux/fs.h

## 关键结构体

### struct file
```c
// info: 表示一个打开的文件实例，包含文件的状态和操作接口

struct file {
    loff_t f_pos;               // 文件当前位置（偏移量）
    atomic_long_t f_count;      // 引用计数
    struct path f_path;         // 文件路径（包含dentry和vfsmount）
    const struct file_operations *f_op; // 文件操作函数表
    // ...
};
```

### struct inode

```c
// info：描述文件的元数据，例如权限、大小、时间戳等
struct inode {
    umode_t i_mode;             // 文件类型和权限
    uid_t i_uid;                // 所有者UID
    gid_t i_gid;                // 所有者GID
    loff_t i_size;              // 文件大小
    struct timespec64 i_atime;  // 最后访问时间
    struct timespec64 i_mtime;  // 最后修改时间
    struct super_block *i_sb;   // 所属超级块
    // ...
};
```

### struct dentry
```c
//info：目录项，连接文件名与iNode
struct dentry {
    struct qstr d_name;         // 文件名
    struct inode *d_inode;      // 关联的inode
    struct dentry *d_parent;    // 父目录项
    // ...
};
```

## 操作方式
### filp_open && filp_close
```c
// 打开文件
struct file *filp_open(const char *filename, int flags, umode_t mode);
/*
参数：
filename: 文件路径（如 /etc/config）。
flags: 打开标志（如 O_RDWR | O_CREAT）。
mode: 文件权限（如 0644）。
返回值：成功返回 struct file*，失败返回错误指针（需用 IS_ERR() 判断）。
*/

#include <linux/fs.h>

struct file *fp;
const char *filename = "/tmp/testfile";
int flags = O_RDWR | O_CREAT;
umode_t mode = 0644;

fp = filp_open(filename, flags, mode);
if (IS_ERR(fp)) {
    pr_err("Failed to open file: %ld\n", PTR_ERR(fp));
    return PTR_ERR(fp);
}

int filp_close(struct file *file, fl_owner_t id);
/*
参数：
file: 要关闭的 struct file*。
id: 通常传 NULL
*/
if (fp) {
    int ret = filp_close(fp, NULL);
    if (ret < 0) {
        pr_err("Close failed: %d\n", ret);
    }
}

```

### kernel_read && kernel_write

```c
ssize_t kernel_read(struct file *file, void *buf, size_t count, loff_t *pos);
ssize_t kernel_write(struct file *file, const void *buf, size_t count, loff_t *pos);

char buf[128];
loff_t pos = 0;
ssize_t ret;

// 读取文件
ret = kernel_read(fp, buf, sizeof(buf), &pos);
if (ret < 0) {
    pr_err("Read failed: %zd\n", ret);
    filp_close(fp, NULL);
    return ret;
}

// 写入文件
strncpy(buf, "Hello from kernel!", sizeof(buf));
ret = kernel_write(fp, buf, strlen(buf), &pos);
if (ret < 0) {
    pr_err("Write failed: %zd\n", ret);
}
```