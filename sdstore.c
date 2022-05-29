#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define SIZEOFBUFF 1024
#define COMMANDMAXSIZE 200
#define MAXSIZEINT 20

void sendRequest(int fdFifoMain,int argc, char** argv, char* pid){
    printf("pid:%s\n",pid);

    char nrCmds[MAXSIZEINT];

    char* concateneted = malloc(sizeof(char)*COMMANDMAXSIZE);

    sprintf(nrCmds,"%d",argc-5);

    strcat(concateneted,argv[1]);
    strcat(concateneted," ");

    strcat(concateneted,pid);                                 // adiciona pid para enviar
    strcat(concateneted," ");

    if(strcmp(argv[1], "proc-file") == 0){
        strcat(concateneted,nrCmds);                          // adiciona nr de argumentos para enviar
        strcat(concateneted," ");

        for(int i=2 ; i<argc; i++){                           // adicionar todos os comandos do argv para enviar
            strcat(concateneted,argv[i]);
            strcat(concateneted," ");
        }
    }

    printf("Send: <%s>\n",concateneted);

    write(fdFifoMain,concateneted,strlen(concateneted));                   // envia string concatenada para fifo
}

void sendFinished(int fdFifoMain, char* pidStr){

    char* concateneted = malloc(sizeof(char)*SIZEOFBUFF);

    strcat(concateneted,"finish ");
    strcat(concateneted,pidStr);
    strcat(concateneted," ");

    printf("Send:<%s>\n",concateneted);

    write(fdFifoMain,concateneted,strlen(concateneted));
}


int main(int argc, char** argv){

    int fdFifoMain = open("FifoMain",O_WRONLY);
    char* buff = malloc(sizeof(char)*SIZEOFBUFF);
    char pidStr[20];

    int pid = getpid();
    sprintf(pidStr, "%d", pid);

    mkfifo(pidStr,0666);

    sendRequest(fdFifoMain, argc, argv, pidStr);
    printf("Pending...\n");

    int fdFifo = open(pidStr,O_RDONLY);

    if(strcmp(argv[1], "proc-file") == 0){
        read(fdFifo,buff,SIZEOFBUFF);
        printf("Processing...\n");

        read(fdFifo,buff,SIZEOFBUFF);
        int bytesInput = atoi(strsep(&buff," "));
        int bytesOutput = atoi(strsep(&buff," "));
        printf("Concluded(bytes-input: %d, bytes-output: %d)\n",bytesInput,bytesOutput);

        sendFinished(fdFifoMain,pidStr);
    } else {
        int readBytes = 0;
        while((readBytes = read(fdFifo,buff,SIZEOFBUFF)) > 0){
            printf("%s",buff);
        }
    }

    close(fdFifo);
}