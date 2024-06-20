#include <stdlib.h>//exit(0)
#include <stdio.h>//printf
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#define BUFSIZE 32

int main(){
    //Create a socket
    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock < 0){
        printf("Error in opening socket\n");
        exit(0);
    }
    //Specify address for the socket
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    printf("Socket Created, assigning address\n");
    serverAddr.sin_family = AF_INET;
    // printf("Working 1");
    // fflush(stdout);
    serverAddr.sin_port = htons(9002);
    // printf("Working 2");
    // fflush(stdout);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    // printf("Working 3");
    // fflush(stdout);
    int status = connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    // printf("Working 4");
    // fflush(stdout);
    //Check for error in the connection
    if(status == -1){
        printf("Error Connecting to server\n\n");
        exit(0);
    }
    printf("Connection established\n");
    printf("You: ");

    //Recieve data from our server
    char buf[BUFSIZE];
    fgets(buf, BUFSIZE, stdin);
    // fscanf("%s\n", buf);
    int bytesSent = send(sock, buf, strlen(buf), 0);
    if(bytesSent != strlen(buf)){
        printf("Error sending message\n");
        exit(0);
    }
    printf("Data Sent\n");
    memset(buf, 0, BUFSIZE);
    int bytesRecv = recv(sock, &buf, sizeof(buf), 0);
    if(bytesRecv < 0){
        printf("Error recieving message\n");
        exit(0);
    }
    // Print the data received
    printf("Server:%s\n", buf);
    //Close the socket
    close(sock);

    return 0;
}