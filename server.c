#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#define BACKLOG 10
/*
ADIMLAR
-SOCKET OLUŞTUR
-SOCKET AYARLARINI YAP
-BIND
-LİSTEN


*/
int main(int argc, char const *argv[]){
    int sockfd, new_fd;
    int portN = 8080 ;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t sin_size;

    if((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1 ){
        perror("socket creation error");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portN);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(server_addr.sin_zero),'\0',8);
    
    if(bind(sockfd,(struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1){
        perror("binding error");
        exit(1);
    }
    if (listen(sockfd,BACKLOG) == -1 ){
        perror("listen error");
        exit(1);
    }
    while (1)
    {
        sin_size = sizeof(struct sockaddr_in);
        if((new_fd = accept(sockfd,(struct sockaddr *)&client_addr,&sin_size))== -1){
            perror("accept error");
            continue;
        }
        printf("server: got connection from %s\n",inet_ntoa(client_addr.sin_addr));

        if(send(new_fd,"Hello WORLD!\n",14,0) == -1)
            perror("send error");
        close(new_fd);
    }
return 0;

}