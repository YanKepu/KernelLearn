# 内存问题



## 内存相关配置查看

### 设备内存相关问题

#### /proc/meminfo —— 核心状态文件

linux最终要的内存监控文件，是free、top等工具的数据源，实时记录系统当前内存使用情况

``` 
[root@localhost ~]# cat /proc/meminfo
MemTotal:       16241528 kB		—— 系统总物理内存
MemFree:        11932772 kB		—— 未使用的完全空闲内存
MemAvailable:   11878964 kB		—— 预测的可用内存（考虑可回收缓存）
Buffers:           11724 kB		—— 用于块设备数据的临时缓冲区，通常存放磁盘元数据
Cached:          1870504 kB	—— 用于文件内容的页缓存，在内存充裕时会尽量将文件保存在此
SwapCached:            0 kB		—— 曾经swap out -> swap in且副本仍在的内存
# LRU算法相关
Active:          1083260 kB		—— 最近经常使用的内存
Inactive:        1685640 kB		—— 最近不常使用的内存，优先被回收
Active(anon):    1071536 kB		—— 匿名内存的统计
Inactive(anon):  1685636 kB
Active(file):      11724 kB
Inactive(file):        4 kB
Unevictable:           0 kB
Mlocked:               0 kB
# 交换分区
SwapTotal:             0 kB		—— 交换分区总大小
SwapFree:              0 kB		—— 交换分区剩余可用空间
Dirty:                 0 kB
Writeback:             0 kB
AnonPages:        886708 kB
Mapped:           689176 kB
Shmem:           1870500 kB		—— 进程间共享内存
# 内核内存消耗
Slab:             271120 kB		—— 内核用于存放小数据结构的缓存
SReclaimable:      82096 kB		—— slab中可回收的部分
SUnreclaim:       189024 kB		—— slab中不可回收的部分
KernelStack:       49728 kB		——内核栈占用的内存
PageTables:        49744 kB		—— 用于存储虚拟地址到物理地址映射表的内存
NFS_Unstable:          0 kB		
Bounce:                0 kB
WritebackTmp:          0 kB
CommitLimit:     7596476 kB
Committed_AS:    7021880 kB
VmallocTotal:   34359738367 kB	—— vmalloc虚拟地址空间的总量
VmallocUsed:           0 kB		—— 已通过vmalloc分配的内核虚拟内存的大小
VmallocChunk:          0 kB
CmaTotal:         131072 kB
CmaFree:          130364 kB
HugePages_Total:       1		—— 预分配的静态大页总数
HugePages_Free:        0
HugePages_Rsvd:        0
HugePages_Surp:        0
Hugepagesize:    1048576 kB
DirectMap4k:       12552 kB
DirectMap2M:     2971648 kB
DirectMap1G:    13631488 kB


```

#### /proc/vmstat

记录了系统启动以来，各种虚拟内存管理操作的累计次数，通常需要搭配watch指令去观察

```
watch -n 1 "cat /proc/vmstat | egrep 'pswp|pgscan|pgsteal|pgfault'"

[root@localhost ~]# cat /proc/vmstat
nr_free_pages 2987056              —— 当前空闲内存页数
nr_zone_inactive_anon 421299       —— 区域内非活跃匿名内存页数（准备交换到Swap）
nr_zone_active_anon 265700         —— 区域内活跃匿名内存页数（进程堆栈等）
nr_zone_inactive_file 1            —— 区域内非活跃文件缓存页数（回收首选目标）
nr_zone_active_file 2931           —— 区域内活跃文件缓存页数
nr_slab_reclaimable 20532          —— 可回收的内核SLAB内存（如inode/dentry缓存）
nr_slab_unreclaimable 46764        —— 不可回收的内核SLAB内存
nr_page_table_pages 11613          —— 页表占用的内存页数
nr_kernel_stack 48896              —— 内核栈占用的字节数（注意单位可能是字节或页，取决于内核）
nr_inactive_anon 421299            —— 全局非活跃匿名内存总量
nr_active_anon 265700              —— 全局活跃匿名内存总量
nr_inactive_file 1                 —— 全局非活跃文件缓存（极其低，说明几乎没有可回收的文件缓存）
nr_active_file 2931                —— 全局活跃文件缓存
nr_anon_pages 219488               —— 用户态匿名内存页数（不含Shmem）
nr_mapped 172158                   —— 映射到进程地址空间的文件页数
nr_file_pages 470450               —— 文件缓存总页数（含Shmem）
nr_dirty 0                         —— 等待写入磁盘的脏页数
nr_writeback 0                     —— 正在写入磁盘的页数
nr_shmem 467518                    —— 共享内存/tmpfs占用的页数
nr_dirty_threshold 592258          —— 触发后台写回磁盘的脏页阈值
pgpgin 98726                       —— 从磁盘读入的累计总量 (KB)
pgpgout 19226833                   —— 写入磁盘的累计总量 (KB)
pswpin 0                           —— 从Swap读回内存的累计次数（0表示物理内存稳）
pswpout 0                          —— 换出到Swap的累计次数（0表示没用过虚拟内存）
pgalloc_normal 27336872            —— 在Normal区分配内存的累计次数
allocstall_normal 0                —— 进程因等待内存分配而被迫暂停的次数（0是健康表现）
pgfault 8151815                    —— 累计缺页中断次数
pgmajfault 0                       —— 累计大缺页中断次数（需要读磁盘，0说明性能极好）
pgsteal_direct 702333              —— 【重点】直接回收成功的页数（进程被迫自己去腾空间）
pgscan_direct 26369667             —— 【重点】直接回收扫描的页数（值很高，说明曾有过严重的内存紧张）
pgscan_kswapd 0                    —— 后台回收进程扫描的页数
compact_stall 0                    —— 因碎片整理导致的进程停顿次数
oom_kill 0                         —— OOM Killer 杀掉进程的累计次数nr_free_pages 
```



#### /proc/slabinfo

用于专门查看内核内存分配细节的文件，slab会将内存切成页（4K)切成小块进行分配。

```
name: 内核对象的名称（例如 inode_cache 存放文件索引节点，dentry 存放目录项）。active_objs: 当前正在使用的对象数量。num_objs: 总共已分配的对象数量（包括空闲的）。objsize: 每个对象的字节大小。
objperslab: 一个 Slab 块（通常是一个内存页）能放多少个该对象。
pagesperslab: 每个 Slab 块占用多少个物理页。
```





### 进程下的相关内存配置

#### /proc/[pid]/status

进程内存概览，大多时候用grep搭配使用

```
VmSize: 虚拟内存总量。、
VmRSS: 实际物理内存占用（这是最重要的指标，对应 top 里的 RES）。
VmData: 数据段大小（通常反映了堆内存的使用）。
VmStk: 栈空间大小。
VmExe: 代码段大小（程序本身占用的空间）。


```

#### /proc/[pid]/stat

进程的详细状态信息，以单行数字形式输出，字段较多，常用于脚本解析

```
cat /proc/[pid]/stat

# 关键字段（按位置编号）：
# 字段1:  PID            — 进程ID
# 字段2:  (comm)         — 进程名（被括号包裹）
# 字段3:  state          — 进程状态 (R/S/D/Z/T)
# 字段10: num_threads     — 线程数
# 字段18: num_fds         — 打开的文件描述符数（实际需从 /proc/[pid]/fd 统计）
# 字段20: vsize          — 虚拟内存大小（字节）
# 字段21: rss            — 实际驻留物理内存（页数，乘4得KB）
# 字段24: rsslim         — RSS软限制（字节）
# 字段43: guest_time     — 虚拟化客户机时间

# 实用命令：提取关键字段
PID=$(pidof your_app)
cat /proc/$PID/stat | awk '{print "PID="$1, "VM="$20/1024"KB", "RSS="$21*4"KB", "Threads="$20}'
```

#### /proc/[pid]/smaps

详细分配的数据表

```
PSS（Proportional set size）
如果共享库占10MB，被5个进程共享，那么每个进程RSS只算2MB。
通过查看Private_Dirty可以识别出进程独占且修改过的内存，通常内存泄漏发生在这里
```





### 内核参数相关文件

```
/proc/sys/vm/ 目录下是内核虚拟内存子系统的可调参数，可直接 echo 修改（临时），或写入 /etc/sysctl.conf 持久化
```

#### 常用内核参数

| 路径 | 含义 | 典型值/说明 |
|------|------|-------------|
| `/proc/sys/vm/swappiness` | 内核使用Swap的倾向 | 0~100，默认60；嵌入式常设0~10避免swap |
| `/proc/sys/vm/min_free_kbytes` | 系统保留的最小空闲内存(KB) | 强制保留，用于紧急分配；设太小易OOM |
| `/proc/sys/vm/overcommit_memory` | 内存过量分配策略 | 0=启发式，1=总是允许，2=严格限制 |
| `/proc/sys/vm/overcommit_ratio` | 严格模式下允许的内存超配比例 | 默认50，即物理内存+Swap的50% |
| `/proc/sys/vm/drop_caches` | 手动释放页缓存 | 1=释放pagecache，2=释放dentry/inode，3=全部 |
| `/proc/sys/vm/pagecache_limit_ratio` | 页缓存占用门限 | 超过时触发slab drop回收 |
| `/proc/sys/vm/vfs_cache_pressure` | 回收dentry/inode缓存的倾向 | 默认100，>100更积极回收 |
| `/proc/sys/vm/dirty_ratio` | 脏页占内存百分比触发同步写 | 默认20 |
| `/proc/sys/vm/dirty_background_ratio` | 脏页占比触发后台写回 | 默认10 |
| `/proc/sys/vm/oom_kill_allocating_task` | OOM时是否杀当前分配进程 | 0=选最该杀的，1=杀当前进程 |
| `/proc/sys/vm/panic_on_oom` | OOM时是否触发kernel panic | 0=执行OOM Killer，1=panic，2=强制panic |

#### 实用操作

```bash
# 临时修改参数
echo 10 > /proc/sys/vm/swappiness

# 持久化修改（写入sysctl.conf）
echo "vm.swappiness=10" >> /etc/sysctl.conf
sysctl -p

# 释放缓存（不影响业务，仅清理可回收缓存）
echo 3 > /proc/sys/vm/drop_caches

# 查看当前所有vm参数
sysctl -a | grep "^vm\."
```



### 封装内存申请

```c
#define MID_PRODUCT_BASE	0x0F000000

// 每个模块的定制
#define MID_DRV_INIT		(MID_PRODUCT_BASE + 0xa0000)
#define MID_DRV_SYSM		(MID_PRODUCT_BASE + 0xb0000)

void* drv_malloc_tag(ULONG ulMallocLen, UINT uiUserInfo)
{
	if(1 != g_uiDrvLogDetaiTag)
	{
		uiUserInfo &= 0xffff0000;
	}
	else if((MID_DRV_SDK == (uiUserInfo & 0xffff0000)) && (1 != g_uiDrvLogDetailTag_SDK))
	{
		uiUserInfo &= 0xffff0000;
	}
    return malloc_tag(ulMallocLen, uiUserInfo);
}

extern void * malloc_tag(size_t __size, unsigned int __tag) __attribute_malloc__ ;

#define kmalloc_tag(ulSize,ulFlag, ulMId) malloc(ulSize)

// 实际使用，申请的标签增加标号和行号
pstRpcRet = MEM_Malloc(MID_DRV_SYSM + __LINE__, sizeof(DRV_RPC_SYSM_RET_INTF_U));

void *kmalloc_tag(size_t size, gfp)
```



### 定位示例

检测到 9820-8M设备的0x0f0b01ba出现了内存持续的增长








## 内存上涨导致设备重启

### 原因分析

设备因内存问题重启，本质是可用内存耗尽触发了保护机制：

```
内存持续上涨
    ├─ 用户态泄漏：进程RSS持续增长，堆积至OOM
    ├─ 内核态泄漏：Slab/SUnreclaim持续增长，内核对象未释放
    ├─ 缓存不回收：Cached/Buffers持续增长且不释放
    └─ 突发分配：瞬间大量分配（如网络风暴、大文件操作）
        ↓
可用内存低于门限
    ├─ 触发 kswapd 后台回收
    ├─ 触发 direct reclaim 直接回收（进程阻塞）
    └─ 触发 OOM Killer 杀进程
        ↓
如果OOM Killer杀掉了关键进程 → 设备功能异常/重启
如果内核自身panic_on_oom=1 → 直接kernel panic重启
如果硬件看门狗检测到系统卡死 → 看门狗复位重启
```

### 门限机制

```
内存使用层级：

[MemTotal]
    │
    ├── 正常运行区
    │
    ├── low 水位线 ──── kswapd 被唤醒，开始后台回收
    │
    ├── min 水位线 ──── 触发 direct reclaim，进程在分配时被迫等待
    │
    └── OOM ──── 触发 OOM Killer

# 水位线查看
cat /proc/zoneinfo | grep -E "Node|zone|min|low|high|present|managed"
```

### 定位流程

```bash
# Step 1: 确认是否因OOM重启
dmesg | grep -i "oom\|out of memory\|killed process"
# 如果有 "Out of memory: Kill process" 则确认是OOM

# Step 2: 查看历史日志（如果支持persistent log）
journalctl -k --since "last reboot" | grep -i oom

# Step 3: 确认内存增长来源
# 3a. 全局概览
cat /proc/meminfo | grep -E "MemAvailable|Slab|SUnreclaim|Cached|AnonPages|Shmem"

# 3b. 用户态Top进程
ps aux --sort=-%mem | head -15

# 3c. 内核态Slab
slabtop -s c -o | head -20

# Step 4: 持续监控预防
# 在crontab或启动脚本中添加内存监控
while true; do
    avail=$(grep MemAvailable /proc/meminfo | awk '{print $2}')
    if [ "$avail" -lt 50000 ]; then   # 低于50MB报警
        echo "$(date) WARNING: MemAvailable=${avail}kB" >> /var/log/mem_warning.log
        # 可选：触发dmesg/进程快照
        dmesg >> /var/log/mem_warning.log
        ps aux --sort=-%mem >> /var/log/mem_warning.log
    fi
    sleep 30
done
```

### OOM相关内核参数调优

```bash
# 调整水位线（增大保留内存，让kswapd更早介入）
echo 65536 > /proc/sys/vm/min_free_kbytes      # 保留64MB

# 关闭panic_on_oom（让OOM Killer选择进程而非直接panic）
echo 0 > /proc/sys/vm/panic_on_oom

# 调整swappiness（减少swap抖动，嵌入式常用）
echo 0 > /proc/sys/vm/swappiness

# 增大vfs_cache_pressure（更积极回收dentry/inode缓存）
echo 200 > /proc/sys/vm/vfs_cache_pressure
```





# 内存泄漏

## 概述

内存泄漏是指程序分配的内存未被正确释放，导致可用内存持续减少的现象。在嵌入式设备上，由于资源有限，即使很小的泄漏长时间运行也会导致OOM。

## 分类

```
内存泄漏
├── 用户态泄漏
│   ├── malloc/new 未 free/delete
│   ├── 文件描述符泄漏（未close）
│   ├── 线程泄漏（未join/detach）
│   └── 共享内存未释放
└── 内核态泄漏
    ├── kmalloc/vmalloc 未 kfree/vfree
    ├── sk_buff 未释放
    ├── 内核线程未停止
    └── 模块卸载时未释放资源
```

## 快速判定

```bash
# 1. 确认是否存在泄漏：观察MemAvailable是否持续下降
watch -n 5 'grep -E "MemAvailable|Slab|SUnreclaim|AnonPages" /proc/meminfo'

# 2. 区分用户态/内核态
# MemAvailable↓ + Slab↑        → 内核态泄漏
# MemAvailable↓ + AnonPages↑   → 用户态泄漏
# MemAvailable↓ + Shmem↑       → 共享内存泄漏

# 3. 用户态：找问题进程
ps aux --sort=-%mem | head -10
# 对可疑进程持续监控RSS
watch -n 5 'grep VmRSS /proc/$(pidof your_app)/status'

# 4. 内核态：找问题Slab对象
slabtop -s c -o | head -20
# 对比前后slabinfo
cat /proc/slabinfo > /tmp/slab_before
# ... 运行一段时间 ...
cat /proc/slabinfo > /tmp/slab_after
diff /tmp/slab_before /tmp/slab_after


通过查看/proc/meminfo信息可以大致确认是用户态还是内核态泄漏：
l  slab字段持续增加，说明是kmalloc_tag/kmem_cache_alloc_tag类接口导致的
l  VmallocUsed字段持续增加，说明是vmalloc类接口导致泄漏
l  AnonPages字段持续增加，用户态导致的内存泄漏
l  Cached异常持续增加，通常是/var/log目录下有人持续写log导致 
```

## 详细定位方法

> 详细的内存泄漏定位流程和工具使用，请参考 [MemLeak.md](./MemLeak.md)，包含：
> - 原生工具初步定位（/proc接口、pmap、slabtop）
> - 高级工具深度分析（ASan、kmemleak、KASAN）
> - ARM嵌入式平台特例
> - 内核编译选项

