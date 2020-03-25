#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#define PORT 8080
#define MAXDATASIZE 100
int main(int argc, char *argv[]){
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct hostent *he;
    struct sockaddr_in server_addr;
   
    if ((sockfd = socket(AF_INET,SOCK_STREAM,0))== -1){
        perror("socket");
        exit(1);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(server_addr.sin_zero),'\0',8);

    
    if(connect(sockfd,(struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1){
        perror("connect");
        exit(1);
    }
    if((numbytes = recv(sockfd,buf,MAXDATASIZE-1,0)) == -1){
        perror("recv");
        exit(1);
    }
    buf[numbytes] = '\0';
    printf("Received: %s",buf);
    close(sockfd);
    print("deneme")
    return 0;
    
}