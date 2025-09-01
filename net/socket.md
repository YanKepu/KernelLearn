# 一 套接字通信
## ISO参考模型和TCP/IP参考模型
ISO是七层，物理层/传输层
应用层：网络服务与最终用户的接口
表示层：数据的表示、安全、压缩：格式jpeg/ascii等
会话层：建立、管理及终止回话：
传输层：定义传输数据的协议，以及流控和差错校验（数据包一旦离开网卡进入网络传输层）
网络层：进行逻辑地址建立、实现不同网络之间的路由选择。
链路层：建立逻辑连接、进行硬件地址寻址、差错检验等功能。
物理层：

核心术语
数据帧（frame）：一种信息单位，起始点都是数据链路层
数据包（Packet）：起始位置是网络层
数据报（Datagram）：通常起始点使用无连接网络服务的网络层的信息单元
段（Segment）：起始点和目的都是传输层的信息单元
消息（Message）：经常用于应用层
元素（cell）：一种固定长度的信息

各层执行任务
主机到网络层负责将信息从一台计算机传输到远程计算机。它处理传输截止的物理性质，并将数据流划分为定长的帧，以便于在发生传输错误时重传数据块。

1. 内核开发工程师将这些头文件存储到一个专门的目录 include/net中，而不是存储到标准位置 include/linux。
2. 计算机之间通信是一个复杂的问题，如何建立物理连接？使用什么样的线缆？通信介质有哪些限制和特殊的要求？如何处理传输错误？如何识别一个计算机？

### 创建套接字
套接字不仅可以用于各种传输协议的IP连接，也可以用于内核支持的所有其他地址和协议类型（例如IPX、appletalk，本地unix套接字

```c
// include/linux/socket.h
struct sockaddr {
	sa_family_t	sa_family;	/* address family, AF_xxx	*/
	char		sa_data[14];	/* 14 bytes of protocol address	*/
};
// sa_family: 是地址家族，一般都是以AF_XXX形式出现，比如大多数都是用AF_INET，代表TCP/IP协议族
// sa_data:是14字节协议地址
//这个数据结构用作bind， connect， recvfrom, sendto等函数的参数，用于指明地址信息


/* Structure describing an Internet socket address.  */
struct sockaddr_in
  {
    __SOCKADDR_COMMON (sin_);
    in_port_t sin_port;			/* Port number.  采用网络数据格式*/
    struct in_addr sin_addr;		/* Internet address. IP地址 */

    /* Pad to size of `struct sockaddr'.  */
    unsigned char sin_zero[sizeof (struct sockaddr) // 只是为了对齐
			   - __SOCKADDR_COMMON_SIZE
			   - sizeof (in_port_t)
			   - sizeof (struct in_addr)];
  };

/* Internet address.  */
typedef uint32_t in_addr_t;
struct in_addr
  {
    in_addr_t s_addr;
  };



```

struct in_addr 其实就是一个32位的IP地址
Struct sockaddr是通用的套接字地址，struct sockaddr_in是Internet环境下套接字的地址形式，二者长度为16字节，指向socket

sockaddr_in结构的指针也可以指向sockaddr，一般情况下，需要吧sockaddr_in结构强制转换成sockaddr结构再传入系统调用函数当中

BSD网络软件中包含两个重要的函数：inet_addr, inet_ntoa。用来在二进制地址格式和点分十进制字符串之间

### 使用套接字
在服务端的模型中，通常提前创建多个子进程，当客户端的请求到来的时候，系统从进程池中选取一个子进程处理客户端的请求，每个子进程处理一个客户端的请求，在全部子进程的处理能力得到满足之前，服务器的网络负载是基本不变的。（只是例子）

server     start -> socket() -> bind() -> fork() -> wait -> end
client     loop (accept() -> recv() -> process data )

TCP 并发服务器简单案例
TCP并发服务器，在处理客户端请求之前，程序先分叉成3个子进程，对应多个客户端的请求，由多个子进程进行处理，与循环服务器相比较，并发的方式在处理请求时，不在只用一个while做串行处理，而是用fork分到子进程中处理。
