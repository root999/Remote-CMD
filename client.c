#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <argp.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/select.h>
#define MAXDATASIZE 100
#define RANDOM_STRING_SIZE 10
#define USER_PASSWORD_SIZE 10
#define HOSTNAME_SIZE 25
#define HASH_OUTPUT_SIZE 40
#define HASH_INPUT_SIZE 20
#define DEFAULT_PORT 4123
#define NUM_OF_ARG 4

/*
A client that has few bugs. Will be updated.
-Sometimes can't execute command quickly and asks for new command 


*/


int arg_number = 0;
static char args_doc[] = "Enter port value (using --port=val), username(using --user=) and password (using --pw=)";
static struct argp_option options[] = {

   {
      "hostn",
      'h',
      "host",
      OPTION_ARG_OPTIONAL,
      "Defines hostname.Default:localhost"
   },

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
   char hostname[HOSTNAME_SIZE];
   char * args[NUM_OF_ARG]; /* arg1 & arg2 */
   int port, portEntered, hostEntered;
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

/* Parse a single option. */
static error_t
parse_opt(int key, char * arg, struct argp_state * state) {
   /* Get the input argument from argp_parse, which we
      know is a pointer to our arguments structure. */
   struct arguments * arguments = state -> input;

   switch (key) {
   case 'h': 
      strcpy(arguments -> hostname, arg);
      arguments -> hostEntered = 1;
      arg_number++;
      break;
   case 's':
      arguments -> port = atoi(arg);
      arguments -> portEntered = 1;
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
      if (arg_number > NUM_OF_ARG) {
         argp_usage(state);
         arguments -> args[state -> arg_num] = arg;
      }
      /* Too many arguments.*/
      break;

   case ARGP_KEY_END:
      if (arg_number < NUM_OF_ARG && arguments -> portEntered == 1 && arguments -> hostEntered == 1) {
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

int main(int argc, char * argv[]) {
   int sockfd, numbytes, portNumber, on;
   char buf[MAXDATASIZE];
   char inputBuf[MAXDATASIZE];
   struct sockaddr_in server_addr;
   struct hostent * hoste;
   char hostname[HOSTNAME_SIZE];
   char randSt[USER_PASSWORD_SIZE];
   // char data4hash[HASH_INPUT_SIZE];
   char hash[HASH_OUTPUT_SIZE];
   unsigned long hashValue;
   on = 1;
   struct arguments arguments;
   struct usr_info user;
   pid_t child_pid;
   argp_parse( & argp, argc, argv, 0, 0, & arguments);
   strncpy(user.user_name, arguments.username, USER_PASSWORD_SIZE);
   strncpy(user.password, arguments.password, USER_PASSWORD_SIZE);

   if (arguments.portEntered == 1) {
      portNumber = arguments.port;
   } else {
      portNumber = DEFAULT_PORT; //Default port number
   }
   if (arguments.hostEntered == 1) {
      strncpy(hostname, arguments.hostname, HOSTNAME_SIZE);
   } else {
      strcpy(hostname, "localhost"); //default server address
   }
   printf("\nHost: %s User:%s Password:%s Port:%d\n", hostname, user.user_name, user.password, portNumber);
   printf("Trying to connect to server...\n");
   while (1) {
      if ((hoste = gethostbyname(hostname)) == NULL) {
         perror("gethosttbyname");
         exit(1);
      }

      if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
         perror("socket");
         exit(1);
      }

      server_addr.sin_family = AF_INET;
      server_addr.sin_port = htons(portNumber);
      server_addr.sin_addr = * ((struct in_addr * ) hoste -> h_addr);
      memset( & (server_addr.sin_zero), '\0', 8);

      if (connect(sockfd, (struct sockaddr * ) & server_addr, sizeof(struct sockaddr)) == -1) {
         perror("connect");
         exit(1);
      }
      if (send(sockfd, user.user_name, strlen(user.user_name), 0) == -1) {          //Sending username to start authentication
         perror("send error");
         close(sockfd);
      }

      if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1) {
         perror("recv errır");
         exit(1);
      }
      buf[numbytes] = '\0';
      printf("Received random string: %s\n", buf);
      strcpy(randSt, buf);
      strcat(randSt, user.password);            //random string (salt) added to user's password
      hashValue = djb2(randSt);                 // for hashing. Client will hash this string and send to server
                                                // Server did the same process and
      printf("Hash Value: %ld\n", hashValue);               // got a hash value. Two value will be compared and if they are same
      snprintf(hash, sizeof(hash), "%ld", hashValue);           // same, connection will allowed.
      if (send(sockfd, hash, strlen(hash), 0) == -1) {
         perror("send error");
         close(sockfd);
      }
      printf("Command line ready \n");
      printf("You can close it using CTRL + C\n");
      fflush(stdout);
      while (1) {
         fgets(inputBuf, MAXDATASIZE, stdin);

         if (send(sockfd, inputBuf, strlen(inputBuf), 0) == -1) {
            perror("send error");
            close(sockfd);
         }

         if ((numbytes = recv(sockfd, buf, 1000-1, 0)) == -1) {
            perror("recv");
            exit(1);
         }
         buf[numbytes] = '\0';

         while (strcmp(buf, "disconnect")) {
            if ((numbytes = read(sockfd, buf, 1000-1)) == -1) {
               perror("recv");
               exit(1);
            }
            buf[numbytes] = '\0';
            if(strcmp(buf, "disconnect")){
                printf("%s\n",buf);
            }
            
         }
         strcpy(buf,"");
         printf("Enter new command \n");
      }
      close(sockfd);
   }

   return 0;

}