#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>

//linux path length
#define path_length 256
char* dir;

void recursiveTraverse(char* path,int spaces){
    DIR* thisDir = opendir(path);

    struct dirent* file;
    struct stat stats;

    char* fName;
    char* parent = malloc(sizeof(char)*strlen(path));
    strcpy(parent,path);
    //recursive call and read directories as they are found
    while((file = readdir(thisDir)) != NULL){
        if(!strcmp(file->d_name, ".") || !strcmp(file->d_name, "..")){continue;}

        //allocate file memory and append string for file
        fName = malloc(sizeof(char) * (strlen(file->d_name) + strlen(parent) + 1));
        strcat(fName,parent);
        strcat(fName,"/");
        strcat(fName, file->d_name);
       
        //grab stats of current file
        stat(fName, &stats);
      
        //if directory call searcha gain else just print
        if(S_ISDIR(stats.st_mode)){
            int i = 0; 
            for(i = 0; i < spaces;i++){
                printf(" ");
            }
            printf("%s \n",file->d_name);
            recursiveTraverse(fName, ++spaces);
            
        }else{
            int i = 0;
            for(i = 0; i < spaces; i++){
                printf(" ");
            }
            printf("%s \n",file->d_name);
        }
        strcpy(fName,"");
    }

    return;
}


int main(int argc, char** argv){
//get current directory and start recursion
dir = malloc(sizeof(char)*path_length);
getcwd(dir, path_length);

    if(argc == 2){
        strcpy(dir,argv[1]);
        recursiveTraverse(dir, 1);
    }else {
        recursiveTraverse(dir, 1);
    }   
exit(0);
}