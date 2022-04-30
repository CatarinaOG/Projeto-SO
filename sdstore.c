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

void sendArgumento(int fdFifoMain,int argc, char** argv){
    int totalSize = 0;
    char strPid[20];
    char strArgc[20];


    char* concateneted = malloc(sizeof(char)*100);
    int pid = getpid();

    sprintf(strPid, "%d", pid);                                  // pid como string
    sprintf(strArgc,"%d",argc-3);

    strcat(concateneted,strPid);                                 // adiciona pid para enviar
    strcat(concateneted," ");
    totalSize += strlen(strPid) + 1;

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

int main(int argc, char** argv){

    int fdFifoMain = open("FifoMain",O_WRONLY);
    char buff[SIZEOFBUFF];
    int readBytes;

    sendArgumento(fdFifoMain,argc,argv);


}