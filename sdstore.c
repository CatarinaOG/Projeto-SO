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
    char* concateneted = malloc(sizeof(char)*100);
    int totalSize = 0;

    for(int i=1 ; i<argc; i++){
        strcat(concateneted,argv[i]);
        strcat(concateneted," ");
        totalSize += strlen(argv[i])+1;
    }

    write(fdFifoMain,concateneted,totalSize);

}

int main(int argc, char** argv){

    int fdFifoMain = open("FifoMain",O_WRONLY);
    char buff[SIZEOFBUFF];
    int readBytes;

    sendArgumento(fdFifoMain,argc,argv);


}