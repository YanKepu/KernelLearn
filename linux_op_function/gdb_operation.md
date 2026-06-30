# GDB 调试精简手册

---

## 一、启动与退出

### 1.1 基本启动

```bash
gdb ./program                    # 调试可执行文件
gdb ./program core               # 调试 core dump 文件
gdb --tui ./program              # TUI 图形界面模式
gdb --args ./program arg1 arg2   # 带参数启动（--args 后全部作为程序参数）
```

### 1.2 附加到运行中的进程（attach）

当程序已经在运行、卡死或需要在线排查时，可以用 attach 方式接入：

```bash
# 方式一：命令行直接指定 PID
gdb -p <PID>                     # 等价于 gdb --pid=<PID>
gdb -p 1234                      # 例：附加到 PID 为 1234 的进程

# 方式二：先启动 gdb，再 attach
gdb
(gdb) attach 1234                # 在 GDB 内部执行 attach

# 方式三：同时指定可执行文件和 PID（推荐，符号更准确）
gdb ./program -p 1234
```

**attach 常见场景：**

| 场景 | 操作 |
|------|------|
| 程序死锁/卡住 | `gdb -p <PID>` → `bt` 查看调用栈定位卡住位置 |
| 服务在线排查 | `gdb -p <PID>` → 设断点 → `continue` → 触发断点后分析 |
| 多线程问题 | `gdb -p <PID>` → `info threads` + `thread apply all bt` |
| CPU 飙高 | `gdb -p <PID>` → 反复 `bt` 看是否卡在同一位置（热循环） |

**detach 与退出：**

```bash
(gdb) detach                     # 脱离进程，进程继续运行（不会杀掉）
(gdb) quit                       # 退出 GDB（attach 模式下会先自动 detach）
```

> **注意：**
> - attach 需要 root 权限或对进程有 `ptrace` 权限
> - 容器内调试可能需要 `--cap-add=SYS_PTRACE` 或 `--privileged`
> - 被 attach 的进程会**暂停**，设好断点后用 `continue` 恢复运行
> - `detach` 后进程恢复自由运行，GDB 退出不影响进程

### 1.3 退出

| 命令 | 说明 |
|------|------|
| `quit` / `q` | 退出 GDB |
| `Ctrl+D` | 退出 GDB |

---

## 二、断点管理

```bash
break main                       # 在函数设断点 (简写 b)
break file.c:42                  # 在指定行设断点
break file.c:42 if x > 10       # 条件断点
tbreak main                      # 临时断点（命中一次后自动删除）

info breakpoints                 # 查看所有断点 (简写 info b)
delete 1                         # 删除断点1 (简写 del)
disable 1                        # 禁用断点1
enable 1                         # 启用断点1
clear main                       # 清除函数处的断点
```

---

## 三、执行控制

| 命令 | 简写 | 说明 |
|------|------|------|
| `run [args]` | `r` | 从头运行程序 |
| `continue` | `c` | 继续运行到下一个断点 |
| `next` | `n` | 单步跳过（不进入函数） |
| `step` | `s` | 单步进入（进入函数内部） |
| `nexti` | `ni` | 汇编级单步跳过 |
| `stepi` | `si` | 汇编级单步进入 |
| `finish` | | 运行到当前函数返回 |
| `until` | `u` | 运行到循环外（跳出循环） |
| `advance location` | | 运行到指定位置 |

---

## 四、查看数据

### 4.1 变量与表达式

```bash
print x                          # 打印变量值 (简写 p)
print *ptr                       # 打印指针指向的值
print arr[0]@5                   # 打印数组前5个元素
print/x x                        # 十六进制显示
print/d x                        # 十进制显示
print/t x                        # 二进制显示
print/c x                        # 字符显示
```

### 4.2 内存查看

```bash
x/10x 0x400000                  # 查看内存，16进制，10个单元
x/20i $pc                        # 查看20条汇编指令
x/s str_ptr                      # 查看字符串
```

**x 格式：`x/NFU addr`**
- N：数量，F：格式(x/d/s/i/c/t)，U：单元大小(b/h/w/g)

### 4.3 寄存器

```bash
info registers                   # 查看所有寄存器
info registers rax rbx           # 查看指定寄存器
print $rax                       # 打印单个寄存器
```

### 4.4 栈帧

```bash
backtrace                        # 查看调用栈 (简写 bt)
bt full                          # 带局部变量的调用栈
frame 2                          # 切换到第2层栈帧 (简写 f)
info frame                       # 当前栈帧详情
info locals                      # 当前栈帧局部变量
info args                        # 当前函数参数
```

### 4.5 查看函数返回值

断点命中后，想看函数返回值有以下几种方式：

**方式一：`finish` — 最常用，自动显示返回值**

```bash
# 在函数入口或内部打断点
(gdb) break foo
(gdb) continue
# 命中断点后，执行 finish 让函数运行到返回
(gdb) finish
# GDB 自动打印：Value returned is $1 = 42
```

> `finish` 会执行到当前函数 return，然后停下并**自动打印返回值**。
> 返回值同时保存在 GDB 便利变量 `$_` 中，可用 `p $_` 再次查看。

**方式二：在 return 语句打断点，查返回值寄存器**

```bash
# 在 return 那一行打断点
(gdb) break file.c:55        # 假设第55行是 return result;
(gdb) continue
# 命中后打印即将返回的值
(gdb) print result            # 直接看返回表达式

# 或者单步执行到 return 之后，查看返回值寄存器
(gdb) stepi                   # 执行完 ret 指令
(gdb) print $rax              # x86_64 返回值存在 rax 中
(gdb) print $r0               # ARM 返回值存在 r0 中
```

**方式三：断点命令自动捕获返回值**

```bash
# 在函数入口设断点，命中后自动 finish 并记录返回值
(gdb) break foo
(gdb) commands
Type commands for breakpoint(s), end with "end":
> finish
> printf "foo returned: %d\n", $_
> continue
> end
```

**方式四：强制修改返回值**

```bash
# 在函数内打断点后，直接强制返回（不会执行后续代码）
(gdb) return 999              # 强制让当前函数立即返回 999
(gdb) return (int)999         # 带类型转换
```

**总结：**

| 方法 | 命令 | 适用场景 |
|------|------|----------|
| 最常用 | `finish` | 函数内断点处，让函数跑完自动显示返回值 |
| 看寄存器 | `p $rax` | return 语句处，看 ABI 规定的返回值寄存器 |
| 自动捕获 | `commands` + `finish` + `$_` | 反复调用同一函数，自动记录每次返回值 |
| 强制返回 | `return <val>` | 测试不同返回值对调用者的影响 |
| 便利变量 | `p $_` | 引用最近一次 `finish` 的返回值 |

---

## 五、监视点

```bash
watch x                          # 变量改变时暂停（写监视）
rwatch x                         # 变量被读取时暂停
awatch x                         # 变量读写时暂停
info watchpoints                 # 查看监视点
```

---

## 六、修改运行时数据

```bash
set variable x = 10              # 修改变量值 (简写 set var)
set {int}0x400000 = 42           # 修改内存值
set $rax = 0                     # 修改寄存器
```

---

## 七、多线程调试

```bash
info threads                     # 列出所有线程
thread 2                         # 切换到线程2
thread apply all bt              # 所有线程调用栈
break file.c:42 thread 3         # 仅在线程3设断点
```

---

## 八、多进程调试

```bash
set follow-fork-mode child       # fork 后跟子进程
set follow-fork-mode parent      # fork 后跟父进程
set detach-on-fork off           # 不分离，可调试两个进程
info inferiors                   # 列出所有进程
inferior 2                       # 切换到进程2
```

---

## 九、远程调试

```bash
# 目标机启动 gdbserver
gdbserver :1234 ./program

# 主机 GDB 连接
target remote 192.168.1.100:1234
target remote localhost:1234
```

---

## 十、实用技巧

### 10.1 自动显示

```bash
display x                        # 每次暂停自动打印 x
display/i $pc                    # 每次暂停自动显示当前指令
info display                     # 查看自动显示列表
undisplay 1                      # 取消自动显示
```

### 10.2 命令日志与脚本

```bash
set logging on                   # 开启日志记录
set logging file gdb.log         # 指定日志文件
source gdb_cmds.txt              # 执行脚本文件
```

### 10.3 反复执行

```bash
ignore 1 5                       # 忽略断点1接下来5次命中
commands 1                       # 断点1命中时自动执行命令
> print x
> continue
> end
```

### 10.4 编译准备

> **务必加 `-g` 编译选项**，否则无调试符号：
> `gcc -g -o program program.c`

---

## 速查表

| 场景 | 命令 |
|------|------|
| 设断点 | `b main` / `b file.c:42` |
| 条件断点 | `b file.c:42 if x>10` |
| 单步(跳过) | `n` |
| 单步(进入) | `s` |
| 继续运行 | `c` |
| 查看变量 | `p x` |
| 查看内存 | `x/10x addr` |
| 查看调用栈 | `bt` |
| 查看局部变量 | `info locals` |
| 修改变量 | `set var x=10` |
| 查看线程 | `info threads` |
| 所有线程栈 | `thread apply all bt` |
| 远程调试 | `target remote host:port` |
| 查看汇编 | `x/20i $pc` / `disas` |



打个断点
b

结束
finish

(gdb) p $_
$1 = void
(gdb) p/x $rax
$2 = 0x40010005     // 查看 16 进制的
(gdb) p/d $rax
$3 = 1073807365     // 查看 10 进制的

# 定位示例
问题：端口拆分操作失败

```
# 1 查找进程号
bash-4.4# ps -ef | grep drvuse
root        1075       1  3 06:35 ?        00:02:05 /sbin/drvuserd
root       25456   24739  0 07:34 pts/0    00:00:00 grep drvuse

# 进入gdb中 & 关闭多余打印
bash-4.4# gdb -p 1075
(gdb) set print thread-events off 

# 设置断点 & 继续执行
(gdb) break DRV_SYSM_1to4PorcessStep0_NeedRebootOrNot
Breakpoint 1 at 0x7f0b372d4bb0
(gdb) c
Continuing.

# 查看返回值
(gdb) finish
Run till exit from #0  0x00007f0b372d4bb0 in DRV_SYSM_1to4PorcessStep0_NeedRebootOrNot () from /lib/libdrvapp_product.so
0x00007f0b372f37c1 in DRV_PORT_PDT_IOCtl () from /lib/libdrvapp_product.so
(gdb) p $_
$1 = void
(gdb) p/x $rax      ## 直接查看 16 进制的寄存器
$2 = 0x40010005
(gdb) p/d $rax
$3 = 1073807365

# 限定调用者的条件断点
(gdb) b DRV_DATA_GetSubCardInfoPtr if $_caller_is("DRV_SYSM_1to4PorcessStep0_NeedRebootOrNot")


[Switching to Thread 0x7f0b139a0700 (LWP 1147)]

Breakpoint 1, 0x00007f0b372d4bb0 in DRV_SYSM_1to4PorcessStep0_NeedRebootOrNot
    () from /lib/libdrvapp_product.so

# 断点之后查看调用栈和参数
(gdb) bt
#0  0x00007f0b372d4bb0 in DRV_SYSM_1to4PorcessStep0_NeedRebootOrNot ()
   from /lib/libdrvapp_product.so
#1  0x00007f0b372f37c1 in DRV_PORT_PDT_IOCtl () from /lib/libdrvapp_product.so
#2  0x00007f0b383102f3 in DRV_PORT_PhyIoCtl_Legacy ()
   from /lib/libdrvapp_dune.so
#3  0x00007f0b38315798 in DRV_PORT_PhyIoCtl () from /lib/libdrvapp_dune.so
#4  0x00007f0b3dfc4754 in IF_IOCTL_Dispatch () from /lib/libsystem.so
#5  0x00007f0b382fd977 in drv_port_ioctl_routine () from /lib/libdrvapp_dune.so
#6  0x00007f0b3f26633e in start_thread () from /lib/libpthread.so.0
#7  0x00007f0b3ea07d33 in clone () from /lib/libc.so.6
(gdb) p/x $rdi
$1 = 0x11
(gdb) p/x $rsi
$2 = 0x2
(gdb) p/x $rdx
$3 = 0x7f0b16299270
(gdb) finish
Run till exit from #0  0x00007f0b372d4bb0 in DRV_SYSM_1to4PorcessStep0_NeedRebootOrNot () from /lib/libdrvapp_product.so
[New Thread 0x7f0af4678700 (LWP 63501)]
[Thread 0x7f0af4678700 (LWP 63501) exited]
[New Thread 0x7f0af8c7d700 (LWP 63506)]
0x00007f0b372f37c1 in DRV_PORT_PDT_IOCtl () from /lib/libdrvapp_product.so
(gdb) p/x $rax
$4 = 0x40010005

参数1是 0x11  ,参数2 是 0x2 ，参数3 是 0x7f0b16299270 ，返回值是 0x40010005


(gdb) b DRV_SYSM_OPTIC_CheckDisable
Breakpoint 1 at 0x7f0b37301350
(gdb) c
Continuing.
[Switching to Thread 0x7f0b139a0700 (LWP 1147)]

Breakpoint 1, 0x00007f0b37301350 in DRV_SYSM_OPTIC_CheckDisable ()
   from /lib/libdrvapp_product.so
(gdb) bt
#0  0x00007f0b37301350 in DRV_SYSM_OPTIC_CheckDisable ()
   from /lib/libdrvapp_product.so
#1  0x00007f0b372d4f68 in DRV_SYSM_1to4PorcessStep0_NeedRebootOrNot ()
   from /lib/libdrvapp_product.so
#2  0x00007f0b372f37c1 in DRV_PORT_PDT_IOCtl () from /lib/libdrvapp_product.so
#3  0x00007f0b383102f3 in DRV_PORT_PhyIoCtl_Legacy ()
   from /lib/libdrvapp_dune.so
#4  0x00007f0b38315798 in DRV_PORT_PhyIoCtl () from /lib/libdrvapp_dune.so
#5  0x00007f0b3dfc4754 in IF_IOCTL_Dispatch () from /lib/libsystem.so
#6  0x00007f0b382fd977 in drv_port_ioctl_routine () from /lib/libdrvapp_dune.so
#7  0x00007f0b3f26633e in start_thread () from /lib/libpthread.so.0
#8  0x00007f0b3ea07d33 in clone () from /lib/libc.so.6
(gdb) p/x $rdi
$1 = 0x1
(gdb) p/x $rsi
$2 = 0x0
(gdb) p/x $rdx
$3 = 0xe8
(gdb) finish
Run till exit from #0  0x00007f0b37301350 in DRV_SYSM_OPTIC_CheckDisable ()
   from /lib/libdrvapp_product.so
0x00007f0b372d4f68 in DRV_SYSM_1to4PorcessStep0_NeedRebootOrNot ()
   from /lib/libdrvapp_product.so
(gdb) p/x $rax
$4 = 0x40010005


```