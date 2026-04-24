# GDB调试使用手册

## 一、GDB简介

### 什么是GDB
GDB（GNU Debugger）是GNU项目开发的强大调试工具，用于调试C、C++等编程语言的程序。它可以帮助开发者：
- 查看程序的运行状态
- 设置断点和观察点
- 单步执行程序
- 检查变量和内存
- 分析程序崩溃原因

### 编译调试信息
使用GCC编译程序时，需要添加`-g`选项生成调试信息：
```bash
gcc -g -o program program.c
```

### 启动GDB
```bash
gdb program                    # 启动并加载程序
gdb program corefile           # 调试core文件
gdb -p pid                    # 附加到正在运行的进程
gdb --args program arg1 arg2   # 传递命令行参数
```

## 二、基本命令

### 运行控制
```gdb
run [args]            # 运行程序，可带参数，直接重新执行了，一般不要用，除非调试阶段
start                 # 在main函数开始处暂停
continue              # 继续执行
next                  # 单步执行，不进入函数
step                  # 单步执行，进入函数
finish                # 执行到当前函数返回
until [location]      # 执行到指定位置
quit                  # 退出GDB
```

### 断点管理
```gdb
break [location]      # 设置断点
break function_name   # 在函数处设置断点
break filename:line   # 在指定文件的指定行设置断点
break *address        # 在内存地址处设置断点
tbreak [location]     # 设置临时断点（只触发一次）
disable [n]           # 禁用断点
enable [n]            # 启用断点
delete [n]            # 删除断点
clear [location]      # 清除指定位置的断点
info breakpoints      # 查看断点信息
```

### 查看变量和内存
```gdb
print var             # 打印变量值
print &var            # 打印变量地址
print *ptr            # 打印指针指向的值
print array[0]@10     # 打印数组的前10个元素
print sizeof(var)     # 打印变量大小
print /x var          # 以十六进制格式打印
print /d var          # 以十进制格式打印
print /c var          # 以字符格式打印
print /s var          # 以字符串格式打印
display var           # 每次暂停时自动显示变量
undisplay [n]         # 取消自动显示
```

### 查看堆栈
```gdb
backtrace             # 查看堆栈调用链
backtrace full        # 显示堆栈中的局部变量
frame [n]             # 切换到指定栈帧
info frame            # 查看当前栈帧信息
info locals           # 查看当前栈帧的局部变量
info args             # 查看当前函数的参数
```

### 查看源代码
```gdb
list                  # 显示当前位置的源代码
list function_name    # 显示指定函数的源代码
list filename:line    # 显示指定文件的指定行
list start,end        # 显示指定范围的源代码
set listsize n        # 设置每次显示的行数
```

### 修改变量值
```gdb
set var=value         # 修改变量值
set {type}address=value  # 修改内存地址的值
```

## 三、高级功能

### 条件断点
```gdb
break location if condition   # 设置条件断点
watch expression             # 当表达式值改变时暂停
rwatch expression            # 当表达式被读时暂停
awatch expression            # 当表达式被读或写时暂停
```

### 调试多线程
```gdb
info threads                # 查看所有线程
thread [id]                 # 切换到指定线程
break location thread [id]  # 在指定线程设置断点
set scheduler-locking off   # 所有线程都运行（默认）
set scheduler-locking on    # 只有当前线程运行
```

### 调试多进程
```gdb
set follow-fork-mode parent  # 调试父进程（默认）
set follow-fork-mode child   # 调试子进程
set detach-on-fork on        # 分离未调试的进程（默认）
set detach-on-fork off       # 保持所有进程
```

### 内存操作
```gdb
x/nfu address               # 检查内存
  n: 显示的单元数
  f: 显示格式（x:十六进制，d:十进制，u:无符号，o:八进制，t:二进制，a:地址，i:指令，c:字符，s:字符串）
  u: 单元大小（b:字节，h:半字，w:字，g:双字）
```

### 自定义命令
```gdb
define cmdname             # 定义自定义命令
  commands...
end
document cmdname           # 为自定义命令添加文档
```

## 四、调试技巧

### 常用快捷键
- `Ctrl+C`: 暂停程序执行
- `Ctrl+D`: 退出GDB
- `Tab`: 命令补全
- `Up/Down`: 历史命令

### 调试核心转储
```bash
# 生成core文件
ulimit -c unlimited
./program

# 调试core文件
gdb program core
```

### 远程调试
```bash
# 目标机器启动gdbserver
gdbserver host:port program

# 开发机连接
gdb program
target remote host:port
```

### 调试动态库
```gdb
set solib-search-path /path/to/libs  # 设置动态库搜索路径
set auto-solib-add on               # 自动加载共享库符号
```

### 优化调试体验
```gdb
set print pretty on                 # 美化结构体打印
set print array on                  # 打印数组索引
set print elements 0                # 不限制打印元素数量
set pagination off                  # 关闭分页显示
```

## 五、实战示例

### 示例1：基本调试流程
```gdb
gdb ./program
break main
run
next
print variable
continue
quit
```

### 示例2：调试段错误
```gdb
gdb ./program core
backtrace
frame 0
print *ptr
```

### 示例3：调试多线程程序
```gdb
gdb ./thread_program
info threads
thread 2
break function_name
continue
```

### 示例4：调试循环中的问题
```gdb
break loop_function
run
while (condition)
  print counter
  continue
end
```

### 示例5：内存泄漏检测
```gdb
set malloc-warn on
break malloc
commands
  print $rdi
continue
end
```

## 六、常见问题解决

### 符号无法加载
- 确保编译时添加了`-g`选项
- 检查动态库路径设置
- 使用`file`命令重新加载符号文件

### 断点无法设置
- 确认源代码路径正确
- 检查是否在优化编译（-O）下调试
- 使用`info source`查看当前源文件信息

### 调试优化代码
```gdb
set disassembly-flavor intel    # 设置反汇编风格
disassemble function_name       # 查看函数反汇编
```

### 调试C++程序
```gdb
set print demangle on           # 启用名称修饰
print 'namespace::class::function'  # 打印命名空间中的函数
```

## 七、GDB配置文件

创建`.gdbinit`文件在用户主目录或当前目录：
```gdb
# .gdbinit
set print pretty on
set pagination off
set history save on
set history size 1000
define hook-run
  break main
end
```

## 八、扩展工具

### GDB可视化前端
- **DDD**: Data Display Debugger，图形化调试工具
- **cgdb**: 带语法高亮的终端界面
- **Eclipse CDT**: IDE集成调试
- **VS Code**: 支持GDB调试

### 辅助脚本
- **GDB Python API**: 使用Python扩展GDB功能
- **GDB Dashboard**: 增强的GDB界面
- **pwndbg**: 面向漏洞利用的GDB插件

通过以上内容，您可以全面掌握GDB的使用方法，提高调试效率。实践是最好的学习方式，建议在实际项目中多使用GDB进行调试。