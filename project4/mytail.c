#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>


#define buffer 100


int main(int argc, char** argv){
    
    int lineCount = 20;
    char* fName;
    char* line;
    int fDesc;
    FILE* file;

    if(argc < 2){
        printf("missing args");
        exit(1);
    }

    //set line count if needed and grab file name
    if(!strcmp(argv[1], "-n")){
        lineCount  = atoi(argv[2]);
        fName = malloc(sizeof(char) * strlen(argv[3]));
        strcpy(fName,argv[3]);
    }else{
        fName = malloc(sizeof(char) * strlen(argv[1]));
        strcpy(fName, argv[1]);
    }

    //create array to hold the lines from the file
    line = malloc(sizeof(char) * lineCount*buffer);


    //open file and file descriptor
    file = fopen(fName,"r");
    fDesc = fileno(file);
  

    //read last n blocks into line array
    lseek(fDesc, 0-lineCount*buffer,SEEK_END);
    read(fDesc, line, lineCount*buffer - 1);

    int i = 0;
    int terminators = 0;
    //count # number of lines in file
    while(line[i] != EOF){
        if(line[i] == '\n'){
            terminators++;
        }
        i++;
    }
    i = 0;
    //skip to the desired stating point of lines
    while(line[i] != EOF){
        if(line[i] == '\n'){
            terminators--;
            if(terminators == lineCount -1){
                i++;
                break;
            }
        }
        i++;
    }

    char* output = malloc(sizeof(char)*((lineCount*buffer) - i));
    int j = 0;
    //put the remaining file in a new array to print to screen
    while(line[i] != EOF){
        output[j] = line[i];
        i++;
        j++;
    }
    puts(output);
    return 0;
}