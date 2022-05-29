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
#define SIZEOFBUFF 400
#define QUEUESIZE 15
#define RESPONSEMAXSIZE 100
#define MAXFILESIZEINT 50

typedef struct request {
    int nrCmds;
    char* pid;    
    char* input;
    char* output;
    char** cmds; 
    int priority;
}* Request;


char* transformationsRepsFolder = "./../transformationsReps/Reps";
char* transformationsFolder = "./../transformations/";

char* transformationsFile[NROFTRANSF];
int repsFile[NROFTRANSF];

int transformationsReps[NROFTRANSF];

Request* queue;
Request* working;
int end = 0;



void readTransformationsReps(char* filepath){                    //Lê o ficheiro de reps de transformacoes

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

void addFolderToTransformation(char* t, char* transformation){      //vai colocar em transformation o caminho correto para a transformacao t   
    strcpy(transformation,transformationsFolder);                  // Ex:  t=bcompress transformation = ./transformations/bcomrpess
    strcat(transformation,t);
}

void fillRequest(char* buff, Request r){                 // vai preencher cmds com todas os elementos do pedido

    r->pid = strdup(strsep(&buff," "));
    r->nrCmds = atoi(strsep(&buff," "));
    r->priority = atoi(strsep(&buff," "));
    r->input = strdup(strsep(&buff," "));
    r->output = strdup(strsep(&buff," "));

    r->cmds = malloc(sizeof(char)*(r->nrCmds));
    
    for(int i=0 ; i< (r->nrCmds) ; i++)
        r->cmds[i] = strdup(strsep(&buff," "));
}

void processFile(Request r, int fdPipe){

    printf("Processing... <%s>\n", r->pid);
    char* buff = "Processing...";
    write(fdPipe, buff, strlen(buff));

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

    for(int i = 0; i < nrCmds; i++){
        wait(NULL);
    }


    char* tamInput = malloc(sizeof(char)*MAXFILESIZEINT);
    char* tamOutput = malloc(sizeof(char)*MAXFILESIZEINT);

    int tamanhoInput = lseek(fdinput, 0, SEEK_END);
    int tamanhoOutput = lseek(fdoutput, 0, SEEK_END);

    sprintf(tamInput,"%d",tamanhoInput);
    sprintf(tamOutput,"%d",tamanhoOutput);

    close(fdinput);
    close(fdoutput);

    char* concateneted = malloc(sizeof(char)*RESPONSEMAXSIZE);

    strcat(concateneted,tamInput);
    strcat(concateneted," ");
    strcat(concateneted,tamOutput);

    write(fdPipe,concateneted,strlen(concateneted));

}

void showState(char* pid){

    char* buff = malloc(sizeof(char) * SIZEOFBUFF);
    int fdFifoWr = open(pid,O_WRONLY);

    for(int i = 0; i < QUEUESIZE; i++){
        Request r = working[i];
        if(r){
            sprintf(buff, "task #%d: proc-file %d %s %s", i, r->priority, r->input, r->output);
            for(int j = 0; j < r->nrCmds; j++){
                strcat(buff, " ");
                strcat(buff, r->cmds[j]);
            }
            strcat(buff, "\n");
            write(fdFifoWr,buff,strlen(buff));
        }
    }

    for(int i = 0 ; i < NROFTRANSF; i++){
        sprintf(buff, "transf %s: %d/%d (running/max)\n", transformationsFile[i], transformationsReps[i], repsFile[i]);
        write(fdFifoWr,buff,strlen(buff));
    }

    close(fdFifoWr);
}

void setToZeroAllTransformationsReps(int transformationsReps[]){
    for(int i=0 ; i<NROFTRANSF ; i++)
        transformationsReps[i] = 0;
}

int checkReps(char** elements,int nrElements){
    for(int i=0 ; i<nrElements ; i++){
        for( int j=0 ; j<NROFTRANSF ; j++){
            if( strcmp(elements[i],transformationsFile[j])==0 )
                if (transformationsReps[j] == repsFile[j] )
                    return 1;
        }
    }
    return 0;
}

void updateReps(Request r){
    for(int i=0 ; i<r->nrCmds ; i++){
        for(int j=0 ; j<NROFTRANSF ; j++){
            if(strcmp(r->cmds[i],transformationsFile[j])==0)
                transformationsReps[j] += 1;
        }
    }
}

void removeReps(char* transformation){
    for(int i=0 ; i<NROFTRANSF ; i++){
        if(strcmp(transformation,transformationsFile[i])==0)
            transformationsReps[i]--;
    }
}

void fillQueueNULL(){
    queue = malloc(sizeof(struct request)*QUEUESIZE);
    working = malloc(sizeof(struct request)*QUEUESIZE);
    
    for(int i=0 ; i<QUEUESIZE ; i++){
        queue[i] = NULL;
        working[i] = NULL;
    }
}

void addToQueue(Request r){
    for(int i=0 ; (i<QUEUESIZE) ; i++)
        if(queue[i] == NULL){
            queue[i] = r;
            return;
        }
}

void removeFromQueue(Request r){
    for(int i=0 ; i<QUEUESIZE ; i++){
        if(queue[i] == r)
            queue[i] = NULL;
    }
}

void addToWorking(Request r){
    for(int i=0 ; (i<QUEUESIZE); i++)
        if(!working[i]){
            working[i] = r;
            return;
        }
}

Request removeFromWorking(char* pid){
    Request r = NULL;

    for(int i=0 ; i<QUEUESIZE ; i++){
        if(working[i] && strcmp(working[i]->pid, pid) == 0){
            r = working[i];
            working[i] = NULL;
        }
    }
    return r;
}

int requestInQueueCanProceed(){
    for(int priorityLevel = 5; priorityLevel >= 0; priorityLevel--)
        for(int i=0 ; i<QUEUESIZE ; i++)
            if(queue[i] != NULL && queue[i]->priority == priorityLevel)
                if(checkReps(queue[i]->cmds,queue[i]->nrCmds) == 0)
                    return i;

    return -1;
}

int checkRequest(char** buff){

    char* type = strsep(buff," ");
    int ret = 0;

    if(strcmp(type,"proc-file") == 0) 
        ret = 1;
    else if(strcmp(type,"status") == 0) 
        ret = 2;
    
    return ret;
}

void checkQueue(){

    int requestToProceed;

    while( (requestToProceed = requestInQueueCanProceed()) >= 0){
        Request r = queue[requestToProceed];
        
        int fdPipe = open(r->pid,O_WRONLY);
        updateReps(r);
        addToWorking(r);
        removeFromQueue(r);
        if (fork() == 0){
            processFile(r,fdPipe);
            exit(0);
        }
        close(fdPipe);
    }
}

int isWorkingEmpty(){
    for(int i=0 ; (i<QUEUESIZE); i++)
        if(working[i])
            return 0;
    return 1;
}

void ctrl_c(int signum){
    if(isWorkingEmpty()){
        int p = getpid();
        kill(p,SIGKILL);
    }
    end = 1;
}

int main(int argc, char** argv){
    signal(SIGINT,ctrl_c);

    readTransformationsReps(argv[1]);
    setToZeroAllTransformationsReps(argv[2]);

    mkfifo("../tmp/FifoMain",0666);

    int fdFifo = open("../tmp/FifoMain",O_RDONLY);
    int fdFifoWR = open("../tmp/FifoMain",O_WRONLY);

    char* buff = malloc(sizeof(char)*SIZEOFBUFF);
    int readBytes;

    fillQueueNULL();

    while((readBytes = read(fdFifo,buff,SIZEOFBUFF)) > 0){
        Request r;
        char* pid;

        switch(checkRequest(&buff)){
            case 1:
                if(!end){
                    r = malloc(sizeof(struct request));
                    fillRequest(buff,r);

                    printf("Pending... <%s>\n", r->pid);
                    
                    addToQueue(r);
                    checkQueue();
                }
                break;

            case 2:
                if(!end){
                    if(fork()==0){
                        pid = strsep(&buff," ");
                        showState(pid);
                        printf("Status <%s>\n", pid);
                        exit(0);
                    }
                }
                break;

            default:
                pid = strdup(strsep(&buff," "));

                r = removeFromWorking(pid);

                if(r){
                    for(int i=0; i < r->nrCmds; i++){
                        removeReps(r->cmds[i]);
                    }
                } else printf("nao fiz nada\n");

                printf("Concluded! <%s>\n", pid);
                checkQueue();
                break;
        }

        if(end && isWorkingEmpty()){
            int p = getpid();
            kill(p,SIGKILL);
        }
    }
    close(fdFifoWR);
    close(fdFifo);
    return 0;
}