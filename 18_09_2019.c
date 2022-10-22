/******************************************************************

SPECIFICATION TO BE IMPLEMENTED:
Implementare una programma che riceva in input, tramite argv[], un insieme di
stringhe S_1 ..... S_n con n maggiore o uguale ad 1. 
Per ogni stringa S_i dovra' essere attivato un thread T_i.
Il main thread dovra' leggere indefinitamente stringhe dallo standard-input.
Ogni stringa letta dovra' essere resa disponibile al thread T_1 che dovra' 
eliminare dalla stringa ogni carattere presente in S_1, sostituendolo con il 
carattere 'spazio'.
Successivamente T_1 rendera' la stringa modificata disponibile a T_2 che dovra' 
eseguire la stessa operazione considerando i caratteri in S_2, e poi la passera' 
a T_3 (che fara' la stessa operazione considerando i caratteri in S_3) e cosi' 
via fino a T_n. 
T_n, una volta completata la sua operazione sulla stringa ricevuta da T_n-1, dovra'
passare la stringa ad un ulteriore thread che chiameremo OUTPUT il quale dovra' 
stampare la stringa ricevuta su un file di output dal nome output.txt.
Si noti che i thread lavorano secondo uno schema pipeline, sono ammesse quindi 
operazioni concorrenti su differenti stringhe lette dal main thread dallo 
standard-input.

L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
WinAPI) in modo tale che quando il processo venga colpito esso dovra' 
stampare il contenuto corrente del file output.txt su standard-output.

In caso non vi sia immissione di dati sullo standard-input, l'applicazione 
dovra' utilizzare non piu' del 5% della capacita' di lavoro della CPU.

*****************************************************************/
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <fcntl.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int thread_number;
int sem_threads, sem_main;
char* filename;
char** strings;
char** main_string_pipe;
FILE* output;


void* thread_function(void* param) {

    struct sembuf op;
    int me = (int)param;
    int i, j;
    int ret;

    printf("thread[%d] is started\n", me);
    fflush(stdout);



    while(1) {
        op.sem_flg = 0;
        op.sem_num = me;
        op.sem_op = -1;

try_3:
        ret = semop(sem_threads, &op, 1);
        if(ret == -1 && errno == EINTR) goto try_3;
        if(ret==-1) {
            perror("sem error");
            exit(EXIT_FAILURE);
            
        }

        //for debug
        puts("\n*****************************************");
        printf("thread[%d] is in while(1)\n", me);
        printf("thread[%d]: strings[%d] -> %s\n", me, me, strings[me]);
        fflush(stdout);

        for(i=0; i<strlen(main_string_pipe[me]); i++) {
            for(j=0; j<strlen(strings[me]); j++) {
                if(main_string_pipe[me][i] == strings[me][j])
                    main_string_pipe[me][i] = ' ';
            }
        }

        
        main_string_pipe[me+1] = (char*)malloc(sizeof(char)*(strlen(main_string_pipe[me])+1));
        if(main_string_pipe[me+1] == NULL) {
            perror("malloc error");
            exit(EXIT_FAILURE);

        }

        memcpy(main_string_pipe[me+1], main_string_pipe[me], strlen(main_string_pipe[me])+1);

        //for debug
        printf("thread[%d]: main_string_pipe[%d] -> %s\n", me, me+1, main_string_pipe[me+1]);
        fflush(stdout);

        puts("\n*****************************************");


        op.sem_flg = 0;
        op.sem_num = me+1;
        op.sem_op = 1;

try_4:
        ret = semop(sem_threads, &op, 1);
        if(ret == -1 && errno == EINTR) goto try_4;
        if(ret==-1) {
            perror("sem error");
            exit(EXIT_FAILURE);
            
        }

        if(me == 0) {
            op.sem_flg = 0;
            op.sem_num = 0;
            op.sem_op = 1;

try_5:
            ret = semop(sem_main, &op, 1);
            if(ret == -1 && errno == EINTR) goto try_5;
            if(ret==-1) {
            perror("sem error");
            exit(EXIT_FAILURE);
            
        }

        }
    }
}

void* output_thread(void* param) {
    struct sembuf op;
    int ret;

    while(1) {

        op.sem_flg = 0;
        op.sem_num = thread_number;
        op.sem_op = -1;

try_3:
        ret = semop(sem_threads, &op, 1);
        if(ret == -1 && errno == EINTR) goto try_3;
        if(ret==-1) {
            perror("sem error");
            exit(EXIT_FAILURE);
            
        }


        fprintf(output, "%s\n", main_string_pipe[thread_number]);
        fflush(output);
    }
}


void handler(int dummy) {

    puts("\n\n##### STAMPO IL FILE #####\n");

    char buffer[1024];
    sprintf(buffer, "cat %s", filename);
    system(buffer);

    puts("##########################\n");
}


int main(int argc, char** argv) {

    int i, ret;
    pthread_t tid;
    filename = "ouput.txt";
    struct sembuf op;
    char* buffer;

    if(argc < 2) {
        perror("too few args");
        exit(EXIT_FAILURE);

    }

    thread_number = argc-1;   // dobbiamo creare N threads
    strings = argv+1;

    main_string_pipe = (char**)malloc(sizeof(char)*(thread_number+1));
    if(main_string_pipe == NULL) {
        perror("malloc error");
        exit(EXIT_FAILURE);

    }

    output = fopen(filename, "w+");
    if(output == NULL) {
        perror("fopen error");
        exit(EXIT_FAILURE);

    }

    sem_threads = semget(IPC_PRIVATE, thread_number+1, IPC_CREAT | 0666);
    if(sem_threads == -1) {
        perror("semget error");
        exit(EXIT_FAILURE);

    }

    for(i=0; i < thread_number+1; i++) {
        ret = semctl(sem_threads, i, SETVAL, 0);
        if(ret == -1) {
            perror("semctl error");
            exit(EXIT_FAILURE);
        }
    }

    sem_main = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    if(sem_main == -1) {
        perror("semget error");
        exit(EXIT_FAILURE);

    }

    ret = semctl(sem_main, 0, SETVAL, 1);
    if(ret == -1) {
        perror("semctl error");
        exit(EXIT_FAILURE);

    }

    for(i=0; i < thread_number; i++) {
        if(pthread_create(&tid, NULL, thread_function, (void*)i)) {
            perror("pthread_create error");
            exit(EXIT_FAILURE);

        }
    }

    if(pthread_create(&tid, NULL, output_thread, NULL)) {
        perror("pthread_create error");
        exit(EXIT_FAILURE);

    }  

    signal(SIGINT, handler);

    while(1) {
        
        op.sem_flg = 0;
        op.sem_num = 0;
        op.sem_op = -1;

try_1:
        ret = semop(sem_main, &op, 1);
        if(ret == -1 && errno == EINTR) goto try_1;

read_again:
        sleep(1);
        puts("\n-->dammi una stringa:");
        ret = scanf("%ms", &buffer);
        if(ret == EOF && errno == EINTR) goto read_again;
        if(ret==-1) {
            perror("sem error");
            exit(EXIT_FAILURE);
            
        }

        if(main_string_pipe[0] != NULL) {
            free(main_string_pipe[0]);
        }

        main_string_pipe[0] = (char*)malloc(sizeof(char)*(strlen(buffer)+1));
        if(main_string_pipe[0] == NULL) {
            perror("malloc error");
            exit(EXIT_FAILURE);

        }

        memcpy(main_string_pipe[0], buffer, strlen(buffer)+1);

        printf("-->main: %s\n\n", main_string_pipe[0]);
        fflush(stdout);

        op.sem_flg = 0;
        op.sem_num = 0;
        op.sem_op = 1;

try_2:
        ret = semop(sem_threads, &op, 1);
        if(ret == -1 && errno == EINTR) goto try_2;
        if(ret==-1) {
            perror("sem error");
            exit(EXIT_FAILURE);

        }

    }  

    return 0;
}
