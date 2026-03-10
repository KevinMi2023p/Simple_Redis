#define _WIN32_WINNT 0x0601

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "Ws2_32.lib")

static void msg(const char *text) {
    fprintf(stderr, "%s\n", text);
}

static void die(const char *text) {
    int err = WSAGetLastError();
    fprintf(stderr, "[%d] %s\n", err, text);
    abort();
}

static void do_something(SOCKET connfd) {
    char rbuf[64];
    ZeroMemory(rbuf, sizeof(rbuf));

    int n = recv(connfd, rbuf, sizeof(rbuf) - 1, 0);
    if (n == SOCKET_ERROR || n == 0) {
        msg("recv() error or connection closed");
        return;
    }

    fprintf(stderr, "client says: %s\n", rbuf);

    {
        const char wbuf[] = "world";
        send(connfd, wbuf, (int)strlen(wbuf), 0);
    }
}

int main(void) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }

    SOCKET fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == INVALID_SOCKET) {
        die("socket()");
    }

    BOOL val = TRUE;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
               (const char *)&val, sizeof(val));

    struct sockaddr_in addr;
    ZeroMemory(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
        die("bind()");
    }

    if (listen(fd, SOMAXCONN) == SOCKET_ERROR) {
        die("listen()");
    }

    while (1) {
        struct sockaddr_in client_addr;
        int addrlen = sizeof(client_addr);
        ZeroMemory(&client_addr, sizeof(client_addr));

        SOCKET connfd = accept(fd,
                               (struct sockaddr *)&client_addr,
                               &addrlen);
        if (connfd == INVALID_SOCKET) {
            continue;
        }

        do_something(connfd);
        closesocket(connfd);
    }

    closesocket(fd);
    WSACleanup();
    return 0;
}