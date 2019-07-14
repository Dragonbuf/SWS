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

#define USER_LIMIT 5
#define BUFFER_SIZE 64 // 读缓冲区大小
#define FD_LIMIT 65535

struct client_data {
    struct sockaddr_in address; // 客户端 socket
    char *write_buf; // 待写到客户端数据的位置
    char buf[BUFFER_SIZE]; // 从客户端读入的数据
};

int setnonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

// 可以直接 ./a.out 127.0.0.1 9503 运行
// 客户端程序使用 poll 同时监听用户输入和网络连接
int main(int argc, char *argv[]) {
    const char *ip = "127.0.0.1";
    int port = atoi("9503");

    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));

    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int listendfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listendfd >= 0);

    ret = bind(listendfd, (struct sockaddr *) &address, sizeof(address));
    assert(ret != -1);

    ret = listen(listendfd, 5);
    assert(ret != -1);

    // 分配 client_data 对象

    struct client_data *users = malloc(sizeof(struct client_data[FD_LIMIT]));

    // 为提高 poll 性能， 限制用户数量
    struct pollfd fds[USER_LIMIT + 1];
    int user_counter = 0;
    for (int i = 1; i <= USER_LIMIT; ++i) {
        fds[i].fd = -1;
        fds[i].events = 0;
    }
    fds[0].fd = listendfd;
    fds[0].events = POLLIN | POLLERR;
    fds[0].revents = 0;

    while (1) {
        ret = poll(fds, user_counter + 1, -1);
        if (ret < 0) {
            printf("poll failure \n");
            break;
        }

        for (int i = 0; i < user_counter + 1; ++i) {
            if ((fds[i].fd == listendfd) && (fds[i].revents & POLLIN)) {
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(listendfd, (struct sockaddr *) &client_address, &client_addrlength);
                if (connfd < 0) {
                    printf("errno is :%d \n", errno);
                    continue;
                }

                // 请求太多，关闭新到的连接
                if (user_counter >= USER_LIMIT) {
                    const char *info = "too many users\n";
                    printf("%s", info);
                    send(connfd, info, strlen(info), 0);
                    close(connfd);
                    continue;
                }

                // 对于新连接，同时修改 fds users
                user_counter++;
                users[connfd].address = client_address;
                setnonblocking(connfd);
                fds[user_counter].fd = connfd;
                fds[user_counter].events = POLLIN | POLLRDHUP | POLLERR;
                fds[user_counter].revents = 0;
                printf("comes a new user ,now have %d users\n", user_counter);
            } else if (fds[i].revents & POLLERR) {
                printf("get an error from %d\n", fds[i].fd);
                char errors[100];
                memset(errors, '\0', 100);
                socklen_t length = sizeof(errors);
                if (getsockopt(fds[i].fd, SOL_SOCKET, SO_ERROR, &errors, &length) < 0) {
                    printf("get socket option failed\n");
                }
                continue;
            } else if (fds[i].revents & POLLRDHUP) {
                users[fds[i].fd] = users[fds[user_counter].fd];
                close(fds[i].fd);
                fds[i] = fds[user_counter];
                i--;
                user_counter--;
                printf(" a client left\n");
            }
        }

    }


    return 0;
}