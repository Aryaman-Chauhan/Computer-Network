#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#define BUFLEN 512  //Max length of buffer
#define PORT 8888  

void die(char* s){
    perror(s);
    exit(1);
}

int main(void){
    int sockfd = 0;
    int bytesReceived =0;
    char recvBuff[BUFLEN];
    unsigned char buff_offset[10]; //buffer to send the File Offset Value
    unsigned char buff_command[2]; //buffer to send the Complete File(0) or Partial FIle Command(1)
    int offset; //required to get the user input for offset in case of partial file command
    memset(recvBuff,0,sizeof(recvBuff));
    /*Create a socket first*/
    struct sockaddr_in si_other;
    int i, slen=sizeof(si_other);
    if ((sockfd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
    
    memset(&si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
    si_other.sin_addr.s_addr = inet_addr("127.0.0.1");
    slen = sizeof(si_other);
    if(connect(sockfd, (struct sockaddr *)&si_other, slen) < 0)
        die("connect() failed");
    /*Create file where data will be stored*/
    FILE *fp;
    int command;
    printf("Enter (0) to get complete file, (1) to specify offset, (2) calculate the offset value from local file\n");
    scanf("%d", &command);
    sprintf(buff_command, "%d", command);
    int temp = sendto(sockfd, buff_command, sizeof(buff_command), 0, (struct sockaddr *)&si_other, slen);
    if(temp == 0){
        die("sendto(command) failed");
    }
    if(command == 1 || command == 2){ // We need to specify the offset
        if(command == 1){ // get the offset from the user
            printf("Enter the value of File offset\n");
            scanf("%d", &offset);
        }
        else{
            fp = fopen("destination_file.txt","ab");
            if(NULL==fp){
                die("fopen()");
            }
            fseek(fp,0,SEEK_END);
            offset=ftell(fp);
            fclose(fp);
            printf("Offset Calculated: %d\n",offset);
        }
        // otherwise offset = size of local partial file, that we have already calculated
        sprintf(buff_offset, "%d", offset);
        /* sending the value of file offset */
        sendto(sockfd, buff_offset, sizeof(buff_offset), 0, (struct sockaddr *)&si_other, slen);
        fp = fopen("destination_file.txt","ab");
    }
    else{
        sprintf(buff_offset, "%d", 0);
        //Handling command 0 on the server side
        // write(sockfd, buff_offset, 10);
        fp = fopen("destination_file.txt","w");
    }
    
    if(NULL == fp){
        printf("Error opening file");
        return 1;
    }
    /* Receive data in chunks of 256 bytes */
    while((bytesReceived = recvfrom(sockfd, recvBuff, sizeof(recvBuff), 0, (struct sockaddr*)&si_other, &slen)) > 0){
        printf("Bytes received %d\n",bytesReceived);
        // recvBuff[n] = 0;
        printf("%s", recvBuff);
        fprintf(fp, "%s", recvBuff); // printf("%s \n", recvBuff);
        memset(recvBuff, 0, sizeof(recvBuff));
    }
    if(bytesReceived < 0){
        printf("\n Read Error \n");
    }
    fclose(fp);
    close(sockfd);
    exit(0);
    return 0;
}