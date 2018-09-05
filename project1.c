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

void ChildProcess1(int *a);
void ChildProcess2(int *a);
void ChildProcess3(int *a);
void ChildProcess4(int *a);

typedef struct
{
	int value;
} shared_memory;

shared_memory *total;


int main(void)
{

	int shmid;
	pid_t pid;

	
	return 0;
}

