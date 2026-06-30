# Linux 进程常用操作

---

## 一、查看进程

### 1.1 ps

```bash
ps aux                     # 查看所有进程（BSD 风格）
ps -ef                     # 查看所有进程（SystemV 风格）
ps -eo pid,ppid,cmd,%mem,%cpu --sort=-%mem   # 自定义列，按内存排序
```

**常用组合：**

| 命令 | 说明 |
|------|------|
| `ps aux` | 全量查看，含命令行完整路径 |
| `ps -ef` | 全量查看，含 PPID 父进程 |
| `ps -ef --forest` | 树形显示进程关系 |
| `ps -u root` | 查看指定用户的进程 |
| `ps -p 1234 -o pid,cmd` | 查看指定 PID |
| `ps -C nginx` | 按进程名查找 |

### 1.2 top / htop

```bash
top                        # 实时监控（内置）
htop                       # 增强版（需安装，推荐）
```

**top 交互键：**

| 按键 | 功能 |
|------|------|
| `M` | 按内存排序 |
| `P` | 按 CPU 排序 |
| `k` | 杀死进程 |
| `1` | 展开每个 CPU 核心 |
| `q` | 退出 |

### 1.3 pgrep / pidof

```bash
pgrep nginx                # 按名查找 PID
pgrep -l nginx             # 同时显示进程名
pgrep -u root nginx        # 指定用户
pidof nginx                # 查找 PID（精确匹配）
```

---

### 1.4 fuser  根据文件/设备反查进程

``` 
fuser /dev/ttyS0   # 哪些进程打开了ttyS0
```





### 1.5 lsof  列出进程打开的所有文件

```
lsof /dev/ttyS0
```







## 二、进程树

```bash
pstree                     # 树形显示
pstree -p                  # 带 PID
pstree -u root             # 指定用户
```

---

## 三、终止进程

```bash
kill 1234                  # 发送 SIGTERM（15），优雅退出
kill -9 1234               # 发送 SIGKILL（9），强制杀死
kill -HUP 1234             # 发送 SIGHUP（1），重载配置

killall nginx              # 按名杀死所有同名进程
killall -9 nginx           # 强制杀死

pkill -f "python app.py"   # 按命令行匹配杀死
```

**常用信号：**

| 信号 | 编号 | 说明 |
|------|------|------|
| SIGHUP | 1 | 挂断，常用于重载配置 |
| SIGINT | 2 | 中断，等同 Ctrl+C |
| SIGKILL | 9 | 强杀，不可捕获 |
| SIGTERM | 15 | 终止，可捕获（默认） |
| SIGSTOP | 19 | 暂停，不可捕获 |
| SIGCONT | 18 | 继续运行 |

---

## 四、前后台控制

```bash
command &                  # 后台运行
Ctrl+Z                     # 暂停当前前台进程
bg                         # 将暂停的进程放后台继续
fg                         # 将后台进程调到前台
fg %1                      # 调指定编号的作业到前台

jobs                       # 查看当前会话的后台作业
nohup command &            # 退出终端后继续运行，输出到 nohup.out
nohup command > out.log 2>&1 &
```

---

## 五、进程优先级

```bash
nice -n 10 ./program       # 启动时设置 nice 值（-20~19，越小越优先）
renice -n 5 -p 1234        # 修改运行中进程的 nice 值
renice -n -5 -p 1234       # 提高优先级（需 root）
```

---

## 六、进程资源限制

```bash
ulimit -a                  # 查看所有限制
ulimit -n 65535            # 设置最大文件描述符数
ulimit -u 4096             # 设置最大用户进程数
ulimit -c unlimited        # 允许 core dump 不限大小
```

永久修改需编辑 `/etc/security/limits.conf`：
```
* soft nofile 65535
* hard nofile 65535
```

---

## 七、/proc 文件系统

```bash
cat /proc/1234/status      # 进程状态信息
cat /proc/1234/maps        # 进程内存映射
cat /proc/1234/cmdline     # 完整启动命令
cat /proc/1234/environ     # 环境变量
ls -l /proc/1234/fd        # 打开的文件描述符
cat /proc/1234/stack       # 内核栈（需内核编译选项）
cat /proc/cpuinfo          # CPU 信息
cat /proc/meminfo          # 内存信息
```

---

## 速查表

| 场景 | 命令 |
|------|------|
| 查看所有进程 | `ps aux` |
| 实时监控 | `htop` |
| 按名找 PID | `pgrep -l name` |
| 查看进程树 | `pstree -p` |
| 优雅终止 | `kill PID` |
| 强制杀死 | `kill -9 PID` |
| 按名杀死 | `killall name` |
| 后台运行 | `nohup cmd &` |
| 查看打开文件 | `ls -l /proc/PID/fd` |
| 修改优先级 | `renice -n 5 -p PID` |
| 查看资源限制 | `ulimit -a` |
| 查看进程内存映射 | `cat /proc/PID/maps` |
