#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define SIZEOFBUFF 100

void sendArgumento(int fdFifoMain,int argc, char** argv, char* fifoClient){
    int totalSize = 0;
    char strPid[20];
    char strArgc[20];


    char* concateneted = malloc(sizeof(char)*150);

    sprintf(strArgc,"%d",argc-3);

    strcat(concateneted,fifoClient);                              // adiciona nome do fifo no qual vai receber respostas para enviar
    strcat(concateneted," ");
    totalSize += strlen(fifoClient) + 1;

    strcat(concateneted,strArgc);                               // adiciona nr de argumentos para enviar
    strcat(concateneted," ");
    totalSize += strlen(strArgc) + 1;


    for(int i=1 ; i<argc; i++){                                 // adicionar todos os comandos do argv para enviar
        strcat(concateneted,argv[i]);
        strcat(concateneted," ");
        totalSize += strlen(argv[i])+1;
    }

    write(fdFifoMain,concateneted,totalSize);                   // envia string concatenada para fifo
}


void receiveAnswers(fifoClient){
    int fdFifo = open(fifoClient, O_RDONLY);
    int fdFifoWR = open(fifoClient, O_WRONLY);

    while((readBytes = read(fdFifo,buff,SIZEOFBUFF)) > 0){
        if(readBytes > 0){
            
        }
    }
}


int main(int argc, char** argv){

    int fdFifoMain = open("FifoMain",O_WRONLY);
    char buff[SIZEOFBUFF];
    int readBytes;
    char fifoClient[20];

    sprintf(fifoClient, "%d", pid);
    mkfifo(fifoClient,0666);

    sendArgumento(fdFifoMain, argc, argv, fifoClient);

    receiveAnswers(fifoClient);
}