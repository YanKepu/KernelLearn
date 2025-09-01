#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8888
#define BUFFER_SIZE 1024

int main() {
    int client_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // 1. 创建 TCP socket
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket 创建失败");
        exit(EXIT_FAILURE);
    }

    // 2. 设置服务端地址
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("地址转换失败");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    // 3. 连接到服务端
    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect 失败");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    printf("已连接到服务端 %s:%d\n", SERVER_IP, PORT);

    // 4. 发送消息
    const char *message = "TIME";
    if (send(client_fd, message, strlen(message), 0) == -1) {
        perror("send 失败");
    }

    // 5. 接收响应
    ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received == -1) {
        perror("recv 失败");
    } else {
        buffer[bytes_received] = '\0';
        printf("服务端响应: %s\n", buffer);
    }

    // 6. 关闭连接
    close(client_fd);
    return 0;
}