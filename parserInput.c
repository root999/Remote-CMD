#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void parseInput(char buf[100], char cmd[50],char cmd_arg[50],int length){
    int i = 0;
    int  j=0;
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
      while(buf[i] != ' ' && buf[i] != '\n' && buf[i] != '\t'){
          cmd_arg[j] = buf[i];
          i++;
          j++;
      }
      cmd_arg[j] = '\0';

    }
printf("burda misin %s :::%s\n",cmd,cmd_arg);
}
int main(){
    char buf[100],cmd[50],cmd_arg[50];

    strcpy(buf,"input deneme");
    parseInput(buf,cmd,cmd_arg,strlen(buf));
    printf("%s %s",cmd,cmd_arg);



    return 0;


}


