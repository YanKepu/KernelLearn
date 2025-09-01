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

}

void sig_int(int num)
{
    exit(1);
}

int main(int argc, char*argv[])
{
    int s_s;

    s_s = socket();
}