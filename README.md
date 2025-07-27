# KernelLearn

参考Linux内核版本

<https://www.kernel.org/pub/linux/kernel/v5.x/>

参考版本：v5.6.14

Linux内核主要由几部分组成

进程管理
内存管理
设备驱动
文件系统
网络协议栈
中断管理


内核编译步骤：
1、下载源码： 
wget https://cdn.kernel.org/pub/linux/kernel/v5.x/linux-5.6.14.tar.xz
tar -xf linux-5.6.14.tar.xz
cd linux-5.6.14

2、保存当前内核配置的信息
make menuconfig
通过gedit config查看

3、内核编译
make

4、模块安装操作

sudo make modules_install

5、重启系统查看内核是否编译成功
