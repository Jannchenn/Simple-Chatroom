#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

// the port client will be connecting to
#define PORT 3996
// max number of bytes we can get at once
#define MAXDATASIZE 300

int read_from_server(int filedes){
        char buffer[MAXDATASIZE];
        int nbytes;

        nbytes = read(filedes, buffer, MAXDATASIZE);
        if (nbytes < 0){
                /* Read error */
                perror("read");
                exit(EXIT_FAILURE);
        } else if (nbytes == 0)
                /* End-of-file. */
                return -1;
        else {
                /* Data read. */
                buffer[nbytes] = '\0';
                // printf("%s",buffer);
                if(strcmp(buffer,"Welcome ")==0){
                        return 1;
                } else {
                        printf("%s\n",buffer);
                        return 2;
                }
                return 0;
        }
}

int write_to_server (int filedes, char* buf, int nbytes)
{
        return write(filedes, buf, nbytes);
}

int main(int argc, char* argv[]){
        int sockfd,numbytes;
        char buf[MAXDATASIZE];
        struct hostent *he;
        /* connector's address information */
        struct sockaddr_in their_addr;

        /* if no command line argument supplied */
        if (argc != 2){
                fprintf(stderr, "Client-Usage: %s the_client_hostname\n", argv[0]);
                exit(1);
        }

        /* get the host info */
        if((he=gethostbyname(argv[1]))==NULL){
                perror("gethostbyname()");
                exit(1);
        } else
                printf("Client-The remote host is: %s\n", argv[1]);

        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
                perror("socket()");
                exit(1);
        } else
                printf("Client-The socket() sockfd is OK...\n");

        /* host byte order */
        their_addr.sin_family = AF_INET;
        /* short, network byte order */
        printf("Server-Using %s and port %d...\n", argv[1], PORT);
        their_addr.sin_port = htons(PORT);
        their_addr.sin_addr = *((struct in_addr *)he->h_addr);
        /* zero the rest of the struct */
        memset(&(their_addr.sin_zero), '\0', 8);

        if(connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1){
                perror("connect()");
                exit(1);
        } else
                printf("Client-The connect() is OK...\n");

        /* host byte order */
        their_addr.sin_family = AF_INET;
        /* short, network byte order */
        printf("Server-Using %s and port %d...\n", argv[1], PORT);
        their_addr.sin_port = htons(PORT);
        their_addr.sin_addr = *((struct in_addr *)he->h_addr);
        /* zero the rest of the struct */
        memset(&(their_addr.sin_zero), '\0', 8);

        if(connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1){
                perror("connect()");
                exit(1);
        } else
                printf("Client-The connect() is OK…\n");

        int i;
        char username[1024];
        char welcome[1024] = "ENTER ";
        int userbytes;
        printf("Username: ");
        gets(username);
        fd_set active_fd_set,read_fd_set;
        FD_ZERO(&active_fd_set);
        FD_ZERO(&read_fd_set);
        FD_SET (sockfd, &active_fd_set);
        FD_SET (0,&active_fd_set);
        while(1){
                read_fd_set = active_fd_set;
                if (select(sockfd+1,&read_fd_set, NULL, NULL, NULL)<0){
                        perror("select");
                        exit(1);
                }
                if (FD_ISSET(sockfd, &read_fd_set)){
                        //printf("FROM SERVER\n");
                        int num = read_from_server(sockfd);
                        if (num == 1){
                                strcat(welcome,username);
                                write_to_server(sockfd,welcome,strlen(welcome));
//                      else if(userbytes == 2)
//                      int welcome = read_from_server(sockfd);
                        } else {
                                continue;
                        }
                }
                if (FD_ISSET(0, &read_fd_set)){
                        char message[1024];
                        char say[1024] = "SAY ";
                        strcat(say,username);
                        strcat(say, " ");
                        gets(message);
                        if(strcmp(message, "Leave")==0){
                                write_to_server(sockfd,message, strlen(message));
                                char leave[1024] = " ";
                                strcat(leave, username);
                                write_to_server(sockfd,leave, strlen(leave));
                                close(sockfd);
                                exit(0);
                        }else{
                                strcat(say, message);
                                write_to_server(sockfd,say,strlen(say));
                        }
                }

        }
        /*if((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1){
                perror("recv()");
                exit(1);
        } else
                printf("Client-The recv() is OK...\n");
     
        buf[numbytes] = '\0';
        printf("Client-Received: %s", buf);
     */
        //printf("Client-Closing sockfd\n");
        //close(sockfd);
        return 0;
}