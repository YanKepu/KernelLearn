

### 查看CPU实际利用率



## CPU重要属性

### /proc/cpuinfo

用来展示每个CPU核的硬件信息

```
[root@H3C flash:]# cat /proc/cpuinfo
processor       : 0
BogoMIPS        : 50.00
Features        : fp asimd evtstrm aes pmull sha1 sha2 crc32 atomics fphp asimdhp cpuid asimdrdm lrcpc dcpop asimddp
CPU implementer : 0x41
CPU architecture: 8
CPU variant     : 0x2
CPU part        : 0xd05
CPU revision    : 0

processor       : 1
BogoMIPS        : 50.00
Features        : fp asimd evtstrm aes pmull sha1 sha2 crc32 atomics fphp asimdhp cpuid asimdrdm lrcpc dcpop asimddp
CPU implementer : 0x41
CPU architecture: 8
CPU variant     : 0x2
CPU part        : 0xd05
CPU revision    : 0

```



#### /sys/devices/system/cpu/cpuX/online

控制/查看某个cpu核是否在线，1——在线可用， 0——离线

```
[root@H3C flash:]# cat /sys/devices/system/cpu/cpu0/online 
1
```







#### 查看是否每个核都在调度任务

```
[root@H3C flash:]# cat /sys/devices/system/cpu/cpu*/online
1
1
```



#### 看per-CPU运行队列长度

```
cat /proc/schedstat
```



#### 看中断是否均衡分布

```
[root@H3C flash:]# cat /proc/interrupts | head -20
           CPU0       CPU1       
  1:          0          0     GICv3  22 Level     vgic
  3:          0          0     GICv3  24 Level     kvm guest ptimer
  4:          0          0     GICv3  26 Level     kvm guest vtimer
  5:   39224432   38227080     GICv3  23 Level     arch_timer
  6:          0          0     GICv3  28 Level     arm-pmu
  7:       5358          0     GICv3 115 Level     ttyS0
  8:          0          0     GICv3 119 Level     mv64xxx_i2c
  9:          0          0     GICv3 120 Level     mv64xxx_i2c
 10:      46419          0     GICv3  77 Level     eth0
 12:          0          0     GICv3  99 Level     ehci_hcd:usb1
 14:          2          0     GICv3 122 Level     805a0000.spi
 15:      16094          0     GICv3 124 Level     mmc0
IPI0:   5909266    7894987       Rescheduling interrupts
IPI1:        30         28       Function call interrupts
IPI2:         0          0       CPU stop interrupts
IPI3:         0          0       CPU stop (for crash dump) interrupts
IPI4:         0          0       Timer broadcast interrupts
IPI5:         0          0       IRQ work interrupts
IPI6:         0          0       CPU wake-up interrupts
```





