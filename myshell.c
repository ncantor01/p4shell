#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

//todo: WORK OUT REDIRECTION, Make sure redirection inputs are correct should be args[0:len-2]

int error(){
    char error_message[30] = "An error has occurred\n";
    write(STDOUT_FILENO, error_message, strlen(error_message));
    return(0);
}

void myPrint(char *msg)
{
    write(STDOUT_FILENO, msg, strlen(msg));
}

void** nextCom(char* input,int buffer){//output= list, first elem is argc, second is argv, third is line buffer, if line buffer = -1, next line
   void** ret = malloc(3 * sizeof(void*));
   int i = buffer;
   char* putter;
   int* argc1 = malloc(sizeof(int));
   argc1[0] = 0;
   int *next = malloc(sizeof(int));
   if(input[i] == ';' || input[i] == '\n'){;
       ret[0] = argc1;
       char** argv1 = malloc(sizeof(char*));
       argv1[0] = (char*)0;
       ret[1] = argv1;
       if(input[i] == ';'){next[0] = buffer+1;}
       else{next[0] = -1;}
       ret[2] = next;
       return ret;


   }
   while (input[i] != ';' && input[i] != '\n')
   { //get number of args in command
       i++;
       if (((input[i] == ' ') || (input[i] == ';') || (input[i] == '\n') || (input[i] == '>') || input[i] == '\t') && (input[i - 1] != ' ' && input[i - 1] != '\t'))
       {
           argc1[0]++;
       }
       if(input[i] == '>'){
           if (input[i + 1] == '+' && input[i] == '>'){
               i++;
           }
           i++;
           argc1[0]++;
        }
    }
   if(input[i] == ';'){next[0] = i+1;}
   else{next[0] = -1;}
   ret[2] = next;
   //printf("%d args\n",argc1);
   char** argv1 = malloc(sizeof(char*) * (argc1[0]+1));
   i = buffer;
   int j = 0;
   int* argpos = malloc(sizeof(int) * argc1[0]);
   if (input[i] != ' ' && input[i] != '\t' && input[i] != '\n' && input[i] != ';')
   {
       argpos[j] = 0;
       j++;
   }
    while (input[i+1] != ';' && input[i+1] != '\n'){ //argpos contains the position of the starts of the args
        if ((input[i] == ' ' || input[i] == '\t') && (input[i + 1] != ' ' && input[i + 1] != '\t'))
        {
            argpos[j] = i+1 - buffer;
            j++;
        }
        else if (input[i] != '>' && input[i+1] == '>')
        {
            argpos[j] = i+1 - buffer;
            j++;
        }
        else if (input[i] == '>'){
            if(input[i+1] == '+'){
                i++;
            }
            if (input[i + 1] != ' ' && input[i + 1] != '\t' && input[i + 1] != ';' && input[i + 1] != '\n')
            {
                argpos[j] = i+1 - buffer;
                j++;
            }
        }
        i++;
    }
    int k;
    for (j = 0; j < argc1[0]; j++)
    {
        i = argpos[j] + buffer;
        if(input[i] != '>'){
            while (input[i] != ' ' && input[i] != '\t' && input[i] != ';' && input[i] != '\n' && input[i] != '>')
            {
                i++;
            }
            argv1[j] = malloc(sizeof(char) * (i + 3 - argpos[j]));
            i = argpos[j] + buffer;
            k = 0;
            while (input[i] != ' ' && input[i] != '\t' && input[i] != ';' && input[i] != '\n' && input[i] != '>')
            {
                argv1[j][k] = input[i];
                i++;
                k++;
            }
            argv1[j][k] = '\0';
        }
        else{
            if(input[i+1] == '+'){
                putter = malloc(sizeof(char) * 3);
                putter[0] = '>';
                putter[1] = '+';
                putter[2] = '\0';
                argv1[j] = putter;
            }
            else{
                putter = malloc(sizeof(char) * 2);
                putter[0]= '>';
                putter[1]= '\0';
                argv1[j] = putter;
            }
        }
    }
    argv1[j] = (char*)0;
    ret[0] = argc1;
    ret[1] = argv1;
    ret[2] = next;
    return(ret);
}

int runCommand(char* input){//does one line of commands
    int* buff = malloc(sizeof(int));
    buff[0] = 0;
    int *argc1;
    char **args;
    void **com;
    char *file;
    char cwd[10000];
    int flag;
    int fd;
    void* readBuff;
    int status;
    char **args2;
    pid_t forkVar = 0;
    int i;
    while (buff[0] != -1)
    {
        com = nextCom(input, buff[0]);
        argc1 = com[0];
        buff = com[2];
        args = com[1];
        if(argc1[0] > 0){
        if (strcmp("exit", args[0]) == 0)
        {
            if(argc1[0] > 1){
                error();
            }
            else{
            exit(0);
            }
        }
        else if(strcmp("cd",args[0]) == 0){
            if(argc1[0] >= 3 || (argc1[0] >= 2 && '>' == args[1][0])){
                error();
            }
            else if (!args[1]) {
                char* root = getenv("HOME");
                chdir(root);
            } 
            else{
                flag = chdir(args[1]);
                if (flag < 0){
                    error();
                    }
            }
        }
        else if(strcmp("pwd",args[0]) == 0){
            if (argc1[0] >= 2 || (argc1[0] >= 2 && '>' == args[1][0])){
                error();
            }
            else{
                if (getcwd(cwd, sizeof(cwd)) != (char *)0)
                {
                    myPrint(cwd);
                    myPrint("\n");
                }
                else{
                    error();
                }
            }
        }
        else if(argc1[0] >= 3 && args[argc1[0] - 2][0] == '>'){
            flag = 0;
            for(i = 0; i < argc1[0]; i++){
                if (strcmp(">", args[i]) == 0 || strcmp(">+", args[i]) == 0)
                {
                    flag++;
                }
            }
            if (flag == 1 && strcmp(args[argc1[0] - 2], ">+") == 0){
                forkVar = fork();
                if (forkVar == 0){
                    file = args[argc1[0]-1] ;
                    flag = 0;
                    fd   =  open(file,O_RDONLY);
                    if (fd != -1){
                        readBuff = malloc(sizeof(char) * 1000);
                        read(fd,readBuff,1000);
                        flag = 1;
                    }
                    else{
                        fd = open(file,O_WRONLY | O_CREAT);
                    }
                    dup2(fd, STDOUT_FILENO);
                    args2 = malloc(sizeof(char *) * argc1[0] - 1);
                    for (i = 0; i < argc1[0] - 2; i++)
                    {
                        args2[i] = args[i];
                    }
                    args2[i] = (char *)0;
                    if (execvp(args2[0], args2) == -1)
                    {
                        error();
                        exit(0);
                    }
                    if(flag){
                        write(fd,readBuff,1000);
                    }
                    exit(0);
                }
                else{
                    waitpid(forkVar, &status, WCONTINUED);
                }

            }
            else if (flag == 1 && strcmp(args[argc1[0] - 2], ">") == 0)
            {
                forkVar = fork();
                if (forkVar == 0)
                {
                    file = args[argc1[0]-1];
                    fd = open(file, O_WRONLY | O_CREAT, 0644);
                    if (fd == -1)
                    {
                        error();
                        exit(0);
                    }
                    args2 = malloc(sizeof(char*) * argc1[0] - 1);
                    for(i = 0; i < argc1[0] - 2; i++){
                        args2[i] = args[i];
                    }
                    args2[i] = (char*)0;
                    dup2(fd, STDOUT_FILENO);
                    if (execvp(args2[0], args2) == -1)
                    {
                        error();
                    }
                    exit(0);
                    }
                else
                {
                waitpid(forkVar, &status, WCONTINUED);
                }
            }
            else{
                error();
            }
            }
            else{
                forkVar = fork();
                if(forkVar == 0){
                    if(execvp(args[0],args) == -1){
                        error();
                    }
                    exit(0);
                }
                else{
                    waitpid(forkVar,&status,WCONTINUED);
                }
            }
            
        }
    }
    return(0);
}



int main(int argc, char *argv[]) 
{
    char cmd_buff[514];
    char cmd_over[10000];
    char *pinput;
    int flag;
    int i;
    FILE* f;
    if(argc == 2){
        f = fopen(argv[1],"r");
        if(!f){
            error();
            exit(0);
        }
    }
    else if(argc != 1){
        error();
        exit(0);
    }
    while (1) {
        if (argc == 1){
        myPrint("myshell> ");
        }
        if(argc == 2){
            pinput = fgets(cmd_buff, 514, f);
        }
        else{
            pinput = fgets(cmd_buff, 514, stdin);
        }
        if (!pinput) {
            exit(0);
        }
        flag = 0;
        for(i = 0; i < 514; i++){
            if(pinput[i] == '\n'){
                flag = 1;
            }
        }
        if(flag < 1){
            myPrint(pinput);
            pinput = fgets(cmd_over, 100000,f);
            myPrint(pinput);
            error();
        }

        else{
            myPrint(cmd_buff);
            runCommand(cmd_buff);
            //int len = (int)strlen(cmd_buff);
            //void **xd = nextCom(cmd_buff,0);
            //int *len = xd[0];
            // int *buff = xd[2];
            //char **args = xd[1];
            // printf("%d\n",buff[0]);
            //printf("%s\n", args[0]);
            // printf("%s\n", args2[0]);
            //printf("%s\n", args[1]);
            //printf("%s\n", args[2]);
            // printf("%s\n", args[2]);
            //printf("%d\n", *len);
            //printf("%d\n", *buff);
        }
    }
}
