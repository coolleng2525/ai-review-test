#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 12345
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in serverAddr, clientAddr;
    char buffer[BUFFER_SIZE];

    // Create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    memset(&clientAddr, 0, sizeof(clientAddr));

    // Configure server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Bind socket to the specified address and port
    if (bind(sockfd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    printf("UDP server is listening on port %d...\n", PORT);

    while (1) {
        socklen_t len = sizeof(clientAddr);

        // Receive data from client
        ssize_t numBytes = recvfrom(sockfd, (char *)buffer, BUFFER_SIZE, 0, (struct sockaddr *)&clientAddr, &len);
        if (numBytes < 0) {
            perror("Error in receiving data");
            exit(EXIT_FAILURE);
        }

        buffer[numBytes] = '\0';
        printf("Received message from client: %s\n", buffer);

        // Process received data (add your logic here)

        // Send response back to client
        if (sendto(sockfd, (const char *)buffer, strlen(buffer), 0, (const struct sockaddr *)&clientAddr, len) < 0) {
            perror("Error in sending response");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}