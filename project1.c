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

// key number
#define SHMKEY ((key_t) 1497);

typedef struct
{
	int value;
} shared_memory;

shared_memory *total;

int main(void)
{
	return 0;
}

