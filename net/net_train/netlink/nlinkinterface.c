// 程序功能：应用Netlink套接字从Linux内核打印输出所有网络接口名称

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h> // rtgenmsg/ifinfomsg

#define BUFSIZE 10240

struct ln_request_s
{
    struct nlmsghdr hdr;
    struct rtgenmsg gen;
};

void rtnl_print_link(struct nlmsghdr *h)
{
    struct ifinfomsg *iface;
    struct rtattr *attr;
    int len = 0;

    iface = NLMSG_DATA(h);
    len = RTM_PAYLOAD(h);

    // 循环输出地址
    for (attr = IFLA_RTA(iface); RTA_OK(attr, len); attr = RTA_NEXT(attr, len))
    {
        switch (attr->rta_type)
        {
        case IFLA_IFNAME:
            printf("接口名称%d : %s\n", iface->ifi_index, (char *)RTA_DATA(attr));
            break;

        default:
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    struct sockaddr_nl nkernel;     // 
    struct msghdr msg; // sendmsg
    struct iovec io;

    struct ln_request_s req;
    int s = 0, end = 0, len = 0;
    char buf[BUFSIZE];

    // 构建Netlink
    memset(&nkernel, 0, sizeof(nkernel));   // 清空
    nkernel.nl_family = AF_NETLINK;         // 
    nkernel.nl_groups = 0;                  // 组播组掩码

    // 创建netlink套接字
    if ((s = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0)
    {
        printf("创建Netlink套接字失败.\n");
        exit(1);
    }

    // 为Netlink参数设置
    memset(&req, 0, sizeof(req));
    req.hdr.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtgenmsg)); // 消息长度
    req.hdr.nlmsg_type = RTM_GETLINK;                          // 消息类型
    req.hdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;          // 消息字段取值：消息为请求消息 检索有关表/条目的信息
    req.hdr.nlmsg_seq = 1;                                     // 序列号，用于排列消息处理
    req.hdr.nlmsg_pid = getpid();                              // 发送端口的ID
    req.gen.rtgen_family = AF_INET;

    memset(&io, 0, sizeof(io));
    io.iov_base = &req;
    io.iov_len = req.hdr.nlmsg_len;

    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_name = &nkernel;
    msg.msg_namelen = sizeof(nkernel);

    // 发送消息
    if (sendmsg(s, &msg, 0) < 0)
    {
        printf("结果为：发送消息失败.\n");
    }

    // 解析操作
    while (!end)
    {
        memset(buf, 0, BUFSIZE);
        msg.msg_iov->iov_base = buf;
        msg.msg_iov->iov_len = BUFSIZE;

        if ((len = recvmsg(s, &msg, 0)) < 0)
        {
            printf("结果为：接收消息失败.\n");
        }

        for (struct nlmsghdr *msg_ptr = (struct nlmsghdr *)buf; NLMSG_OK(msg_ptr, len); msg_ptr = NLMSG_NEXT(msg_ptr, len))
        {
            switch (msg_ptr->nlmsg_type)
            {
            case NLMSG_DONE:
                end++;
                break;
            case RTM_NEWLINK:
                rtnl_print_link(msg_ptr);
                break;
            default:
                printf("忽略消息：type=%d , len=%d\n",msg_ptr->nlmsg_type,msg_ptr->nlmsg_len);
                break;
            }
        }
    }

    return 0;
}

