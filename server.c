#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>

// the port client will be connecting to
#define PORT 3996
#define MYPORT PORT
#define MAXDATASIZE 300

/* how many pending connections queue will hold */
#define BACKLOG 10
int accept_lines[BACKLOG];
char buffer_global[MAXDATASIZE];
int count = 0;


void crunch_up_from_index(int i){
        for (int j = i; j < count; j++)
                accept_lines[j] = accept_lines[j+1];
}


void remove_ll(int fd)
{
   for(int i = 0; i < count; i++)
        if(accept_lines[i] == fd){
          crunch_up_from_index(i);
          count--;
        }
}

int read_from_client(int filedes){
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
                strcpy(buffer_global,buffer);
                if(strcmp(buffer, "Leave") == 0){
                    remove_ll(filedes);
                    return -1;
                }
                fprintf(stderr, "Server: got message: %s\n",buffer);
                return 0;
        }
}

int make_socket(uint16_t port){
        /* listen on sock_fd, new connection on new_fd */
        int sockfd, new_fd;
        /* my address information, address where I run this program */
        struct sockaddr_in my_addr;

        sockfd = socket(AF_INET,SOCK_STREAM,0);
        if (sockfd == -1){
                perror("socket() error lol!");
                exit(1);
        } else
                printf("socket() is OK...\n");

        /* host byte order */
        my_addr.sin_family = AF_INET;
        /* short, network byte order */
        my_addr.sin_port = htons(port);
        /* auto-fill with my IP */
        my_addr.sin_addr.s_addr = INADDR_ANY;

        /* zero the rest of the struct */
        memset(&(my_addr.sin_zero), 0, 8);

        if(bind(sockfd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr))==-1){
                perror("bind() error lol!");
                exit(1);
        } else
                printf("bind() is OK...\n");
        return sockfd;
}

int write_to_client (int filedes, char* buf, int nbytes)
{
        return write(filedes, buf, nbytes);
}

int main(){
        int sock;
        int i;
        /* remote address information */
        struct sockaddr_in their_addr;
        int sin_size;
        count = 0;
        fd_set active_fd_set, read_fd_set;

        sock = make_socket(MYPORT);
        if (listen(sock, BACKLOG) == -1){
                perror("listen() error lol!");
                exit(1);
        } else
                printf("listen() is OK...\n");

        /* ...other codes to read the received data... */

        FD_ZERO(&active_fd_set);
        FD_SET (sock, &active_fd_set);

        while(1){
                /* Block until arrives on one or more active sockets */
                read_fd_set = active_fd_set;
                if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0){
                        perror("select");
                        exit(1);
                }
                /* Service all the sockets with input pending */
                for (i = 0; i < FD_SETSIZE; ++i){
                        if (FD_ISSET(i, &read_fd_set)){
                                printf("%d\n",i);
                                if (i == sock){
                                        printf("i==sock\n");
                                        /* Connection request on original socket */
                                        int new;
                                        sin_size = sizeof(their_addr);
                                        new = accept(sock, (struct sockaddr*)&their_addr,(socklen_t*)&sin_size);
                                        if (new < 0){
                                                perror("accept");
                                                exit(1);
                                        }
                                        accept_lines[count++] = new;
                                        /*
                                        fprintf(stderr, 
                                                "Server: connect from host %s, port %hd.\n",
                                                inet_ntoa(their_addr.sin_addr),
                                                ntohs(their_addr.sin_port));
                                        */
                                        FD_SET(new, &active_fd_set);
                                        char buf[8] = "Welcome ";
                                        write(new, buf, 8);
                                } else {
                                        /* Data arriving on an already-connected socket */
                                        /*int read_size;
                                        char client_message[256];
                                        while(1){
                                                read_from_client(i);
                                        }*/
                                        if (read_from_client (i) < 0)
                                        {
                                                printf("ddd");
                                                close (i);
                                                FD_CLR (i, &active_fd_set);
                                        } else {
                                        for (int k = 0; k < count; k++){
                                                if(accept_lines[k]!=i){
                                                        write_to_client(accept_lines[k],buffer_global,strlen(buffer_global));
                                                        printf("count: %d", count);
                                        }}}
                                }
                        }
                }
        }

        //close(sock);

        return 0;
}