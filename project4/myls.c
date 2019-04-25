#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>


//linux max path length
#define path_length 256


//list all files in a directory;
void listFiles(char* dir){
    DIR* fdesc = opendir(dir);
    if(fdesc){
        struct dirent* current;

        while((current = readdir(fdesc)) != NULL){
            printf("%s\n", current->d_name);
        }
    }else {
        printf("failed opening directory");
        exit(1);
    }
}

//print out all permissions for a given file stats struct
void permissions(struct stat stats){

    printf("File Permission (");
    if(stats.st_mode &S_IRUSR) {
        printf("r");
    }
    else{
        printf("-");
    }
    if(stats.st_mode &S_IWUSR) {
        printf("w");
    }
    else{
        printf("-");
    }
    if(stats.st_mode &S_IXUSR) {
        printf("x");
    }
    else{
        printf("-");
    }
    if(stats.st_mode &S_IRGRP) {
        printf("r");
    }
    else{
        printf("-");
    }
    if(stats.st_mode &S_IWGRP) {
        printf("w");
    }
    else{
        printf("-");
    }
    if(stats.st_mode &S_IXGRP) {
        printf("x");
    }
    else{
        printf("-");
    }
    if(stats.st_mode &S_IROTH) {
        printf("r");
    }
    else{
        printf("-");
    }
    if(stats.st_mode &S_IWOTH) {
        printf("w");
    }
    else{
        printf("-");
    }
    if(stats.st_mode &S_IXOTH) {
        printf("x");
    }
    else{
        printf("-");
    }
    printf(")\n");
}

//print out detailed information for the given directory
void callStat(char* dir){
    DIR* fdesc = opendir(dir);
    
    if(fdesc){
        struct dirent* current;
        struct stat stats;
        struct passwd* owner;
        struct passwd* group;

        while((current = readdir(fdesc)) != NULL){
            stat(current->d_name, &stats);
            owner = getpwuid(stats.st_uid);
            group = getpwuid(stats.st_gid);

            printf("%s %s %s %ld", current->d_name, owner->pw_name, group->pw_name, stats.st_size);
            permissions(stats);
        }

    }else{
        printf("failed opening file");
        exit(1);
    }
}


int main( int argc, char** argv){

    char* dir;
    
    if(argc == 2){
        //check for detailed view flag
        if(!strcmp(argv[1], "-l")){
            //allocate memory for directory string
            dir = (char *) malloc(sizeof(char*) * path_length);
            getcwd(dir,path_length);
            
            if(dir == NULL){
                printf("directory error");
                return 1;
            }
            callStat(dir);
        }else{
            // no detailed flag param so second cli arg passed is a 
            // specific dir
            dir = (char *) malloc(sizeof(char*) * path_length);
            strcpy(dir, argv[1]);

            listFiles(dir);
        }
    }else if(argc == 3){
        //this is a detailed view on a targeted 
        // directory 
        dir = (char *) malloc(sizeof(char*) * path_length);
        //account for '-l' being out of place
        if(!strcmp(argv[1],"-l")){
            
            strcpy(dir,argv[2]);
            callStat(dir);
        }else{
            strcpy(dir,argv[1]);
            callStat(dir);
        }
    }
    else{
        // no parameters are passed
        dir = (char *) malloc(sizeof(char*) * path_length);
        getcwd(dir, path_length);
        
        if(dir == NULL){
            printf("directory error");
            return 1;
        }
        listFiles(dir);
    }


    return 0;
}