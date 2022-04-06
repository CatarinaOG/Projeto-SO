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

char* getInputFileFormat(char* inputFileClean){
    char temp[ARGVMAXSIZE];
    temp[0] = '\0';
    strcat(temp, "<");
    strcat(temp, inputFileClean);
    strcat(temp, ">");

    strcpy(inputFileClean,temp);
}

char* getTransformationFormat(char* t, char* transformation){

    char temp[ARGVMAXSIZE];
    temp[0] = '\0';
    strcat(temp, transformationsFolder);
    strcat(temp, t);
    
    strcpy(transformation,temp);
}

int processFile(int argc, char** argv){
    
    char* inputFile = strdup(argv[2]);
    getInputFileFormat(inputFile);

    char* outputFile = argv[3];

    //for(int i=4 ; i<argc ; i++){
        char* t = strdup(argv[4]);
        char* transformation;
        getTransformationFormat(t,transformation);

        //printf("%s",transformations[i]);

        printf("(%s,%s,%s,%s\n",argv[4],argv[4],inputFile,outputFile);
        execlp(argv[4],argv[4],inputFile,outputFile,NULL);
        printf("hello");
        
    //}
    



}

int showState(){

}


int main(int argc, char** argv){

    readTransformationsReps(transformationsReps);

    if( strcmp(argv[1],"proc-file") == 0) processFile(argc,argv);                   //se for para processar ficheiro
    else if( strcmp(argv[1],"status") == 0) showState();                            //se for para aceder a estado
        else printf("Comando inserido não existe");

}