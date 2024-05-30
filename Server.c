#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

void error(const char *msg) {
    fprintf(stderr, "%s: %d\n", msg, WSAGetLastError());
    exit(1);
}

int main() {
    WSADATA wsa;
    SOCKET sockfd, newsockfd;
    struct sockaddr_in serv_addr, cli_addr;
    int clilen;
    char buffer[256];
    int n;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        error("WSAStartup failed");
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        error("ERROR opening socket");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(8080);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        error("ERROR on binding");
    }

    if (listen(sockfd, 5) == SOCKET_ERROR) {
        error("ERROR on listen");
    }

    printf("Server listening on port 8080...\n");

    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd == INVALID_SOCKET) {
        error("ERROR on accept");
    }

    memset(buffer, 0, 256);
    n = recv(newsockfd, buffer, 255, 0);
    if (n == SOCKET_ERROR) {
        error("ERROR reading from socket");
    }

    printf("Here is the message: %s\n", buffer);

    n = send(newsockfd, "I got your message", 18, 0);
    if (n == SOCKET_ERROR) {
        error("ERROR writing to socket");
    }

    closesocket(newsockfd);
    closesocket(sockfd);
    WSACleanup();

    return 0;
}
