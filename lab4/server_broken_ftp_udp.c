#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#define BUFLEN 512  //Max length of buffer
#define PORT 8888   //The port on which to listen for incoming data

void die(char* s){
    perror(s);
    exit(1);
}

int main(void){
    struct sockaddr_in si_me, si_other;
    int s, i, slen = sizeof(si_other) , recv_len;
    char buf[BUFLEN];
    
    //create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
    
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
    
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    
    //bind socket to port
    if( bind(s, (struct sockaddr*)&si_me, sizeof(si_me)) == -1)
    {
        die("bind");
    }
    unsigned char offset_buffer[10];
    unsigned char command_buffer[2];
    memset(command_buffer, 0, sizeof(command_buffer));
    memset(offset_buffer, 0, sizeof(offset_buffer));
    int offset;
    int command;
    printf("Waiting for client to send the command (Full File (0) Partial File (1)\n");
    int byteRecv = recvfrom(s, command_buffer, sizeof(command_buffer), 0, (struct sockaddr *)&si_other, &slen);
    if(byteRecv < 0)
        die("recvfrom()");
    sscanf(command_buffer, "%d", &command);

    if(command == 0)
        offset = 0;
    else{
        printf("Waiting for client to send the offset\n");
        byteRecv = recvfrom(s, offset_buffer, sizeof(offset_buffer), 0, (struct sockaddr *)&si_other, &slen);
        if(byteRecv < 0)
            die("recvfrom 2");
        sscanf(offset_buffer, "%d", &offset);
    }

    /* Open the file that we wish to transfer */
    FILE *fp = fopen("source_file.txt","rb");
    if(fp==NULL){
        die("fopen");
    }
    /* Read data from file and send it */
    fseek(fp, offset, SEEK_SET);
    while(feof(fp) == 0){
        memset(buf,0,sizeof(buf));
        fgets(buf, BUFLEN, fp);
        printf("Bytes read: %ld\n", strlen(buf));
        /* If read was success, send data. */
        sendto(s, buf, strlen(buf), 0, (struct sockaddr *)&si_other, slen);
    }
    sendto(s, "", 0, 0, (struct sockaddr *)&si_other, slen);
    close(s);
    fclose(fp);
    exit(0);
    return 0;
}