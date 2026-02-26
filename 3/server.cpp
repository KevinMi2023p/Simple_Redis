#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#endif

static void msg(const char *msg) {
    fprintf(stderr, "%s\n", msg);
}

static void die(const char *msg) {
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

// 6. Read & Write
static void do_something(int connfd) {
    char rbuf[64] = {};
#ifdef _WIN32
    int n = recv(connfd, rbuf, sizeof(rbuf) - 1, 0);
#else
    ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
#endif
    if (n < 0) {
        msg("read() error");
        return;
    }
    printf("client says: %s\n", rbuf);

    char wbuf[] = "world";

#ifdef _WIN32
    send(connfd, wbuf, strlen(wbuf), 0);
#else
    write(connfd, wbuf, strlen(wbuf));
#endif
}

int main() {
    // 1. Obtain socket handle
#ifdef _WIN32
    WSADATA wsaData;
    int wsaErr = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaErr != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", wsaErr);
        return 1;
    }

    SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);
#else
    int fd = socket(AF_INET, SOCK_STREAM, 0);
#endif


    // 2. Set socket options
    char val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    // 3. Bind to address
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = htonl(0);
    int rv = bind(fd, (const struct sockaddr *)&addr, sizeof(addr));
    if (rv) {
        die("bind()");
    }

    // 4. Listen
    rv = listen(fd, SOMAXCONN);
    if (rv) {
        die("bind()");
    }

    // 5. Accept connections
    while (true) {
        struct sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);
        int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen);
        if (connfd < 0) {
            continue;
        }

        do_something(connfd);

#ifdef _WIN32
        closesocket(connfd);
#else
        close(connfd);
#endif
    }

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}