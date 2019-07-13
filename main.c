#include "sys/socket.h"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "assert.h"
#include "stdlib.h"
#include "unistd.h"
#include "errno.h"
#include "string.h"
#include "fcntl.h"

#define BUFFER_SIZE 4096 // 读缓冲区大小

// 主机状态： 正在分析请求行，正在分析头部字段
enum CHECK_STATE {
    CHECK_STATE_REQUEST_LINE = 0,
    CHECK_STATE_HEADER
};

// 行的读取状态： 读取到一个完整的行，行出错， 行数据尚且不完整
enum LINE_STATUS {
    LINE_OK = 0,
    LINE_BAD,
    LINE_OPEN
};

enum HTTP_CODE {
    NO_REQUEST, // 请求不完整
    GET_REQUEST, // 获得了一个完整的客户端请求
    BAD_REQUEST, // 客户端语法有错误
    FORBIDDEN_REQUEST,
    INTERNAL_ERROR,
    CLOSED_CONNECTION // 客户端已经关闭连接
};

static const char *szret[] = {"I get a correct result\n", "something wrong\n"};

// 从状态机解析出一行内容
enum LINE_STATUS parse_line(char *buffer, int checked_index, int read_index) {
    char temp;
    for (; checked_index < read_index; ++checked_index) {
        temp = buffer[checked_index];

        if (temp == '\r') {
            if ((checked_index + 1) == read_index) {
                return LINE_OPEN;
            } else if (buffer[checked_index + 1] == '\n') {
                buffer[checked_index++] = '\0';
                buffer[checked_index++] = '\0';
                return LINE_OK;
            }

            return LINE_BAD;
        } else if (temp == '\n') {
            if (checked_index > 1 && buffer[checked_index - 1] == '\r') {
                buffer[checked_index - 1] = '\0';
                buffer[checked_index++] = '\0';
                return LINE_OK;
            }

            return LINE_BAD;
        }

    }

    return LINE_OPEN;
}

// 分析请求行
enum HTTP_CODE parse_requestLine(char *temp, enum CHECK_STATE checkstate) {
    char *url = strpbrk(temp, " \t");
    if (!url) {
        return BAD_REQUEST;
    }
    *url++ = '\0';

    char *method = temp;
    if (strcasecmp(method, "GET") == 0) {
        printf("The request method id GET\n");
    } else {
        return BAD_REQUEST;
    }

    url += strspn(url, " \t");
    char *version = (char *) strspn(url, " \t");
    if (!version) {
        return BAD_REQUEST;
    }

    *version++ = '\0';
    version += strspn(version, " \t");
    if (strcasecmp(version, "HTTP/1.1") != 0) {
        return BAD_REQUEST;
    }

    if (strncasecmp(url, "http://", 7) == 0) {
        url += 7;
        url = strchr(url, '/');
    }

    if (!url || url[0] != '/') {
        return BAD_REQUEST;
    }

    printf("The request URL is %s\n", url);

    checkstate = CHECK_STATE_HEADER;
    return NO_REQUEST;
}

int main(int argc, char *argv[]) {
    if (argc <= 2) {
        printf("usage : %s ip_address port_number\n", basename(argv[0]));
        return 1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int listendf = socket(PF_INET, SOCK_STREAM, 0);
    assert(listendf > 0);
    int ret = bind(listendf, (struct sockaddr *) &address, sizeof(address));
    assert(ret != -1);

    ret = listen(listendf, 5);

    struct sockaddr_in client_address;
    socklen_t client_addrlength = sizeof(client_address);
    int fd = accept(listendf, (struct sockaddr *) &client_address, &client_addrlength);

    if (fd < 0) {
        printf("error is %d\n", errno);
    } else {

    }

    close(listendf);
    return 0;

}