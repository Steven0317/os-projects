#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <time.h>

#define billion 1E9
#define iterations 10000

int main(int argc, char* argv[])
{
	char* file = "test.txt";
		
	

	//arbitrary data initalization
	char data[1];
	int i;

	//time stamp struxct initalizer
	struct timespec start, end;

	//array to store interval time
	uint64_t diff;
	long long unsigned duration[iterations];

	printf("taking the average of 10000 system calls\n");

	for(i = 0; i < iterations; ++i)
	{
		//open file call
		
		int fileDescriptor = open(file, O_RDONLY);
	
		if(fileDescriptor < 0)
			return 1;

		clock_gettime(CLOCK_MONOTONIC, &start);
		//read file call
		read(fileDescriptor,data,0);
		clock_gettime(CLOCK_MONOTONIC, &end);
		
		//calculate time difference in nanoseconds
		diff = billion * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec; 	
		
		
		duration[i] = (long long unsigned)diff;

		//close file call
		if(close(fileDescriptor) < 0)
			return 1;
	}

	uint64_t sum = 0;

	for(i = 0; i < iterations; ++i)
	{
		sum += duration[i];	
	}

	//prints out average duration of system call in nanoseconds
	printf("%llu nanosecond average for system calls\n",(long long unsigned)(sum)/iterations);
	//printf("%llu", (long long unsigned)sum/iterations);
	return 0;
}
