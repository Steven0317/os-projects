#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int main( int argc, char ** argv){
    if( argc < 2){
        printf("missing operand");
        return 1;
    }

    struct stat stats;
    stat(argv[1], &stats);

    //File Size//
    printf("Size: %d bytes\n", stats.st_size);
    
    //Block Size Allocated//
    printf("Blocks: %d blocks\n", stats.st_blocks);
    
    //Reference Count//
    printf("Reference (Link) Count: %d\n", stats.st_nlink);
    
    //File Permissions//
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
   
    //file inode//
    printf("File Inode %d", stats.st_ino);

    return 0;
}