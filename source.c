#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#define MSG_HELLO 100
#define MSG_SIZE 2

int answerCount = 0;
// zmienne globalne (odbierane przez recvThread, wykorzystywane przez main:

void sec_loc(int time, int rank){
	printf("Local section in %d\n", rank);
	sleep(time);
}

void *recvThread(void *rank){
	MPI_Status status;
	int msg[MSG_SIZE];
	int size;
	while(1){
		MPI_Recv(msg, MSG_SIZE, MPI_INT,MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		int tid = status.MPI_SOURCE;
		int tag = status.MPI_TAG;
		MPI_Get_count(&status, MPI_INT, &size);
		// (?) nowy wątek:
		if(tag == STATUS_REQUEST){
			msg[0] = myStatus;
			msgSize = 1;
			if(myStatus == LANE || myStatus == CARRIER || myStatus == START_QUEUE){
				msg[1] = myLane;
				msgSize += 1;
			}
			MPI_Send(answerMSG, msgSize, MPI_STRING, tid, MSG_STATUS_ANSWER, MPI_COMM_WORLD)
		}
	}
}

int main(int argc, char **argv)
{
	int rank,size,sender=0,receiver=1;
	int msg[MSG_SIZE];
	int queue = -1;
	MPI_Status status;
	srand(time(NULL));
	MPI_Init(&argc, &argv);

	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	MPI_Comm_size( MPI_COMM_WORLD, &size );
	
	/*if (rank == 0)
	{
		msg[0] = rank;
		
		msg[1] = size;
		sec_loc(rand()%4 + 3, rank);
		printf("%d: Wysylam (%d %d) do %d\n", rank, msg[0], msg[1], receiver);
		MPI_Send( msg, MSG_SIZE, MPI_INT, 1, MSG_HELLO, MPI_COMM_WORLD );
		
	} */
	while (1)
	{
		sec_loc(rand()%4 + 3, rank);
		pthread_mutex_lock(&mutex);
		queue = -1;
		answerCount = 0;
		do{
			msg[0] = rank;
			for(int i = 0; i < size, i++){
				if(i != rank)
					MPI_Send(msg, 1, MPI_STRING, i, MSG_STATUS_REQUEST, MPI_COMM_WORLD);
			}
			// if answerCount = N - 1:
			if(queue >= 0 && queue < freeLanes){
				sec_crit(rand()%4 + 3, rank);
				printf("%d: Wysylam (%d %d) do %d\n", rank, msg[0], msg[1], receiver);
				MPI_Send( msg, MSG_SIZE, MPI_INT, receiver, MSG_HELLO, MPI_COMM_WORLD, status);
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
