#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>

#define NROFTRANSF 7
#define ARGVMAXSIZE 300

char* transformationsReps = "./transformationsReps/Reps";
char* transformationsFolder = "./transformations/";

char* transformationsFile[NROFTRANSF];
int reps[NROFTRANSF];




int readTransformationsReps(char* filepath){                    //Lê o ficheiro de reps de transformacoes

    int fd = open(filepath,O_RDONLY);
    int bufferSize = 1024;
    char* buffer = malloc(sizeof(char) * bufferSize);
    char* token;
    bool isTransformation = true;
    int ind = 0;

    read(fd,buffer,bufferSize);

    token = strtok(buffer, " \n");
   
    while( token != NULL ) {
        if(isTransformation){
            transformationsFile[ind] = token;
            isTransformation = false;
        }
        else{
            reps[ind] = atoi(token);
            isTransformation = true;
            ind++;
        }
        token = strtok(NULL, " \n");
    }
}

int addFolderToTransformation(char* t, char* transformation){
    strcpy(transformation,transformationsFolder);
    strcat(transformation,t);
}

int processFile(int argc, char** argv){
    
    int fdinput = open(argv[2],O_RDONLY);
    int fdoutput = open(argv[3],O_WRONLY | O_TRUNC | O_CREAT, 0666);


    for(int i=4 ; i<argc ; i++){

        if( fork() == 0){
            char* t = strdup(argv[i]);

            char* transformation = malloc(sizeof(char)*(strlen(t)+strlen(transformationsFolder))) ;
            addFolderToTransformation(t,transformation);
            free(t);

            dup2(fdinput,0);
            dup2(fdoutput,1);

            execl(transformation,transformation,NULL);
        }
    }

}

int showState(){

}


int main(int argc, char** argv){

    readTransformationsReps(transformationsReps);

    if( strcmp(argv[1],"proc-file") == 0) processFile(argc,argv);                   //se for para processar ficheiro
    else if( strcmp(argv[1],"status") == 0) showState();                            //se for para aceder a estado
        else printf("Comando inserido não existe");

}