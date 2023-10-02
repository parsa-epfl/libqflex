#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 4234 // Port number to listen on
#define MAX_BUFFER_SIZE 1024 // Maximum buffer size for incoming data

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    char buffer[MAX_BUFFER_SIZE];
    FILE *file;

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("socket");
        return 1;
    }

    // Prepare server address structure
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Bind socket to address and port
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("bind");
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, 5) == -1) {
        perror("listen");
        return 1;
    }

    printf("Server listening on port %d...\n", PORT);

    // Accept a connection
    clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (clientSocket == -1) {
        perror("accept");
        return 1;
    }

    printf("Client connected.\n");

    // Open a file to save the incoming data
    file = fopen("received_data.txt", "w");
    if (file == NULL) {
        perror("fopen");
        return 1;
    }

    while (1) {
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            break;
        }
        fwrite(buffer, 1, bytesRead, file);
    }

    printf("Data received and saved to 'received_data.txt'.\n");

    // Close the file and sockets
    fclose(file);
    close(clientSocket);
    close(serverSocket);

    return 0;
}
