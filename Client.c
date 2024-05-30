#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

void error(const char *msg) {
    fprintf(stderr, "%s: %d\n", msg, WSAGetLastError());
    exit(1);
}

int main(int argc, char *argv[]) {
    WSADATA wsa;
    SOCKET sockfd;
    struct addrinfo hints, *servinfo, *p;
    char buffer[256];
    int n;
    const char *hostname = "127.0.0.1";
    const char *port = "8080";

    if (argc >= 2) {
        hostname = argv[1];
    }

    if (argc >= 3) {
        port = argv[2];
    }


    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        error("WSAStartup failed");
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(hostname, port, &hints, &servinfo) != 0) {
        fprintf(stderr, "ERROR, no such host\n");
        WSACleanup();
        exit(1);
    }


    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == INVALID_SOCKET) {
            continue;
        }

        if (connect(sockfd, p->ai_addr, (int)p->ai_addrlen) == SOCKET_ERROR) {
            closesocket(sockfd);
            continue;
        }

        break; 
    }

    if (p == NULL) {
        error("ERROR connecting");
    }

    freeaddrinfo(servinfo); 

    printf("Please enter the message: ");
    memset(buffer, 0, 256);
    fgets(buffer, 255, stdin);

    n = send(sockfd, buffer, strlen(buffer), 0);
    if (n == SOCKET_ERROR) {
        error("ERROR writing to socket");
    }

    memset(buffer, 0, 256);
    n = recv(sockfd, buffer, 255, 0);
    if (n == SOCKET_ERROR) {
        error("ERROR reading from socket");
    }

    printf("Server reply: %s\n", buffer);

    closesocket(sockfd);
    WSACleanup();

    return 0;
}
