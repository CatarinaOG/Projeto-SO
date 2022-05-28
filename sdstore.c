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

    sprintf(nrCmds,"%d",argc-4);

    strcat(concateneted,"proc-file ");

    strcat(concateneted,pid);                                 // adiciona pid para enviar
    strcat(concateneted," ");

    strcat(concateneted,nrCmds);                               // adiciona nr de argumentos para enviar
    strcat(concateneted," ");


    for(int i=2 ; i<argc; i++){                                 // adicionar todos os comandos do argv para enviar
        strcat(concateneted,argv[i]);
        strcat(concateneted," ");
    }

    printf("Send: %s\n",concateneted);

    write(fdFifoMain,concateneted,strlen(concateneted));                   // envia string concatenada para fifo


}

void sendFinished(int fdFifoMain, int argc, char** argv){

    char* concateneted = malloc(sizeof(char)*COMMANDMAXSIZE);

    strcat(concateneted,"finish ");

    char nrCmds[MAXSIZEINT];

    sprintf(nrCmds,"%d",argc-4);
    strcat(concateneted,nrCmds);
    strcat(concateneted," ");

    for(int i=0 ; i<argc-4; i++){                                 // adicionar todos os comandos do argv para enviar
        strcat(concateneted,argv[i+4]);
        strcat(concateneted," ");
    }

    printf("concat:|%s|\n",concateneted);

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

    int fdFifo = open(pidStr,O_RDONLY);

    read(fdFifo,buff,SIZEOFBUFF);
    printf("Processing...\n");

    read(fdFifo,buff,SIZEOFBUFF);
    int bytesInput = atoi(strsep(&buff," "));
    int bytesOutput = atoi(strsep(&buff," "));
    printf("Concluded(bytes-input: %d, bytes-output: %d)\n",bytesInput,bytesOutput);

    sendFinished(fdFifoMain,argc,argv);

}