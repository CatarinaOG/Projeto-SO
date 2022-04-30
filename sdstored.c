#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>


#define NROFTRANSF 7
#define ARGVMAXSIZE 300
#define SIZEOFBUFF 100
#define MAXQUEUESIZE 100
#define QUEUESIZE 10

typedef struct request {
    int nrCmds;
    int pid;    
    char* input;
    char* output;
    char** cmds; 
}* Request;


char* transformationsRepsFolder = "./transformationsReps/Reps";
char* transformationsFolder = "./transformations/";

char* transformationsFile[NROFTRANSF];
int repsFile[NROFTRANSF];

int transformationsReps[NROFTRANSF];

Request* queue;



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
            repsFile[ind] = atoi(token);                        // guardar nr de reps na lista de reps (msm indice que a correspondente transformação)
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

int fillRequest(char* buff, Request r){                 // vai preencher cmds com todas os elementos do pedido
    
    r->pid = atoi(strsep(&buff," "));
    r->nrCmds = atoi(strsep(&buff," "));
    r->input = strsep(&buff," ");
    r->output = strsep(&buff," ");

    r->cmds = malloc(sizeof(char)*(r->nrCmds));
    
    for(int i=0 ; i< (r->nrCmds) ; i++){
        r->cmds[i] = strdup(strsep(&buff," "));
    }
}

int processFile(Request r){

    char** cmds = r->cmds;
    int nrCmds = r->nrCmds;
    
    int fdinput = open(r->input,O_RDONLY);                                      
    int fdoutput = open(r->output,O_WRONLY | O_TRUNC | O_CREAT, 0666);

    int fildes[nrCmds][2];                                          // lista de fildes para cada uma das transformações

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

int setToZeroAllTransformationsReps(int transformationsReps[]){

    for(int i=0 ; i<NROFTRANSF ; i++){
        transformationsReps[i] = 0;
    }

}

int checkReps(char** elements,int nrElements){

    for(int i=0 ; i<nrElements ; i++){
        for( int j=0 ; j<NROFTRANSF ; j++)
            if( strcmp(elements[i],transformationsFile[j])==0 ){
                if (transformationsReps[j] == repsFile[j] ) return 1;
            }
    }
    return 0;
}

int updateReps(Request r){

    for(int i=0 ; i<r->nrCmds ; i++){
        for(int j=0 ; j<NROFTRANSF ; j++){
            if(strcmp(r->cmds[i],transformationsFile[j])==0)
                transformationsReps[j] += 1;
        }
    }

}

int fillQueueNULL(){

    for(int i=0 ; i<QUEUESIZE ; i++){
        queue[i] = NULL;
    }

}

int addToQueue(Request r){

    int added = 0;

    for(int i=0 ; (i<QUEUESIZE) && (added == 0) ; i++){
        if(queue[i] == NULL){
            queue[i] = r;
            added = 1;
        }
    }
}

int removeFromQueue(Request r){
    for(int i=0 ; i<QUEUESIZE ; i++){
        if(queue[i] == r)
            queue = NULL;
    }
}

int requestInQueueCanProceed(){

    for(int i=0 ; i<QUEUESIZE ; i++){
        if(checkReps(queue[i]->cmds,queue[i]->nrCmds) == 0){
            return i;
        }
    }

    return -1;
}

int main(int argc, char** argv){

    readTransformationsReps(transformationsRepsFolder);
    setToZeroAllTransformationsReps(transformationsReps);
    
    mkfifo("FifoMain",0666);

    int fdFifo = open("FifoMain",O_RDONLY);
    int fdFifoWR = open("FifoMain",O_WRONLY);
    
    char* buff = malloc(sizeof(char)*SIZEOFBUFF);
    int readBytes;
    int requestToProceed;
    char* elementOfRequest;

    queue = malloc(sizeof(struct request)*QUEUESIZE);
    fillQueueNULL();

    
    while((readBytes = read(fdFifo,buff,SIZEOFBUFF))>0 || (requestToProceed = requestInQueueCanProceed()) > 0){


        if(readBytes > 0){
            Request r = malloc(sizeof(struct request));
            fillRequest(buff,r);

            int proceed = checkReps(r->cmds,r->nrCmds);
            
            if(proceed == 0){
                updateReps(r);
                processFile(r);
            }
            else{
                addToQueue(r);
            }
            printf("%s:%d\n",transformationsFile[3],transformationsReps[3]);
        }
        else{
            Request r = queue[requestToProceed];
            updateReps(r);
            removeFromQueue(r);
            processFile(r);
        }

        
        if(queue[0] != NULL)
            printf("pid %d in queue\n",queue[0]->pid);
    }
}