#define _GNU_SOURCE
#define billion 1E9
#define iterations 500

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sched.h>
#include <pthread.h>

struct timespec start;
struct timespec end;
char input[iterations];
int fd1[2];
int fd2[2];

void* thread_1_function()
{
	//write to the buffer that parent was waiting on
	write(fd2[1],"",sizeof(""));
	//initalize read to transfer controlback	
	read(fd1[0], input,0);
}


int main(int argc, char argv[])
{
	//time struct declaration
	struct timespec start,end;
	
	//sets program to only use core 0
	cpu_set_t cpu_set;
	CPU_ZERO(&cpu_set);
	CPU_SET(0,&cpu_set);


	if((sched_setaffinity(0, sizeof(cpu_set_t), &cpu_set) < 1))
	{

	int nproc = sysconf(_SC_NPROCESSORS_ONLN);
	int k;
	
	printf("Processor used: ");
	for(k = 0; k < nproc; ++k)
	{
		printf("%d ", CPU_ISSET(k, &cpu_set));
	}
	
	printf("\n");


	if(pipe(fd2) == -1)
	{
		printf("fd1 pipe error");
		return 1;
	}
	//fail on file descriptor 2 fail
	if(pipe(fd2) == -1)
	{
		printf("fd2 pipe error");
		return 1;
	}


	pthread_t thread_1;


	pthread_create(&thread_1, NULL, &thread_1_function, NULL);

	
	pthread_join(thread_1,NULL);

	//seed parent pipe
	write(fd1[1],"start",sizeof("start"));	
	
	
	int i;
	uint64_t sum = 0;

	for(i = 0; i < iterations; ++i)
	{

		//initalize clock start
		clock_gettime(CLOCK_MONOTONIC, &start);
		//wait for child thread to write to pipe
		read(fd2[0],input,0);
		//record clock end
		clock_gettime(CLOCK_MONOTONIC, &end);	
		
		write(fd1[1],"",sizeof(""));

		

		uint64_t diff;
		diff = billion * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
		diff = diff;
		sum += diff;
	}

	
	sum = sum/iterations;
	printf("%.4llu nanosecond average for thread context switching", (long long unsigned)sum);

	close(*fd1);
	close(*fd2);
}

	return 0;

}
