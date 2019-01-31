#define billion 1E9L
#define iterations 10000
#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sched.h>
#include <stdlib.h>
#include <sys/types.h>
#include <assert.h>
#include <unistd.h>

int main()
{
	int fd1[2];
	int fd2[2];

	pipe(fd1) || pipe(fd2);

	cpu_set_t cpu_set;
	CPU_ZERO(&cpu_set);

	struct timespec start,end;

	//create child process
	pid_t  pid1;
	assert((pid1 = fork()) >= 0);
	

	/*	child process is the first 
	 *	part of the if statement, while
	 *	the parent process is the else statement
	 *	each process shall set its affinity to core 0
	 *      
	 *      child process will run a loop for as long as
	 *      read does not return an error, writing back to the 
	 *      parent facing pipe.
	 *
	 *      parent process will intialize the timespec structs 
	 *	and run a set iteration loop or reading and writing
	 *	this reading should force a context switch between 
	 *	parent and child
	 *
	 */
	if(pid1 == 0)
	{
		close(fd1[1]);
		close(fd2[0]);
		
		//set affinity
		CPU_SET(0, &cpu_set);
		sched_setaffinity(0,sizeof(cpu_set), &cpu_set);

		int i;
		//loop until fd1 is null	
		while(read(fd1[0], &i,4))
			write(fd2[1],&i,4);
		//close pipes
		close(fd1[0]);
		close(fd2[1]);
		return 0;
	}
	else
	{
		printf("calculating 10000 context switches\n");
		close(fd1[0]);
		close(fd2[1]);
	
		//set affinity
		CPU_SET(0, &cpu_set);
		sched_setaffinity(0, sizeof(cpu_set), &cpu_set);
		
		int i, i_returned;
		//start time
		clock_gettime(CLOCK_MONOTONIC, &start);
		//loop n iterations
		for(i = 0; i < iterations; ++i)
		{	
			write(fd1[1], &i,4);
			read(fd2[0], &i_returned,4);
			if(i_returned != i)
				break;
		}

		clock_gettime(CLOCK_MONOTONIC, &end);
		//calculate switch time poer iteration
		unsigned long long difference = billion * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
		printf("%llu nanosecond average for context switches\n", difference/iterations);
		
		//close pipes
		close(fd1[1]);
		close(fd2[0]);
		return 0;

	}
}
