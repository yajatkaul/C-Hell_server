#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

void handle_client(SOCKET client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

    if (bytes_read == SOCKET_ERROR) {
        fprintf(stderr, "recv failed. Error Code : %d\n", WSAGetLastError());
        closesocket(client_socket);
        return;
    }

    buffer[bytes_read] = '\0';
    printf("Received request:\n%s\n", buffer);

    FILE *file = fopen("index.html", "r");
    if (file == NULL) {
        const char *not_found_response =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<!DOCTYPE html>"
            "<html>"
            "<head><title>404 Not Found</title></head>"
            "<body><h1>404 Not Found</h1></body>"
            "</html>";

        send(client_socket, not_found_response, strlen(not_found_response), 0);
        closesocket(client_socket);
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *file_buffer = (char *)malloc(file_size + 1);
    if (file_buffer == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        closesocket(client_socket);
        return;
    }

    fread(file_buffer, 1, file_size, file);
    file_buffer[file_size] = '\0';
    fclose(file);

    const char *header =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
        "\r\n";

    send(client_socket, header, strlen(header), 0);
    send(client_socket, file_buffer, file_size, 0);

    free(file_buffer);
    closesocket(client_socket);
}

int main() {
    WSADATA wsaData;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int client_addr_len = sizeof(client_addr);

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed. Error Code : %d\n", WSAGetLastError());
        return 1;
    }

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        fprintf(stderr, "Could not create socket. Error Code : %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Prepare the sockaddr_in structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        fprintf(stderr, "Bind failed. Error Code : %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    // Listen to incoming connections
    if (listen(server_socket, 3) == SOCKET_ERROR) {
        fprintf(stderr, "Listen failed. Error Code : %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    printf("Server is listening on port %d\n", PORT);

    // Accept and incoming connection
    while ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len)) != INVALID_SOCKET) {
        printf("Connection accepted from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        handle_client(client_socket);
    }

    if (client_socket == INVALID_SOCKET) {
        fprintf(stderr, "Accept failed. Error Code : %d\n", WSAGetLastError());
    }

    // Clean up
    closesocket(server_socket);
    WSACleanup();

    return 0;
}
