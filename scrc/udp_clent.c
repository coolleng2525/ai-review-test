#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // Set server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &(server_addr.sin_addr)) <= 0) {
        perror("Invalid server address");
        exit(EXIT_FAILURE);
    }

    // Send data to server
    strcpy(buffer, "Hello, server!");
    if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to send data to server");
        exit(EXIT_FAILURE);
    }

    printf("Data sent to server: %s\n", buffer);

    // Close socket
    close(sockfd);

    return 0;
}


void test_1(){
    char * a = malloc(10);
    return ;
}

void test_2(){
   int a;
   printf("%d", a);
}

void test_3(){
    free(NULL);
}
void test_4(){
    int a = 3;
    switch(a){
        case 1:
            break;
        case 2:
            break;
        case 3:
            ;
        default:
            break;
    }
}


void test_5(){
    int a = 3;
    if(a = 3){
        return;
    }
    return;
}

void test_6(){
    int a = 3;
    int b[3];
    for(int i = 0; i < 10; i++){
        printf("%d", b[i]);
    }
}


void test_7(){
    char a[10];
    char *p = a;
    memset(p, 'a', sizeof(p));
}

void test_8(){
   float a = 3.0;
   float b = 3.0;
   if (a == b){
       return;
   }
}

void test_9(){
    int fp = 0;
    fp = fopen("test.txt", "r");
    if (fp == NULL){
        return;
    }
    return;
}

void test_10(char *a){
    *a = 'a';
    return;
}

void test_11(){
    char a[10];
    a[10] = 'a';
    return;
}