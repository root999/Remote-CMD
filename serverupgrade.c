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
#include <argp.h>
#define BACKLOG 10

int arg_number =0;
static char args_doc[] = "Enter port value, username and password";
static struct argp_option options[] = {
  {"port",'s',"socket",OPTION_ARG_OPTIONAL,"Defines port number. Default:4123" },
  
  {"user",'u', "user",0,"Defines authorized user. Default:user" },
  
  {"pw",'p',"pass",0,"Defines password of the user. Default:pass" },
  
  { 0 }
};

/* Used by main to communicate with parse_opt. */
struct arguments
{
  char *args[3];                /* arg1 & arg2 */
  int port;
  char username[20], password[20];
};

/* Parse a single option. */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  struct arguments *arguments = state->input;
   
  switch (key)
    {
    case 's':
      //printf("in s: %s",arg);
      arguments->port = atoi(arg);
      arg_number++;
      break;
    case 'u':
    //printf("in u: %s",arg);
      strcpy(arguments->username,arg);
      arg_number++;
      break;
    case 'p':
    //printf("in p: %s",arg);
     strcpy(arguments->password,arg);
     arg_number++;
      break;

    case ARGP_KEY_ARG:
    printf("\n%d\n",arg_number);
      if (arg_number >= 4){
          argp_usage (state);
          arguments->args[state->arg_num] = arg;
      }
        /* Too many arguments.*/
      break;

    case ARGP_KEY_END:
      if (arg_number < 3){
          argp_usage (state);
      }
        /* Not enough arguments. */
        
      break;  

    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc,doc};

int main(int argc, char **argv){

    struct arguments arguments;
    //printf("first");
    /* Default values. */
    strcpy(arguments.username,"user");
    strcpy(arguments.password,"pass");
    arguments.port = 4123;
    argp_parse (&argp, argc, argv, 0, 0, &arguments);
    printf("\nu:%s pass: %s  port:%d",arguments.username,arguments.password,arguments.port);
    fflush(stdout);
    int sockfd, new_fd;
    int portN = arguments.port ;
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
            char cmd_args[50]; 
            char cmd[50];
            char cmd_arg[50];
            dup2(new_fd, 1); /* Duplicate socket's FD over FD #1, aka stdout */
            dup2(new_fd, 0); /* Duplicate socket's FD over FD #0, aka stdin */
            close(new_fd); /* Only closes the child's copy.  And the stdin/stdout copies remain. */
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
    char cmd_args[50]; 
    char cmd[50];
    char cmd_arg[50];
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