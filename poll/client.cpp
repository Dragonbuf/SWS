#define _GNU_SOURCE 1

#include <poll.h>
#include "sys/socket.h"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "assert.h"
#include "stdlib.h"
#include "unistd.h"
#include "errno.h"
#include "string.h"
#include "fcntl.h"
#include "stdio.h"

#define BUFFER_SIZE 64 // 读缓冲区大小


// 可以直接 ./a.out 127.0.0.1 9503 运行
// 客户端程序使用 poll 同时监听用户输入和网络连接
int main(int argc, char *argv[]) {
//    if (argc <= 2) {
//        printf("should :[ip] [port] \n");
//        return 1;
//    }

    const char *ip = "127.0.0.1";
    int port = atoi("9503");

    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &server_address.sin_addr);
    server_address.sin_port = htons(port);

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);

    int res = connect(sockfd, (struct sockaddr *) &server_address, sizeof(server_address));
    if (res < 0) {
        printf("connect failed \n");
        return 1;
    }

    struct pollfd fds[2];
    // 注册文件描述符 0（标准输入） 和文件描述符 sockfd 上的可读事件
    fds[0].fd = 0;
    fds[0].events = POLLIN;
    fds[0].revents = 0;
    fds[1].fd = sockfd;
    fds[1].events = POLLIN | POLLRDHUP;
    fds[1].revents = 0;

    char read_buf[BUFFER_SIZE];
    int pipefd[2];
    int ret = pipe(pipefd);
    assert(ret != -1);

    while (1) {
        ret = poll(fds, 2, -1);
        if (ret < 0) {
            printf("poll failure \n");
            break;
        }

        if (fds[1].revents & POLLRDHUP) {
            printf("server close the connection \n");
            break;
        } else if (fds[1].revents & POLLIN) {
            memset(read_buf, '\0', BUFFER_SIZE);
            recv(fds[1].fd, read_buf, BUFFER_SIZE - 1, 0);
            printf("%s\n", read_buf);
        }

        if (fds[0].revents & POLLIN) {
            // 使用 splice 将用户输入直接写到 sockfd 上
            ret = splice(0, NULL, pipefd[1], NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
            ret = splice(pipefd[0], NULL, sockfd, NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
        }
    }

    close(sockfd);


    return 0;

}