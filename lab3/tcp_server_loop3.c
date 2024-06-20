#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define MAXPENDING 3
#define BUFSIZE 32

int main(){
    char buf[BUFSIZE];
    //Create socket
    int serverSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(serverSock < 0){
        printf("Error creating server socket \n");
        exit(0);
    }
    printf("Server socket Created\n");
    //Define server address
    struct sockaddr_in serverAddr, clientAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9002);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("Server address Assigned\n");

    //Bind the socket to specified IP and port
    int temp = bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if(temp < 0){
        printf("Error binding socket\n");
        exit(0);
    }
    
    printf("Listening for Connections\n");
    temp = listen(serverSock, MAXPENDING);
    if(temp < 0){
        printf("Error listening\n");
        exit(0);
    }
    int clientLength = sizeof(clientAddr);
    int count = 1;
    while(count < 4){
        int clientSock = accept(serverSock, (struct sockaddr*)&clientAddr, &clientLength);
        if(clientLength < 0){
            printf("Error in Client Socket\n");
            exit(0);
        }
        printf("Handling Client: %s\n", inet_ntoa(clientAddr.sin_addr));
        memset(buf, 0, BUFSIZE);
        temp = recv(clientSock, buf, BUFSIZE, 0);
        if(temp < 0){
            printf("Error in receiving\n");
            exit(0);
        }
        printf("Client%d: %s\n", count, buf);
        printf("Server to Client%d: ", count);
        memset(buf, 0, BUFSIZE);
        fgets(buf, BUFSIZE, stdin);
        fflush(stdin);
        // fscanf("%s\n", buf);
        int bytesSent = send(clientSock, buf, strlen(buf), 0);
        if(bytesSent < 0){
            printf("Error in sending\n");
            exit(0);
        }
        close(clientSock);
        count++;
    }
    close(serverSock);
    return 0;
}