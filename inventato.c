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

FILE *file;
int nthread;
char **buffers; //lista degli argumenti a partire da argv[2]
pthread_mutex_t *done;
pthread_mutex_t *ready;
char buffer[4096];


typedef struct __thread_args {
	int me;
	char *my_string;
} thread_args;

void* thread_function(void *arg){
	int ret;
	int me = (int) arg;
	int i,j;
        while(1) {
		
                pthread_mutex_lock(ready+me);        // chiamata bloccante se ready[curr] e' lockato
		printf("thread %d in while\n",me);
		
		if(strcmp(buffer,buffers[me])==0){

			for(i=0;i<strlen(buffer);i++){
				buffer[i]='*';
			}
			
			
		}
		printf("%s --- %s \n", buffers[me],buffer);
		printf("fine thread [%d]\n",me);
		puts("--------------------------------");
	
		if(me < nthread-1){
			pthread_mutex_unlock(ready+me+1);
		}else{
			pthread_mutex_unlock(done);
		}
}

                                
}


void error(char *str){
	perror(str);
	exit(EXIT_FAILURE);
}
int main(int argc, char **argv){
	//char *buffer;
	int i,ret;
	pthread_t tid;
	if (argc<2){
		error("missed arg");
	}
	nthread = argc-2;
	file = fopen(argv[1],"w+");
	if(file== NULL){
		error("open file errror");
	}
	buffers=argv+2;
	//alloco i mutex
	ready = malloc(sizeof(pthread_mutex_t)*nthread);
	done = malloc(sizeof(pthread_mutex_t));
	//init mutex
	for(i=0;i<nthread;i++){
		if(pthread_mutex_init(ready+i,NULL) || pthread_mutex_lock(ready+i)){
			error("ready init or lock error");
		}
	}
	if(pthread_mutex_init(done,NULL)){
		error("done init error");
	}
	//creo i thread
	for(i=0;i<nthread;i++){
		if(pthread_create(&tid, NULL, thread_function, (void*)i) != 0){
			error("thread creating error");
		}

	} 
	while(1){
		
		pthread_mutex_lock(done);
		if(strcmp(buffer,"\0") != 0){
			printf("SCRIVO NEL FILE, PRIMA DI CHIEDERTI UNA NUOVA STRINGA\n");
			fprintf(file,"%s\n",buffer);
			fflush(file);
		}
		sleep(1);
		puts("--->dammi una parola:");
read_again:                
                ret = scanf("%s", buffer);
                if(ret == EOF && errno == EINTR) goto read_again;
                
                pthread_mutex_unlock(ready);

	}
	return 0;

}

