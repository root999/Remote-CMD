#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <unistd.h>
#define BACKLOG 10

int main(int argc, char const *argv[]){
    int sockfd, new_fd;
    int portN = 4123 ;
    char *cmd_args[] = {"ls", "-l", NULL};
    char *cmd;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t sin_size = sizeof(struct sockaddr_in);;
    pid_t child_pid;
    if((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1 ){   //Creating socket for connection
        perror("socket creation error");
        exit(1);
    }

    server_addr.sin_family = AF_INET;                       // setting up server's specifications
    server_addr.sin_port = htons(portN);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(server_addr.sin_zero),'\0',8);
    
    if(bind(sockfd,(struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1){         //binding socket to a port
        perror("binding error");
        exit(1);
    }
    if (listen(sockfd,BACKLOG) == -1 ){             //start listening the port
        perror("listen error");
        exit(1);
    }
    while (1)
    {
        if((new_fd = accept(sockfd,(struct sockaddr *)&client_addr,&sin_size))== -1){        // if client send request for connection
            perror("accept error");                                                             // accept and route the connection to
            continue;                                                                           // new socket
        }
        child_pid = fork();                 //fork() the process for handling new connection
        
        if( child_pid < 0 ){
            perror("Error forking"); 
            exit(1);
        } 
        if(child_pid){                      //parent side
            close(new_fd);
            //printf("buradayiz\n");
            waitpid(child_pid,NULL,0);
        }
        else {
            // auth client
            // start connection handler
           
        }
        //printf("connection acquired");
        
        //printf("server: got connection from %s\n",inet_ntoa(client_addr.sin_addr));

        /*if(send(new_fd,"Hello WORLD!\n",14,0) == -1)
            perror("send error");
        close(new_fd);*/
    }
return 0;

}

int conn_handl(int connsck){
    dup2(connsck, 1); /* Duplicate socket's FD over FD #1, aka stdout */
    dup2(connsck, 0); /* Duplicate socket's FD over FD #0, aka stdin */
    close(connsck); /* Only closes the child's copy.  And the stdin/stdout copies remain. */
    //execlp("/bin/echo", "HEY GUYS ALJ AF MY FACE IS A ROTTORN BANANA");
    fprintf(stdout,"denemedir bu \n");
    fflush(stdout);
    fscanf(stdin,"%s",cmd);
    fscanf(stdin,"%s",cmd_args);
    fscanf(stdin,"%s",cmd_arg);
    execlp(cmd,cmd_args,cmd_arg,NULL);
     //execvp( cmd, cmd_args );
    perror("the execvp(3) call failed.");
    exit(1);
}