/*
 *	Author: Steven Faulkner
 *	U9616-1844
 *	Project 1 for COP4600
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
	t->value += 100000;
	printf("From Process 1: counter = %d\n", t->value);
}
void ChildProcess2(shared_memory *t)
{
	t->value += 200000;
	printf("From Process 2: counter = %d\n", t->value);
}
void ChildProcess3(shared_memory *t)
{
	t->value += 300000;
	printf("From Process 3: counter = %d\n", t->value);
}
void ChildProcess4(shared_memory *t)
{
	t->value += 500000;
	printf("From Process 4: counter = %d\n", t->value);
}

int main(void)
{

	
	//variable declaration
	int shmid;
	pid_t pid1, pid2, pid3, pid4;

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

	printf("End of Program\n");	

	//fin
	exit(0);
}

