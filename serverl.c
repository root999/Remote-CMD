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
#include <time.h>
#define BACKLOG 10
#define MAXDATASIZE 100
#define HASH_OUTPUT_SIZE 40




int arg_number =0;
static char args_doc[] = "Enter port value, username and password";
static struct argp_option options[] = {
  {"port",'s',"socket",OPTION_ARG_OPTIONAL,"Defines port number.Default:4123" },
  
  {"user",'u', "user",0,"Defines authorized user." },
  
  {"pw",'p',"pass",0,"Defines password of the user." },
  
  { 0 }
};

/* Used by main to communicate with parse_opt. */
struct arguments
{
  char *args[3];                /* arg1 & arg2 */
  int port,portEntered;
  char username[20], password[10];
};

struct usr_info
{
  char user_name[10];
  char password[10];
};


unsigned long djb2(char *str){            //hashing algorithm for random string
        unsigned long hash = 5381;
        int c;
        while (c = *str++)
            hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

        return hash;
    }

static char *rand_string(char *str, size_t size)
{
    srand (time(0));
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK";
    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
    return str;
}
void parseInput(char buf[100], char cmd[50],char cmd_arg[50],int length){
    int i = 0;
    int  j=0;
  printf("%s :::::: \n",buf);
  for(i=0;i<length;i++){
      if (length == 0)
        exit(0);            /* ^d was entered, end of user command stream */
    if (length < 0){
        perror("error reading the command");
      	exit(-1);           /* terminate with error code of -1 */
      }
      while(buf[i] != ' ' && buf[i] != '\n' && buf[i] != '\t' ){
        cmd[j]=buf[i];
        j++;
        i++;
      }
      cmd[j] = '\0';
      i++;
      j=0;
      if(buf[i] == ' ' || buf[i] == '\n' || buf[i] == '\t'){
        cmd_arg[j] = '\0';
            break;
      }
      while(buf[i] != ' ' && buf[i] != '\n' && buf[i] != '\t'){
          cmd_arg[j] = buf[i];
          i++;
          j++;
      }
      cmd_arg[j] = '\0';

    }
//printf("burda misin %s :::%s\n",cmd,cmd_arg);
}
int conn_handl(int connsck){ 
    char buf[MAXDATASIZE];
    char output[1000];
    char cmd[50];
    char cmd_arg[50];
    int numbytes;
    pid_t child_pid ;
    FILE *fp;
    //dup2(connsck, 1);         // I tried to duplicate stdin and stdout to sockets
    //dup2(connsck, 0);         // so I could have get them very easily (scanf and printf would be enough) 
    // close(connsck);           // but I couldn't handle the synchronization between
                                // server and client. I don't know the reason actually.
                                // maybe I'll try to do it later.
    while (1)
    {
      if((numbytes = recv(connsck,buf,MAXDATASIZE-1,0)) == -1){
                  perror("recv");
                  exit(1);
      }
      buf[numbytes] = '\0';
      
      if(strcmp(buf,"exit") == 0 ){
        printf("%s if ici \n",buf);
          close(connsck);
        }
      else{
          parseInput(buf,cmd,cmd_arg,numbytes);
          child_pid = fork();
          if( child_pid < 0 ){
              perror("Error forking"); 
              exit(1);
          } 
         if(child_pid){                      //parent side
              //printf("buradayiz\n");
              waitpid(child_pid,NULL,0);
          }
          else {
            execlp(cmd,cmd,cmd_arg,NULL);
            while (fgets(output, 1000, stdout) != NULL){
                if(send(connsck,output,strlen(output),0) == -1){
                    perror("send error");
                    close(connsck);
                }
            }
            if(send(connsck,"cgr",3,0) == -1){
                    perror("send error");
                    close(connsck);
                }

         // fclose(fp);
          //   snprintf (output, sizeof(execlp(cmd,cmd,cmd_arg,NULL)), "%s",execlp(cmd,cmd,cmd_arg,NULL)); 
          //   if(send(connsck,output,strlen(output),0) == -1){
          //     perror("send error");
          //     close(connsck);
          // }
          }
      }
    }
    close(connsck);
    exit(0);
  
    
}
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
      arguments->portEntered =1;
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
      if (arg_number < 3 && arguments->portEntered == 1){
          printf("burasi %d \n",arguments->portEntered);
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
static struct argp argp = { options, parse_opt, args_doc,0};

int main(int argc, char **argv){
    int sockfd, new_fd;
    int portNumber; 
    int numbytes;
    struct arguments arguments;
    char buf[MAXDATASIZE];
    char hash[HASH_OUTPUT_SIZE];
    unsigned long hashValue;
    char randSt[10],data4hash[20];                    // Random string for authentication. Server sends random string. 
                                            //Client encodes string with his/her password and returns encoded string.
                                            //Server decodes incoming string with user's password. 
                                            // If it's the same random string at the beginning user gets granted.
    struct usr_info granted_usr;
    struct usr_info incoming_usr;
    struct sockaddr_in server_addr; 
    struct sockaddr_in client_addr;
    socklen_t sin_size = sizeof(struct sockaddr_in);;
    
    pid_t child_pid;
    
    argp_parse (&argp, argc, argv, 0, 0, &arguments);
    strcpy(granted_usr.user_name,arguments.username);
    strcpy(granted_usr.password,arguments.password);

    if( arguments.portEntered == 1){
        portNumber = arguments.port;
    }
    else{
      portNumber = 4123;    //Default port number
    }
    
    printf("\nUser:%s Password:%s granted. Port:%d\n",granted_usr.user_name,granted_usr.password,portNumber);
    fflush(stdout);
    
    if((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1 ){   //Creating socket for connection
        perror("socket creation error");
        exit(1);
    }

    server_addr.sin_family = AF_INET;                       // setting up server's specifications
    server_addr.sin_port = htons(portNumber);
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
          strcpy(randSt,rand_string(randSt,10)); // saving for sending random string to client.
          strcpy(data4hash,randSt); // copy rand string to another variable for concatenate random string with user's password. It'll be used for hashing
          strcat(data4hash,granted_usr.password);                                                                                       
          //printf("rand value :%s\n",randSt);
          //printf("%s\n",data4hash);                                    
          hashValue = djb2(data4hash);
          //printf("hash value: %ld\n",hashValue);     
          snprintf (hash, sizeof(hash), "%ld",hashValue); 
          if((numbytes = recv(new_fd,buf,MAXDATASIZE-1,0)) == -1){
                  perror("recv errorr");
            }
          buf[numbytes] = '\0';
         // printf("Received username: %s\n ",buf);
          strcpy(incoming_usr.user_name,buf);
          if(strcmp(incoming_usr.user_name,granted_usr.user_name)){
              if(send(new_fd,"Wrong username.Try Again \n",27,0) == -1){
                    perror("send error");
                    close(new_fd);
              }
              close(new_fd);
              exit(1);
          }
          if(send(new_fd,randSt,strlen(randSt),0) == -1){
                    perror("send error");
                    close(new_fd);
          }
          if((numbytes = recv(new_fd,buf,MAXDATASIZE-1,0)) == -1){
                  perror("recv");
          }
          buf[numbytes] = '\0';
          //printf("Received hash: %s \n",buf);
          if(strcmp(buf,hash)){
            if(send(new_fd,"Wrong password.Try Again \n",27,0) == -1){
                    perror("send error");
                    close(new_fd);
              }
              close(new_fd);
              exit(1);
          }

          //printf("buradayiz\n");
          conn_handl(new_fd);  // handles connection
        }
        //printf("connection acquired");
        
        //

        close(new_fd);
    }
return 0;

}

