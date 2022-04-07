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

    read(fd,buffer,bufferSize);                             //lê ficheiro

    token = strtok(buffer, " \n");                          //separa tudo or espaços e \n
   
    while( token != NULL ) {                                
        if(isTransformation){                               // se for transformação
            transformationsFile[ind] = token;               // guardar na lista de transformações
            isTransformation = false;                       // prox token é o nr de reps
        }
        else{                                               // se nao for transformação
            reps[ind] = atoi(token);                        // guardar nr de reps na lista de reps (msm indice que a correspondente transformação)
            isTransformation = true;                        // o prox token é transformação
            ind++;
        }
        token = strtok(NULL, " \n");                        // avança com o token
    }
}

int addFolderToTransformation(char* t, char* transformation){      //vai colocar em transformation o caminho correto para a transformacao t   
    strcpy(transformation,transformationsFolder);                  // Ex:  t=bcompress transformation = ./transformations/bcomrpess
    strcat(transformation,t);
}

int fillComands(char* cmds[],int argc,char** argv){                 // vai preencher cmds com todas as transformacoes
    
    for(int i=4 ; i<argc ; i++){
        cmds[i-4] = strdup(argv[i]);
    }

}

int processFile(int argc, char** argv){
    
    int fdinput = open(argv[2],O_RDONLY);                                      
    int fdoutput = open(argv[3],O_WRONLY | O_TRUNC | O_CREAT, 0666);

    int nrCmds = argc-4;                                            // nr de transformacoes
    char* cmds[nrCmds];                                             // lista de transformacoes
    int fildes[nrCmds][2];                                          // lista de fildes para cada uma das transformações

    fillComands(cmds,argc,argv);                                    // preencher a lista de transformações para não mexer com os argv's

    for(int cmd=0 ; cmd<nrCmds ; cmd++){                            // para todas as transformação

        if(cmd == 0){                                               // se for a primeira transformação
            pipe(fildes[0]);                                        // criar o pipe para indice igual a 0

            if( fork() == 0){                                       // fork vai tratar de o executar
                close(fildes[0][0]);                                // fechar a leitura do pipe 
                
                char* transformation = malloc(sizeof(char)*(strlen(cmds[cmd])+strlen(transformationsFolder))) ;     // alocar memória para o path necessario para o executavel
                addFolderToTransformation(cmds[cmd],transformation);                                                // colocar o path necessário em transformação

                dup2(fdinput,0);                                    // input passa a ser o ficheiro input inserido

                if(nrCmds == 1) dup2(fdoutput,1);                   // se este for também o ultimo comando, output passa a ser o ficheiro output inserido
                else dup2(fildes[0][1],1);                          // se não for último, output passa a ser a escrita no pipe

                close(fildes[0][1]);                                // fechar escrita no pipe

                execl(transformation,transformation,NULL);          // exec da primeira transformação
            }
        }

        else if( cmd == nrCmds-1){                                  // se for a ultima transformação
            close(fildes[cmd-1][1]);                                // fechar a escrita do pipe da transformação anterior (nao é mais necessário)
            if( fork() == 0 ){

                char* transformation = malloc(sizeof(char)*(strlen(cmds[cmd])+strlen(transformationsFolder))) ;     // alocar memória para o path necessario para o executavel
                addFolderToTransformation(cmds[cmd],transformation);                                                // colocar o path necessário em transformação

                dup2(fildes[cmd-1][0],0);                           // input passa a ser a leitura do pipe da transformação anterior
                dup2(fdoutput,1);                                   // output passa a ser o ficheiro output inserido
                close(fildes[cmd-1][0]);                            // fechar a leitura do pipe da transformação anterior (tudo fechado agora)

                execl(transformation,transformation,NULL);          // executar ultima transformação
            }
        }
        else{                                                       // transformação intermédia
            pipe(fildes[cmd]);                                      // criar pipe para a transformação atual
            close(fildes[cmd - 1][1]);                              // fechar escrita do pipe anterior
            if( fork() == 0 ){

                dup2(fildes[cmd-1][0],0);                           // input passa a ser a leitura do pipe anterior
                dup2(fildes[cmd][1],1);                             // output passa a ser a escrita no pipe atual
                close(fildes[cmd-1][0]);                            // fechar leitura do pipe anterior
                close(fildes[cmd][1]);                              // fechar escrita do pipe atual

                char* transformation = malloc(sizeof(char)*(strlen(cmds[cmd])+strlen(transformationsFolder))) ;  // alocar memória para o path necessario para o executavel
                addFolderToTransformation(cmds[cmd],transformation);                                             // colocar o path necessário em transformação

                execl(transformation,transformation,NULL);          // executar transformação intermédia
            }
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