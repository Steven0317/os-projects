/*
 *	Author: Steven Faulkner
 *	U9616-1844
 *	Project 2 for COP4600
 *	
 */


#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/sem.h>


#define SEMKEY ((key_t) 400L)

#define NSEMS 1

int sem_id;

static struct sembuf OP = {0,-1,0};
static struct sembuf OV = {0,1,0};

struct sembuf *P = &OP;
struct sembuf *V = &OV;

typedef union{
	int val;
	struct semid_ds *buf;
	ushort *array;
}semunion;


int Pop()
{
	int status;
	status = semop(sem_id,P,1);
	return status;
}


int Vop()
{
	int status;
	status = semop(sem_id,V,1);
	return status;
}

//struct for shared memory
typedef struct
{
	int value;
}shared_memory;

shared_memory *total;



//functions for child process 1 - 4 that will manipulate
//the value variable within the struct 
//   @shared_memory *t: struct passed to the function
void ChildProcess1(shared_memory *t)
{
	int i = 0;
	while(i < 100000)
	{	
		Pop();
		if(t->value < 1100000)
		{

			t->value++;
		}
		Vop();
	i++;
	}
	printf("From Process 1: counter = %d\n", t->value);
}
void ChildProcess2(shared_memory *t)
{
	int i = 0;
	while(i < 200000)
	{	
		Pop();
		if(t->value < 1100000)
		{

			t->value++;
		}
		Vop();
	i++;
	}
	printf("From Process 2: counter = %d\n", t->value);
}
void ChildProcess3(shared_memory *t)
{
	int i = 0;
	while(i < 300000)
	{
		Pop();
		if(t->value < 1100000)
		{

			t->value++;
		}
		Vop();
		i++;
	}
	printf("From Process 3: counter = %d\n", t->value);
}
void ChildProcess4(shared_memory *t)
{
	int  i = 0;
	while(i < 500000)
	{
		Pop();
		if(t->value < 1100000)
		{

			t->value++;
		}
		Vop();
		i++;
	}
	printf("From Process 4: counter = %d\n", t->value);
}

int main(void)
{

	
	//variable declaration
	int shmid, value, value1;
	pid_t pid1, pid2, pid3, pid4;
	int semnum = 0;
	int status = 0;
	semunion semctl_arg;

	semctl_arg.val = 1;


	sem_id = semget(SEMKEY,NSEMS, IPC_CREAT | 0666);
	if(sem_id < 0)
		perror("Semaphore error");


	value1 = semctl(sem_id,semnum,SETVAL,semctl_arg);
	value = semctl(sem_id,semnum,GETVAL,semctl_arg);
	if(value < 1) 
		perror("SETVAL ERROR");
	
	//allocate shared memory return error if failed
	shmid = shmget(IPC_PRIVATE,sizeof(int),IPC_CREAT | 0666);
	if (shmid < 0 )
	{
		printf("shmget error\n");
		exit(0);
	}
	

	//return pointer to shared memory allocation retun error if failed
	total =  shmat(shmid,NULL,0);
	if((int) total->value == -1)
	{
		printf("shmat error\n");
		exit(1);
	}

	//intialize value to zero
	total->value = 0;

	//create first child
	pid1 = fork();
	if(pid1 < 0)
	{
		printf("fork error 1: %s\n", strerror(errno));
		exit(1);
	}
	else if (pid1 == 0)
	{
		ChildProcess1(total);
		exit(0);
	}

	//create second child
	pid2 = fork();
	if(pid2 < 0)
	{
		printf("fork error 2: %s\n", strerror(errno));
		exit(1);
	}
	else if(pid2 == 0)
	{
		ChildProcess2(total);
		exit(0);
	}

	//create third child
	pid3 = fork();
	if(pid3 < 0)
	{
		printf("fork error 3: %s\n", strerror(errno));
		exit(1);
	}
	else if(pid3 == 0)
	{
		ChildProcess3(total);
		exit(0);
	}

	//create 4th child
	pid4 = fork();
	if(pid4 < 0)
	{
		printf("fork error 4: %s\n", strerror(errno));
		exit(1);
	}
	else if(pid4 == 0)
	{
		ChildProcess4(total);
		exit(0);
	}
	//print id while waiting for children to finish
	printf("Child with ID: %d exited\n", waitpid(pid4,NULL,0));
	printf("Child with ID: %d exited\n", waitpid(pid3,NULL,0));
	printf("Child with ID: %d exited\n", waitpid(pid2,NULL,0));
	printf("Child with ID: %d exited\n", waitpid(pid1,NULL,0));

	//release shared memory to avoid memory leaks
	shmdt((void *) total);
	shmctl(shmid,IPC_RMID,NULL);

	semctl_arg.val = 0;
	status = semctl(sem_id,0,IPC_RMID,semctl_arg);
	if(status < 0)
		perror("Removing sem error");

	printf("End of Program\n");	

	//fin
	exit(0);
}

