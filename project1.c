/*
 *	Author: Steven Faulkner
 *	Project 1 for COP4600
 *	
 */


#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

typedef struct
{
	int value;
}shared_memory;

shared_memory *total;


void ChildProcess1(shared_memory *a)
{
	a->value += 1000000;
	printf("From Process 1: counter = %d\n", a->value);
}
void ChildProcess2(shared_memory *a)
{
	a->value += 1000000;
	printf("From Process 2: counter = %d\n", a->value);
}
void ChildProcess3(shared_memory * a)
{
	a->value += 1000000;
	printf("From Process 3: counter = %d\n", a->value);
}
void ChildProcess4(shared_memory *a)
{
	a->value += 1000000;
	printf("From Process 3: counter = %d\n", a->value);
}

int main(void)
{

	int shmid;
	pid_t pid;
	int status;

	shmid = shmget(IPC_PRIVATE,sizeof(int),IPC_CREAT | 0666);
	if (shmid < 0 )
	{
		printf("shmget error\n");
		exit(0);
	}

	total =  shmat(shmid,NULL,0);
	if((int) total->value == -1)
	{
		printf("shmat error\n");
		exit(1);
	}

	total->value = 0;

	pid = fork();
	if(pid < 0)
	{
		printf("fork error 1\n");
		exit(1);
	}
	else if (pid == 0)
	{
		ChildProcess1(total);
		exit(0);
	}


	pid = fork();
	if(pid < 0)
	{
		printf("fork error 2\n");
		exit(1);
	}
	else if(pid == 0)
	{
		ChildProcess2(total);
		exit(0);
	}


	pid = fork();
	if(pid < 0)
	{
		printf("fork error 3\n");
		exit(1);
	}
	else if( pid == 0)
	{
		ChildProcess3(total);
		exit(0);
	}


	pid = fork();
	if(pid < 0)
	{
		printf("fork error 4\n");
		exit(1);
	}
	else if(pid == 0)
	{
		ChildProcess4(total);
		exit(0);
	}

	
	exit(0);
}

