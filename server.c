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
#define DATASIZE 100
#define MAXDATASIZE 1000
#define HASH_OUTPUT_SIZE 40
#define RANDOM_STRING_SIZE 10
#define USER_PASSWORD_SIZE 10
#define HOSTNAME_SIZE 25
#define HASH_INPUT_SIZE 20
#define DEFAULT_PORT 4123
#define NUM_OF_ARG 2
#define COMMAND_BUF_SIZE 50

int arg_number = 0;
int portEntered = 0;
static char args_doc[] = "Enter port value, username and password";
static struct argp_option options[] = {
   {
      "port",
      's',
      "socket",
      OPTION_ARG_OPTIONAL,
      "Defines port number.Default:4123"
   },

   {
      "user",
      'u',
      "user",
      0,
      "Defines authorized user."
   },

   {
      "pw",
      'p',
      "pass",
      0,
      "Defines password of the user."
   },

   {
      0
   }
};

/* Used by main to communicate with parse_opt. */
struct arguments {
   char * args[NUM_OF_ARG]; 
   int port;
   char username[USER_PASSWORD_SIZE], password[USER_PASSWORD_SIZE];
};

struct usr_info {
   char user_name[USER_PASSWORD_SIZE];
   char password[USER_PASSWORD_SIZE];
};

unsigned long djb2(char * str) { //hashing algorithm for random string
   unsigned long hash = 5381;
   int c;
   while (c = * str++)
      hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

   return hash;
}

static char * rand_string(char * str, size_t size) {
   srand(time(0));
   const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK";
   if (size) {
      --size;
      for (size_t n = 0; n < size; n++) {
         int key = rand() % (int)(sizeof charset - 1);
         str[n] = charset[key];
      }
      str[size] = '\0';
   }
   return str;
}

int conn_handl(int connsck) {
   char buf[DATASIZE];
   char output[MAXDATASIZE];
   char cmd[COMMAND_BUF_SIZE];
   char cmd_arg[COMMAND_BUF_SIZE];
   int numbytes;
   pid_t child_pid;
   FILE * fp;
   //dup2(connsck, 1);         // I tried to duplicate stdin and stdout to socket
   //dup2(connsck, 0);         // so I could have get them very easily (scanf and printf would be enough) 
   // close(connsck);           // but I couldn't handle the synchronization between
                                // server and client. I don't know the reason actually.
                                // maybe I'll try to do it later.
   while (1) {
      if ((numbytes = recv(connsck, buf, DATASIZE - 1, 0)) == -1) {
         perror("recv");
         exit(1);
      }
      buf[numbytes] = '\0';
      if (strcmp(buf, "disconnect") == 0) {
         close(connsck);
      } 
      else {
         child_pid = fork();
         if (child_pid < 0) {
            perror("Error forking");
            exit(1);
         }
         if (child_pid) {
                            //parent side
            waitpid(child_pid, NULL, 0);
         } 
         else {
            fp = popen(buf, "r");  // used for creating pip from shell to file.
                                    // fp then used for sending output of executed command to socket
            while (fgets(output, MAXDATASIZE, fp) != NULL) {
               if (send(connsck, output, strlen(output), 0) == -1) {
                  perror("send error");
                  close(connsck);
               }
            }
            if (send(connsck, "disconnect", 10, 0) == -1) {    //used for stopping client's recv() function
               perror("send error");
               close(connsck);
            }
         }
      }
   }
   close(connsck);
   exit(0);

}
/* Parse a single option. */
static error_t
parse_opt(int key, char * arg, struct argp_state * state) {
   /* Get the input argument from argp_parse, which we
      know is a pointer to our arguments structure. */
   struct arguments * arguments = state -> input;
   switch (key) {
   case 's':
      arguments -> port = atoi(arg);
      portEntered = 1;
      arg_number++;
      break;
   case 'u':
      strcpy(arguments -> username, arg);
      arg_number++;
      break;
   case 'p':
      strcpy(arguments -> password, arg);
      arg_number++;
      break;

   case ARGP_KEY_ARG:
      if (arg_number >= NUM_OF_ARG) {
         argp_usage(state);
         arguments -> args[state -> arg_num] = arg;
      }
      /* Too many arguments.*/
      break;

   case ARGP_KEY_END:
      if(portEntered == 0 && arg_number <2){
            argp_usage(state);
      }
      else if ((arg_number <= 2 && portEntered == 1)) {
         argp_usage(state);
      }
      /* Not enough arguments. */

      break;

   default:
      return ARGP_ERR_UNKNOWN;
   }
   return 0;
}

/* Our argp parser. */
static struct argp argp = {
   options,
   parse_opt,
   args_doc,
   0
};

int main(int argc, char ** argv) {
   int sockfd, new_fd;
   int portNumber;
   int numbytes;
   struct arguments arguments;
   char buf[MAXDATASIZE];
   char hash[HASH_OUTPUT_SIZE];
   unsigned long hashValue;
   char randSt[10], data4hash[20]; // Random string for authentication. Server sends random string.                
   struct usr_info granted_usr;     //Client encodes string with his/her password and returns encoded string.
   struct usr_info incoming_usr;    //Server decodes incoming string with user's password. 
   struct sockaddr_in server_addr;  // If it's the same random string at the beginning user gets granted.
   struct sockaddr_in client_addr;
   socklen_t sin_size = sizeof(struct sockaddr_in);;

   pid_t child_pid;

   argp_parse( & argp, argc, argv, 0, 0, & arguments);
   strcpy(granted_usr.user_name, arguments.username);
   strcpy(granted_usr.password, arguments.password);

   if (portEntered == 1) {
      portNumber = arguments.port;
   } else {
      portNumber = 4123; //Default port number
   }

   printf("\nUser:%s Password:%s granted. Port:%d\n", granted_usr.user_name, granted_usr.password, portNumber);
   printf("Use CTRL + C for stop server\n");
   fflush(stdout);

   if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { //Creating socket for connection
      perror("socket creation error");
      exit(1);
   }

   server_addr.sin_family = AF_INET; // setting up server's specifications
   server_addr.sin_port = htons(portNumber);
   server_addr.sin_addr.s_addr = INADDR_ANY;
   memset( & (server_addr.sin_zero), '\0', 8);

   if (bind(sockfd, (struct sockaddr * ) & server_addr, sizeof(struct sockaddr)) == -1) { //binding socket to a port
      perror("binding error");
      exit(1);
   }
   if (listen(sockfd, BACKLOG) == -1) { //start listening the port
      perror("listen error");
      exit(1);
   }
   while (1) {
      if ((new_fd = accept(sockfd, (struct sockaddr * ) & client_addr, & sin_size)) == -1) { // if client send request for connection
         perror("accept error"); // accept and route the connection to
         continue; // new socket
      }
      child_pid = fork(); //fork() the process for handling new connection

      if (child_pid < 0) {
         perror("Error forking");
         exit(1);
      }
      if (child_pid) { //parent side
         close(new_fd);
         waitpid(child_pid, NULL, 0);
      } else {
         strcpy(randSt, rand_string(randSt, 10)); // saving for sending random string to client.
         strcpy(data4hash, randSt); // copy rand string to another variable for concatenate random string with user's password. It'll be used for hashing
         strcat(data4hash, granted_usr.password);                               
         hashValue = djb2(data4hash); 
         snprintf(hash, sizeof(hash), "%ld", hashValue);  // unsigned long to char conversion
         if ((numbytes = recv(new_fd, buf, MAXDATASIZE - 1, 0)) == -1) {
            perror("recv errorr");
         }
         buf[numbytes] = '\0';
         strcpy(incoming_usr.user_name, buf);
         if (strcmp(incoming_usr.user_name, granted_usr.user_name)) {
            if (send(new_fd, "Wrong username.Try Again \n", 27, 0) == -1) {
               perror("send error");
               close(new_fd);
            }
            close(new_fd);
            exit(1);
         }
         if (send(new_fd, randSt, strlen(randSt), 0) == -1) {
            perror("send error");
            close(new_fd);
         }
         if ((numbytes = recv(new_fd, buf, MAXDATASIZE - 1, 0)) == -1) {
            perror("recv");
         }
         buf[numbytes] = '\0';
         if (strcmp(buf, hash)) {
            if (send(new_fd, "Wrong password.Try Again \n", 27, 0) == -1) {
               perror("send error");
               close(new_fd);
            }
            close(new_fd);
            exit(1);
         }
         conn_handl(new_fd); // handles connection
      }
      close(new_fd);
   }
   return 0;

}