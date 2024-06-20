#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
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
    printf("Client: %s", buf);
    int flag = 1;
    for(int i = 0; i < BUFSIZE; i++){
        if(buf[i] == '\0' || buf[i] == '\n')
            break;
        else if(buf[i] == '.' && flag == 1)
            flag = 2;
        else if(buf[i] == '.' && flag == 2){
            flag = 0;
            break;
        }
        else if(!isdigit(buf[i])){
            flag = 0;
            break;
        }
    }
    if(flag == 0){
        memset(buf, 0, BUFSIZE);
        sprintf(buf, "%s", "Please enter a Real Number.");
    }
    else{
        int x = (int)atof(buf);
        x++;
        memset(buf, 0, BUFSIZE);
        sprintf(buf, "%d", x);
    }
    
    printf("Server to Client: %s\n", buf);
    int bytesSent = send(clientSock, buf, strlen(buf), 0);
    if(bytesSent < 0){
        printf("Error in sending\n");
        exit(0);
    }
    close(clientSock);
    close(serverSock);
    exit(0);
}