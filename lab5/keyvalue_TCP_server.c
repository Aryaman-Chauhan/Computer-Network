#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

//Error print function//
void error(char *msg)
{
    perror(msg);
    exit(1);
}

void getString(int pos, int len, int c, char string[])
{

    char substring[1000];
    while (c < len) {
        substring[c] = string[pos + c - 1];
        c++;
    }
    substring[c] = '\0';
    // Print the result
    memset(string, 0, strlen(string));
    strncpy(string, substring, len);
}

int read_and_search_token(char *filename, char *token) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("fopen");
        return -1;
    }

    char line[1024];
    int line_number = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        line_number++;
        line[strcspn(line, "\n")] = '\0';
        line[strcspn(line, "\r")] = '\0';
        if (strncmp(line, token, strlen(token)) == 0) {
        fclose(file);
        return line_number - 1;
        }
    }

    fclose(file);
    return -1;  // Token not found
}

void add_tokens(char *filename, char *key, char *value) {
    FILE *file = fopen(filename, "a");
    if (file == NULL) {
        perror("fopen");
        return;
    }
    if(read_and_search_token(filename, key) != -1){//Already found
        fclose(file);
        return;
    }
    fprintf(file, "%s@%s\n", key, value);
    fflush(file);
    fclose(file);
}

void get_token(char *filename, int line_number, char *line) {
    if (line_number < 0) {
        memset(line, 0, 1024); // Handle invalid negative line numbers
        return;
    }

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("fopen");
        memset(line, 0, 1024);
        return;
    }

    int current_line = 0;
    while (fgets(line, 1024, file) != NULL) {
        if (current_line == line_number) {
        line[strcspn(line, "\n")] = '\0';
        line[strcspn(line, "\r")] = '\0';
        fclose(file);
        return;
        }
        current_line++;
    }

    fclose(file);
    memset(line, 0, 1024); // Token not found or end of file reached
}

int delete_token(char* filename, char* token){
    int ln;
    if((ln=read_and_search_token(filename, token)) == -1) return -1;
    FILE* fp = fopen(filename, "r");
    FILE* tp = fopen("temp", "w");
    int curr = 0;
    char buf[1024];
    while(fgets(buf, 1024, fp)!=NULL){
        if(curr++ == ln) continue;
        fputs(buf, tp);
    }
    fclose(fp);
    remove(filename);
    fclose(tp);
    rename("temp", filename);
    return 0;
}

//Main function//
int main(int argc, char *argv[])
{
    //variables declaration
    int sockfd, newsockfd, portno, clilen;
    char socket_buffer[256];
    char value[256],key[256], c;
    struct sockaddr_in serv_addr, cli_addr;
    int n = 0,rm;
    char put_buffer[256];
    time_t mytime;
    char get_buffer[256];

    if (argc < 2)
    {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    //socket creation and restoring communication//
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        error("ERROR opening socket");
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    {
        error("ERROR on binding");
    }
    printf("Server is listening");
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);

    if (newsockfd < 0)
    {
        error("ERROR on accept");
    }

      //Time as in Epoch//
    mytime = time(NULL);

    while(1){
	
	//Reading from buffer by clearing the buffer//
        bzero(socket_buffer,256);
        n = read(newsockfd,socket_buffer,255);

        //printing the current time//
        printf("Service requested at: %s\n",ctime(&mytime));

        //Error printing//
        if (n <= 0)
        {
            error("ERROR reading from socket");
        }

        //clearing key and value buffers//
        bzero(key,256);
        bzero(value,256);

        //initialization of variable to use in a loop//
        int i,j=0,k,l=0;

        //converting socket_buffer character to integer to get operation//
        k = socket_buffer[0] - '0';

        //getting key from the buffer: index starts from 2nd position and ends with ';'. First position has option. From kvsput in clientTCP.c//
        for(i=2;i<strlen(socket_buffer);i++){
            if(socket_buffer[i] == ';'){
                break;
            }
            key[j] = socket_buffer[i];
            j++;
        }

        //getting value from the buffer in a similar way//
        i=i+1;
        for(;i<strlen(socket_buffer);i++){
            if(socket_buffer[i] == '\0'){
                break;
            }
            value[l] = socket_buffer[i];
            l++;
        }

        //concatinating strings of directory and keyname into a file name directory//
        int q;

        //Switch starts here//
        switch(k){
            case 1:
                
                //PUT operation//
                add_tokens("database.txt", key, value);
                bzero(put_buffer, 256);
                sprintf(put_buffer,"Ack");
                q = write(newsockfd,put_buffer,strlen(put_buffer));
                if (q < 0)
                {
                    error("ERROR writing to socket");
                }
                break;

            case 2:

                //GET operation//
                int ln;
                if ((ln=read_and_search_token("database.txt", key)) == -1){
                    printf("File not found\n");
                    char *r = "FILE NOT FOUND";
                    write(newsockfd, r, strlen(r));
                    break;
                }
                bzero(put_buffer, 256);
                get_token("database.txt", ln, put_buffer);
                ln = strcspn(put_buffer, "@");
                printf("%s:%d", put_buffer, ln);
                int end = strcspn(put_buffer, "\n");
                getString(ln+2, end-ln-1, 0, put_buffer);
                q = write(newsockfd,put_buffer,strlen(put_buffer));
                if (q < 0)
                {
                    error("ERROR writing to socket");
                }
                break;

            case 3:
            
                //DELETE operation//
                bzero(get_buffer, 255);
                rm = delete_token("database.txt", key);
                if(rm==0){
                    strcpy(get_buffer, "FILE IS DELETED SUCCESSFULLY");
                }
                else{
                    printf("File not deleted successfully\n");
                    printf("File not found\n");
                    strcpy(get_buffer, "FILE NOT FOUND AND IS NOT DELETED");
                }
                q=  write(newsockfd, get_buffer, strlen(get_buffer));

                if (q < 0)
                {
                    error("ERROR writing to socket");
                }
                printf("%s\n",get_buffer);
                break;
        }
    }

    return 0;
}