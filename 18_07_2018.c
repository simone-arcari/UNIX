/******************************************************************
 
SPECIFICATION TO BE IMPLEMENTED:
Implementare un programma che riceva in input tramite argv[] i pathname 
associati ad N file, con N maggiore o uguale ad 1. Per ognuno di questi
file generi un processo che legga tutte le stringhe contenute in quel file
e le scriva in un'area di memoria condivisa con il processo padre. Si 
supponga per semplicita' che lo spazio necessario a memorizzare le stringhe
di ognuno di tali file non ecceda 4KB. 
Il processo padre dovra' attendere che tutti i figli abbiano scritto in 
memoria il file a loro associato, e successivamente dovra' entrare in pausa
indefinita.
D'altro canto, ogni figlio dopo aver scritto il contenuto del file nell'area 
di memoria condivisa con il padre entrera' in pausa indefinita.
L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
WinAPI) in modo tale che quando il processo padre venga colpito da esso dovra' 
stampare a terminale il contenuto corrente di tutte le aree di memoria 
condivisa anche se queste non sono state completamente popolate dai processi 
figli.

*****************************************************************/
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MIN_ARGS_NUMBER 2
#define SEGMENT_SIZE    4096 


int process_number;
char** strings;
char* mem;


sighandler_t handler(void) {
    
}


int main(int argc, char** argv) {

    int i;
    pid_t pid;
    FILE* file;

    if(argc < MIN_ARGS_NUMBER) {
        printf("too few args\n");
        printf("usage: ./name pathname1 pathname2 ...\n");
        exit(EXIT_FAILURE);

    }

    process_number = argc-1;
    strings = argv+1;

    mem = malloc(sizeof(char)*SEGMENT_SIZE*process_number);
    if(mem == NULL) {
        perror("error in malloc");
        exit(EXIT_FAILURE);

    }

    signal(SIGINT, handler);

    for(i=0; i<process_number; i++) {
        pid = fork();
        if(pid == -1) {
            perror("error in fork");
            exit(EXIT_FAILURE);

        }

        if(pid==0) break;

    }


    if(pid==0) {

        file = fopen(strings[i], "r+");
        if(file==NULL) {
            perror("error in fopen");
            exit(EXIT_SUCCESS);

        }

        fgets(file, SEGMENT_SIZE, mem+(i*SEGMENT_SIZE));


    }else {





    }








    return EXIT_SUCCESS;

}