#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <time.h>

#define billion 1E9

int main(int argc, char* argv[])
{
	char* file = "";
	/* Check for command line file arguement
	 * prompts for file if null
	 */
	if(argc > 1)
	{
		file = argv[1];
	}
	else
	{
		printf("Enter a file within the directory: ");
		scanf("%s",file);
	}
	

	//arbitrary data initalization
	char data[1];
	int i;

	//time stamp struxct initalizer
	struct timespec start, end;

	//array to store interval time
	uint64_t diff;
	long long unsigned duration[100];

	for(i = 0; i < 100; ++i)
	{
		//open file call
		
		int fileDescriptor = open(file, O_RDONLY);
	
		if(fileDescriptor < 0)
			return 1;

		clock_gettime(CLOCK_MONOTONIC, &start);
		//read file call
		read(fileDescriptor,data,1);
		clock_gettime(CLOCK_MONOTONIC, &end);
		
		//calculate time difference in nanoseconds
		diff = billion * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec; 	
		
		
		duration[i] = (long long unsigned)diff;

		//close file call
		if(close(fileDescriptor) < 0)
			return 1;
	}

	uint64_t sum = 0;

	for(i = 0; i < 100; ++i)
	{
		sum += duration[i];	
	}

	//prints out average duration of system call in nanoseconds
	printf("%llu nanosecond average for system calls\n",(long long unsigned)sum/i);
	return 0;
}
