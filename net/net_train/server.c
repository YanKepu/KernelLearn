#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#define BUFFLEN 1024
#define SERVER_PORT 8888
#define BACKLOG 5
#define PIDNUMB 3

static void handle_connect(int s_s)
{
    int s_c;
    struct sockaddr_in from;
    socklen_t len = sizeof(from);

    while (1)
    {
        s_c = accept(s_s, (struct sockaddr*)&from, &len);       /* step 4： 接受客户端连接 */
        time_t now;
        char buff[BUFFLEN];
        int n = 0;
        memset(buff, 0, BUFFLEN);
        n = recv(s_c, buff, BUFFLEN, 0);
        if(n>0 && !strncmp(buff, "TIME", 4))
        {
            memset(buff, 0, BUFFLEN);
            now = time(NULL);
            sprintf(buff, "%24s\r\n", ctime(&now));
            send(s_c, buff, strlen(buff), 0);                   /* step 5 : 发送消息 */
        }
        close(s_c);                                             /* step 6：关闭连接 */
    }

    return;
}

void sig_int(int num)
{
    exit(1);
}

int main(int argc, char*argv[])
{
    int s_s;
    struct sockaddr_in local;
    signal(SIGINT, sig_int);

    s_s = socket(AF_INET, SOCK_STREAM, 0);    /* step1:创建socket描述符 */

    memset(&local, 0, sizeof(local));  
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    local.sin_port = htons(SERVER_PORT);

    bind(s_s, (struct sockaddr*)&local, sizeof(local));     /* step 2: bind本地端口*/
    listen(s_s, BACKLOG);                                   /* step 3: 监听server事件 */

    pid_t pid[PIDNUMB];
    int i = 0;
    for(i = 0; i<PIDNUMB; i++)
    {
        pid[i] = fork();
        if(pid[i]==0)
        {
            handle_connect(s_s);                            /* 处理客户端连接 */
        }
    }
    while (1);
    
    close(s_s);
    
    return 0;

}