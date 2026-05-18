# 嵌入式 Linux 内存泄漏定位指南

## 一、定位思路

```
第一步：原生工具初步定位
    ├─ 确认泄漏是否存在 (/proc/meminfo)
    ├─ 区分用户态/内核态 (slabinfo/ps)
    └─ 缩小范围 (maps/smaps/slabtop)

第二步：高级工具深度分析
    ├─ 用户态: ASAN/Valgrind
    └─ 内核态: KASAN/kmemleak
```

---

## 二、第一步：原生工具初步定位

### 2.1 确认泄漏是否存在

```bash
# 持续监控内存变化
while true; do
    echo "=== $(date) ==="
    cat /proc/meminfo | grep -E "MemTotal|MemFree|MemAvailable|Buffers|Cached|Slab|SUnreclaim"
    sleep 10
done
```

关键指标：
| 指标 | 含义 | 泄漏信号 |
|------|------|----------|
| MemAvailable | 可用内存 | 持续下降 |
| Slab | 内核对象缓存 | 持续增长 |
| SUnreclaim | 不可回收Slab | 持续高位 |

### 2.2 区分用户态 vs 内核态泄漏

```bash
# 方法1: 对比 Slab 与进程占用
echo "=== 内核 Slab 占用 ==="
cat /proc/slabinfo | awk '{sum+=$2} END {print "Total Slab: " sum/1024 " KB"}'

echo "=== Top 进程内存占用 ==="
ps aux --sort=-%mem | head -10
```

判断依据：
- **MemAvailable 下降 + Slab 增长** → 内核态泄漏
- **MemAvailable 下降 + 某进程 RSS 增长** → 用户态泄漏

### 2.3 用户态定位 - 原生工具

#### 2.3.1 实时监控单个进程内存变化

**方法1: 使用 top/htop 监控单进程**

```bash
# 只监控指定进程，1秒刷新
top -p $(pidof your_app) -d 1

# 或查看线程级内存
top -H -p $(pidof your_app) -d 1
```

关键列解释：
| 列 | 含义 | 泄漏信号 |
|----|------|----------|
| VIRT | 虚拟内存总量 | 持续上升 |
| RES | 物理内存占用(RSS) | 持续上升 |
| SHR | 共享内存 | 通常稳定 |
| %MEM | 内存占用百分比 | 持续上升 |

**方法2: 脚本持续记录进程内存**

```bash
#!/bin/bash
PID=$(pidof your_app)
LOGFILE="/tmp/mem_monitor_$(date +%Y%m%d_%H%M%S).log"

echo "Time,PID,VIRT(KB),RSS(KB),SHR(KB),%MEM,DATA(KB),STACK(KB)" > $LOGFILE

while true; do
    # 从 statm 读取原始数据 (单位: 页)
    read -r size resident shared text lib data dt <<< $(cat /proc/$PID/statm)
    
    # 转换为 KB (默认页大小 4KB)
    VIRT=$((size * 4))
    RSS=$((resident * 4))
    SHR=$((shared * 4))
    DATA=$((data * 4))
    
    # 获取栈大小
    STACK=$(grep -oP 'VmStack:\s+\K\d+' /proc/$PID/status || echo 0)
    
    # 获取内存百分比
    MEM_PCT=$(ps -p $PID -o %mem --no-headers | tr -d ' ')
    
    echo "$(date '+%H:%M:%S'),$PID,$VIRT,$RSS,$SHR,$MEM_PCT,$DATA,$STACK" >> $LOGFILE
    sleep 5
done
```

**方法3: 使用 pidstat 详细统计**

```bash
# 每秒输出进程内存变化
pidstat -p $(pidof your_app) -r 1

# 输出解释：
# minflt/s: 每秒 minor faults
# majflt/s: 每秒 major faults  
# VSZ: 虚拟内存(KB)
# RSS: 物理内存(KB)
# %MEM: 内存百分比
```

**方法4: 监控 /proc/[pid]/status 关键字段**

```bash
#!/bin/bash
PID=$(pidof your_app)

while true; do
    echo "=== $(date) ==="
    grep -E "^(Vm|Rss|Threads)" /proc/$PID/status
    echo ""
    sleep 5
done
```

关键字段：
| 字段 | 说明 |
|------|------|
| VmSize | 虚拟内存总量 |
| VmRSS | 实际物理内存 |
| VmData | 数据段大小(堆) |
| VmStk | 栈大小 |
| VmExe | 代码段 |
| VmLib | 共享库 |
| Threads | 线程数(泄漏可能伴随线程泄漏) |

#### 2.3.2 /proc/[pid]/maps 查看进程内存布局

```bash
# 查看进程内存段分布
cat /proc/$(pidof your_app)/maps

# 统计各段大小
cat /proc/$(pidof your_app)/maps | awk '{print $6}' | sort | uniq -c | sort -rn

# 持续监控堆和匿名映射变化
watch -n 2 'cat /proc/$(pidof your_app)/maps | grep -E "\[heap\]|\[anon\]"'
```

关注：
- `[heap]` 堆区是否持续增大
- 大量匿名映射 `[anon]` 未释放

#### 2.3.3 /proc/[pid]/smaps 详细分析

```bash
# 查找内存占用大的区域
cat /proc/$(pidof your_app)/smaps | grep -E "^Size|^Rss|^Pss" | head -50

# 统计匿名页占用
awk '/^Size:/{size=$2} /^Anonymous:/{if($2>0) print size, $2}' /proc/$(pidof your_app)/smaps | sort -k2 -rn | head -20

# 计算各类内存总和
awk '
/^Size:/{total+=$2}
/^Rss:/{rss+=$2}
/^Anonymous:/{anon+=$2}
/^Swap:/{swap+=$2}
END {
    print "Total Size:", total, "kB"
    print "RSS:", rss, "kB"
    print "Anonymous:", anon, "kB"
    print "Swap:", swap, "kB"
}' /proc/$(pidof your_app)/smaps
```

#### 2.3.4 pmap 快速查看

```bash
# 按占用排序
pmap -x $(pidof your_app) | sort -k3 -rn | head -20

# 只显示总量
pmap $(pidof your_app) | tail -1

# 持续监控
watch -n 2 'pmap -x $(pidof your_app) | tail -5'
```

### 2.4 内核态定位 - 原生工具

#### 2.4.1 slabtop 实时监控

```bash
# 按占用排序，观察变化
watch -n 2 'slabtop -s c -o | head -30'
```

常见泄漏对象：
| 对象名 | 说明 |
|--------|------|
| kmalloc-xxx | 通用内核分配 |
| task_struct | 进程/线程未回收 |
| skbuff_head_cache | 网络包缓存泄漏 |
| dentry/inode | 文件系统缓存 |

#### 2.4.2 /proc/slabinfo 定量分析

```bash
# 记录基线
cat /proc/slabinfo > /tmp/slab_before

# 运行一段时间后对比
cat /proc/slabinfo > /tmp/slab_after

# 找出增长的对象
diff /tmp/slab_before /tmp/slab_after | grep "^>" | awk '{print $2, $3}'
```

#### 2.4.3 /proc/meminfo 追踪分配器

```bash
# 查看页分配器状态
cat /proc/pagetypeinfo

# 查看 vmalloc 区域
cat /proc/vmallocinfo | head -50
```

#### 2.4.4 追踪模块内存

```bash
# 查看内核模块占用 (如果支持)
cat /proc/modules | awk '{print $1, $2}'

# 通过 debugfs 查看 per-cpu 占用
cat /sys/kernel/debug/per_cpu_stats 2>/dev/null || echo "debugfs not mounted"
```

---

## 三、第二步：高级工具深度分析

### 3.1 用户态 - AddressSanitizer (ASan)

编译选项：
```bash
CFLAGS += -fsanitize=address -fno-omit-frame-pointer -g
LDFLAGS += -fsanitize=address
```

运行输出：
```
==1234==ERROR: AddressSanitizer: memory leak
Direct leak of 1024 byte(s) in 1 object(s) allocated from:
    #0 malloc
    #1 foo() at test.c:10
    #2 main at test.c:20
```

环境变量控制：
```bash
# 设置报告输出路径
export ASAN_OPTIONS=log_path=/tmp/asan.log

# 继续运行不退出
export ASAN_OPTIONS=halt_on_error=0
```

### 3.2 用户态 - mtrace (嵌入式轻量)

```c
#include <mcheck.h>
int main() {
    mtrace();
    // ... 业务代码
    muntrace();
    return 0;
}
```

```bash
export MALLOC_TRACE=/tmp/mtrace.log
./your_program
mtrace ./your_program $MALLOC_TRACE > /tmp/leak_report.txt
```

### 3.3 内核态 - Kmemleak

需要内核开启 `CONFIG_DEBUG_KMEMLEAK=y`：

```bash
# 挂载 debugfs
mount -t debugfs nodev /sys/kernel/debug/

# 触发扫描
echo scan > /sys/kernel/debug/kmemleak

# 查看结果
cat /sys/kernel/debug/kmemleak
```

常用命令：
```bash
echo clear > /sys/kernel/debug/kmemleak      # 清空记录
echo off > /sys/kernel/debug/kmemleak        # 关闭追踪
echo scan=600 > /sys/kernel/debug/kmemleak   # 自动扫描(秒)
```

代码标记：
```c
#include <linux/kmemleak.h>
void* ptr = kmalloc(1024, GFP_KERNEL);
kmemleak_not_leak(ptr);   // 标记为非泄漏
// 或
kmemleak_ignore(ptr);     // 完全忽略
```

### 3.4 内核态 - KASAN

内核配置：
```
CONFIG_KASAN=y
CONFIG_KASAN_GENERIC=y
CONFIG_DEBUG_KERNEL=y
```

启动参数：
```
kasan.stacktrace=off    # 减少开销
```

检测能力：
- Use-after-free
- Out-of-bounds
- Double-free

---

## 四、嵌入式实战流程

```
发现内存不足/OOM
       ↓
Step 1: 原生工具初步定位
    ├─ cat /proc/meminfo 确认泄漏
    ├─ cat /proc/slabinfo vs ps aux 区分内核/用户态
    ↓
Step 2: 缩小范围
    ├─ 用户态: pmap + /proc/[pid]/smaps 定位问题进程和区域
    └─ 内核态: slabtop + slabinfo 定位问题对象
    ↓
Step 3: 高级工具深度分析
    ├─ 用户态: 编译带 ASan 版本复现
    └─ 内核态: 开启 kmemleak 扫描
    ↓
Step 4: 修复验证
    └─ 长时间稳定性测试
```

---

## 五、工具选择速查

| 阶段 | 用户态工具 | 内核态工具 | 侵入性 |
|------|-----------|-----------|--------|
| **初步定位** | `pmapsmaps` | `slabinfo` | 无 |
| **缩小范围** | `pmap` | `slabtop` | 无 |
| **深度分析** | ASan | kmemleak | 中 |
| **极端检测** | Valgrind | KASAN | 高 |

---

## 六、ARM 嵌入式平台特例

### 6.1 通过 JTAG/Debugger 监控

```bash
# OpenOCD 读取内存
openocd -f board/stm32f4discovery.cfg -c "init; mdw 0x20000000 1024"
```

### 6.2 CMA/ION 驱动内存追踪

```bash
cat /proc/cma
cat /sys/kernel/debug/ion/ion_heap
```

### 6.3 预留内存监控

```bash
cat /sys/kernel/debug/memblock/reserved
```

---

## 七、必备内核编译选项

```
# 原生调试信息
CONFIG_DEBUG_FS=y
CONFIG_DEBUG_KERNEL=y

# Slab 调试
CONFIG_SLUB_DEBUG=y
CONFIG_SLUB_DEBUG_ON=y

# Kmemleak
CONFIG_DEBUG_KMEMLEAK=y

# KASAN (高侵入性)
CONFIG_KASAN=y
CONFIG_KASAN_GENERIC=y

# 符号信息
CONFIG_DEBUG_INFO=y
```
