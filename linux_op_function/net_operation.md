# Linux 网络常用操作

---

## 一、网络接口查看与配置

### 1.1 ifconfig（传统工具，net-tools）

```bash
# 查看所有网络接口
ifconfig

# 查看指定接口
ifconfig eth0

# 启用/禁用接口
ifconfig eth0 up
ifconfig eth0 down

# 配置 IP 地址
ifconfig eth0 192.168.1.100 netmask 255.255.255.0
ifconfig eth0 192.168.1.100/24

# 修改 MAC 地址
ifconfig eth0 hw ether 00:11:22:33:44:55

# 启用/禁用混杂模式
ifconfig eth0 promisc
ifconfig eth0 -promisc
```

**输出字段说明：**

```
eth1: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
       inet6 fe80::2825:bf36:878d:a109  prefixlen 64  scopeid 0x20<link>
       ether aa:f7:a0:84:69:10  txqueuelen 1000  (Ethernet)
       RX packets 2729  bytes 187564 (187.5 KB)
       RX errors 0  dropped 0  overruns 0  frame 0
       TX packets 11915  bytes 1890318 (1.8 MB)
       TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
       device interrupt 82
       
BRI_MBUS: flags=67<UP,BROADCAST,RUNNING>  mtu 8192
        inet6 fe80::84b0:fbff:fecb:7ebe  prefixlen 64  scopeid 0x20<link>
        ether 20:11:22:33:ee:b0  txqueuelen 1000  (Ethernet)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 14  bytes 1108 (1.0 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
```

| 字段 | 说明 |
|------|------|
| flags | 接口状态标志：UP(已启用)、BROADCAST(广播)、RUNNING(运行中)、MULTICAST(多播) |
| mtu | 最大传输单元，以太网默认 1500 |
| inet/inet6 | IPv4/IPv6 地址 |
| ether | MAC 地址 |
| RX packets/bytes | 接收数据包数/字节数 |
| TX packets/bytes | 发送数据包数/字节数 |
| errors | 错误包数 |
| dropped | 丢弃包数 |
| overruns | 溢出次数（缓冲区不足） |
| frame | 帧对齐错误 |
| carrier | 载波丢失 |
| collisions | 冲突次数 |

### 1.2 ip 命令（推荐，iproute2）

```bash
# 查看所有网络接口
ip addr show
ip a                    # 简写

# 查看指定接口
ip addr show eth0

# 添加 IP 地址
ip addr add 192.168.1.100/24 dev eth0

# 删除 IP 地址
ip addr del 192.168.1.100/24 dev eth0

# 启用/禁用接口
ip link set eth0 up
ip link set eth0 down

# 修改 MAC 地址（需先 down）
ip link set dev eth0 down
ip link set dev eth0 address 00:11:22:33:44:55
ip link set dev eth0 up

# 查看链路状态
ip link show
```

**输出字段说明:**

```
root@topeet:/# ip addr show eth1
4: eth1: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc mq state UP group default qlen 1000
    link/ether aa:f7:a0:84:69:10 brd ff:ff:ff:ff:ff:ff
    inet 192.168.0.155/24 brd 192.168.0.255 scope global eth1
       valid_lft forever preferred_lft forever
```





---

## 二、路由管理

### 2.1 route 命令（传统工具）

```bash
# 查看路由表
route -n                # -n 不解析主机名，显示数字地址

# 添加默认网关
route add default gw 192.168.1.1

# 添加到指定网络的路由
route add -net 192.168.2.0/24 gw 192.168.1.1

# 删除路由
route del default gw 192.168.1.1
route del -net 192.168.2.0/24
```

### 2.2 ip route 命令（推荐）

```bash
# 查看路由表
ip route show
ip route                # 简写

# 添加默认路由
ip route add default via 192.168.1.1

# 添加到指定网络的路由
ip route add 192.168.2.0/24 via 192.168.1.1
ip route add 10.0.0.0/8 via 192.168.1.1 dev eth0

# 删除路由
ip route del 192.168.2.0/24

# 查看到某主机的路由路径
ip route get 8.8.8.8
```

---

## 三、网络连通性测试

### 3.1 ping

```bash
# 测试连通性
ping 192.168.1.1

# 指定次数
ping -c 4 192.168.1.1

# 指定间隔（秒）
ping -i 0.5 192.168.1.1

# 指定包大小
ping -s 1024 192.168.1.1

# ping 网关检测链路
ping -c 3 192.168.1.1
```

### 3.2 traceroute / tracepath

```bash
# 跟踪路由路径
traceroute 8.8.8.8
tracepath 8.8.8.8       # 不需要 root 权限
```

### 3.3 mtr（结合 ping + traceroute）

```bash
mtr 8.8.8.8             # 实时显示路由统计
mtr -r -c 10 8.8.8.8   # 报告模式，10次探测
```

---

## 四、DNS 查询

### 4.1 nslookup

```bash
nslookup www.baidu.com
nslookup www.baidu.com 8.8.8.8    # 指定 DNS 服务器
```

### 4.2 dig

```bash
# 查询 A 记录
dig www.baidu.com

# 查询指定类型记录
dig www.baidu.com A       # IPv4
dig www.baidu.com AAAA    # IPv6
dig www.baidu.com CNAME   # 别名
dig www.baidu.com MX      # 邮件
dig www.baidu.com TXT     # TXT记录

# 简洁输出
dig +short www.baidu.com

# 反向解析
dig -x 192.168.1.1
```

### 4.3 host

```bash
host www.baidu.com
host -t MX baidu.com      # 查询 MX 记录
```

---

## 五、网络连接与端口

### 5.1 netstat（传统工具）

```bash
# 查看所有监听端口
netstat -tlnp
# -t TCP, -l 监听, -n 数字, -p 进程

# 查看所有连接
netstat -anp

# 查看路由表
netstat -rn

# 查看接口统计
netstat -i
```

### 5.2 ss（推荐，更快）

```bash
# 查看所有监听端口
ss -tlnp

# 查看所有 TCP 连接
ss -tanp

# 查看所有 UDP 连接
ss -uanp

# 查看指定端口的连接
ss -tnp | grep :80

# 查看已建立的连接
ss -t state established
```

### 5.3 lsof（查看端口占用）

```bash
# 查看指定端口占用
lsof -i :80
lsof -i :8080

# 查看指定进程的网络连接
lsof -i -a -p 1234

# 查看所有网络连接
lsof -i
```

---

## 六、网络抓包

### 6.1 tcpdump

```bash
# 抓取指定接口的所有包
tcpdump -i eth0

# 抓取指定主机的包
tcpdump -i eth0 host 192.168.1.1

# 抓取指定端口的包
tcpdump -i eth0 port 80

# 抓取 TCP SYN 包
tcpdump -i eth0 'tcp[tcpflags] & tcp-syn != 0'

# 抓取 ICMP 包
tcpdump -i eth0 icmp

# 保存到文件（可用 Wireshark 分析）
tcpdump -i eth0 -w capture.pcap

# 从文件读取
tcpdump -r capture.pcap

# 常用组合
tcpdump -i eth0 -nn -vvv port 80
# -nn 不解析主机名和端口名, -vvv 详细输出
```

---

## 七、网络配置文件

### 7.1 /etc/resolv.conf（DNS 配置）

```bash
nameserver 8.8.8.8
nameserver 114.114.114.114
search localdomain
```

### 7.2 /etc/hosts（本地域名解析）

```bash
127.0.0.1   localhost
192.168.1.100  myserver
```

### 7.3 /etc/network/interfaces（Debian/Ubuntu 静态配置）

```bash
auto eth0
iface eth0 inet static
    address 192.168.1.100
    netmask 255.255.255.0
    gateway 192.168.1.1
    dns-nameservers 8.8.8.8
```

### 7.4 网络配置文件（RHEL/CentOS）

路径：`/etc/sysconfig/network-scripts/ifcfg-eth0`

```bash
DEVICE=eth0
BOOTPROTO=static
ONBOOT=yes
IPADDR=192.168.1.100
NETMASK=255.255.255.0
GATEWAY=192.168.1.1
DNS1=8.8.8.8
```

---

## 八、ARP 与邻居表

```bash
# 查看 ARP 表
arp -n

# 使用 ip 命令查看
ip neigh show
ip n                      # 简写

# 添加静态 ARP
arp -s 192.168.1.1 00:11:22:33:44:55

# 删除 ARP 条目
arp -d 192.168.1.1

# 刷新 ARP 缓存
ip neigh flush all
```

---

## 九、iptables 防火墙

### 9.1 基本操作

```bash
# 查看规则
iptables -L -n -v
iptables -L -n --line-numbers

# 查看指定链
iptables -L INPUT -n -v

# 清除所有规则
iptables -F

# 清除指定链
iptables -F INPUT
```

### 9.2 常用规则

```bash
# 允许已建立的连接
iptables -A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT

# 允许 SSH（端口 22）
iptables -A INPUT -p tcp --dport 22 -j ACCEPT

# 允许 HTTP/HTTPS
iptables -A INPUT -p tcp --dport 80 -j ACCEPT
iptables -A INPUT -p tcp --dport 443 -j ACCEPT

# 允许 ICMP（ping）
iptables -A INPUT -p icmp -j ACCEPT

# 允许本地回环
iptables -A INPUT -i lo -j ACCEPT

# 默认拒绝所有入站
iptables -P INPUT DROP

# NAT 端口转发
iptables -t nat -A PREROUTING -p tcp --dport 8080 -j DNAT --to-destination 192.168.1.100:80

# SNAT（源地址转换）
iptables -t nat -A POSTROUTING -s 192.168.1.0/24 -o eth0 -j MASQUERADE
```

### 9.3 删除规则

```bash
# 按行号删除
iptables -L INPUT --line-numbers
iptables -D INPUT 3

# 按规则删除
iptables -D INPUT -p tcp --dport 22 -j ACCEPT
```

---

## 十、网络服务管理

```bash
# 重启网络服务
systemctl restart networking    # Debian/Ubuntu
systemctl restart network       # RHEL/CentOS

# NetworkManager
nmcli device status             # 查看设备状态
nmcli connection show           # 查看连接
nmcli connection up eth0        # 启用连接
nmcli connection down eth0      # 禁用连接
```

---

## 十一、常用组合速查

| 场景 | 命令 |
|------|------|
| 查看本机 IP | `ip addr show` 或 `ifconfig` |
| 查看网关 | `ip route` 或 `route -n` |
| 查看 DNS | `cat /etc/resolv.conf` |
| 查看端口占用 | `ss -tlnp` 或 `lsof -i :PORT` |
| 测试连通性 | `ping -c 4 HOST` |
| 跟踪路由 | `traceroute HOST` |
| DNS 查询 | `dig +short DOMAIN` |
| 抓包分析 | `tcpdump -i eth0 -w file.pcap` |
| 查看 ARP | `ip neigh` |
| 查看防火墙 | `iptables -L -n` |
| 查看路由路径 | `ip route get 8.8.8.8` |
| 查看 TCP 连接状态 | `ss -t state established` |
| 查看网卡统计 | `ip -s link show eth0` |
