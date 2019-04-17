#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#define MSG_HELLO 100
#define MSG_SIZE 2

int hasToken = 0;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void sec_loc(int time, int rank){
	printf("Local section in %d\n", rank);
	sleep(time);
}

void sec_crit(int time, int rank){
	printf("Critical section in %d\n", rank);
	sleep(time);
}

void *recvThread(void *rank){
	MPI_Status status;
	int msg[MSG_SIZE];
	int size;
	while(1){
		MPI_Recv(msg, MSG_SIZE, MPI_INT,MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		MPI_Get_count( &status, MPI_INT, &size);
		hasToken = 1;
		pthread_cond_broadcast(&cond);
	}
	
}

void wait(int time, int rank){
	printf("Waiting in %d\n", rank);
	sleep(time);
}

int main(int argc, char **argv)
{
	int rank,size,sender=0,receiver=1;
	int msg[MSG_SIZE];
	MPI_Status status;
	srand(time(NULL));
	MPI_Init(&argc, &argv);

	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	MPI_Comm_size( MPI_COMM_WORLD, &size );

	pthread_mutex_t mutex;
	
	pthread_mutex_init(&mutex, NULL);

	pthread_t thread_id;
	pthread_create(&thread_id, NULL, recvThread, NULL);
	pthread_cond_init(&cond, NULL);
	
	if (rank == 0)
	{
		msg[0] = rank;
		
		msg[1] = size;
		sec_loc(rand()%4 + 3, rank);
		printf("%d: Wysylam (%d %d) do %d\n", rank, msg[0], msg[1], receiver);
		MPI_Send( msg, MSG_SIZE, MPI_INT, 1, MSG_HELLO, MPI_COMM_WORLD );
		
	}
	receiver = (rank+1)%size;
	while (1)
	{
		sec_loc(rand()%4 + 3, rank);
		pthread_mutex_lock(&mutex);
		do{
			if(hasToken ==1){
				sec_crit(rand()%4 + 3, rank);
				printf("%d: Wysylam (%d %d) do %d\n", rank, msg[0], msg[1], receiver);
				MPI_Send( msg, MSG_SIZE, MPI_INT, receiver, MSG_HELLO, MPI_COMM_WORLD );
				hasToken = 0;
				break;
			}
			else
				pthread_cond_wait(&cond, &mutex);
		} while(1);
		pthread_mutex_unlock(&mutex);	
/*		if(msg[0] < 100 && msg[0] >=0)
			msg[0]++;
		else
			msg[0]=-1; */
		
		
	}
	MPI_Finalize();
}