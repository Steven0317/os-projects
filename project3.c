#define _REENTRANT
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>



typedef int bufferItem;
#define BUFFER_SIZE 5

pthread_mutex_t mutex;
sem_t full;
sem_t empty;
int counter;
bufferItem buffer[BUFFER_SIZE];

//--------------------------------------------
// insert function used by producer thread
//--------------------------------------------
void insertItem(bufferItem item) {

if (counter < BUFFER_SIZE) {
    buffer[counter] = item;
    counter++;
    return;
} else
    printf("Error inserting item");
}

//--------------------------------------------
// remove function used by consumer thread
//---------------------------------------------
bufferItem removeItem() {

if (counter > 0) {
    bufferItem itemRem;
    itemRem = buffer[counter - 1];
    counter--;
    return itemRem;
} else
    return -1;
}

//---------------------------------------------
// producer prototype
//---------------------------------------------
void *thread1() {

char newChar;
FILE *fp;
fp = fopen("mytest.dat", "r");

while (fscanf(fp, "%c", &newChar) != EOF) {

    // produce an item
    bufferItem currentChar = newChar;

    sem_wait(&empty);
    pthread_mutex_lock(&mutex);

    insertItem(currentChar);

    pthread_mutex_unlock(&mutex);
    sem_post(&full);
}
fclose(fp);
}

//---------------------------------------------
// consumer thread prototype
//----------------------------------------------
void *thread2() {
while (1) {
    sem_wait(&full);
    pthread_mutex_lock(&mutex);

    bufferItem itemPrint = removeItem();

    pthread_mutex_unlock(&mutex);
    sem_post(&empty);
    printf("%c\n", itemPrint);

    sleep(1);
}
}

//-------------------------------------------
// MAIN
//-------------------------------------------
int main() {

int r = 0;
int i;
sem_t sem1;
int shmid;              /* shared memory ID */
pthread_t tid1[1];      /* process id for thread 1 */
pthread_t tid2[1];      /* process id for thread 2 */
pthread_attr_t attr[1]; /* attribute pointer array */
pthread_mutex_init(&mutex, NULL);
sem_init(&full, 0, 0);
sem_init(&empty, 0, BUFFER_SIZE);
counter = 0;

fflush(stdout);
/* Required to schedule thread independently.*/
pthread_attr_init(&attr[0]);
pthread_attr_setscope(&attr[0], PTHREAD_SCOPE_SYSTEM);
/* end to schedule thread independently */

/* Create the threads */
pthread_create(&tid1[0], &attr[0], &thread1, NULL);
pthread_create(&tid2[0], &attr[0], &thread2, NULL);

/* Wait for the threads to finish */
pthread_join(tid1[0], NULL);
pthread_join(tid2[0], NULL);

pthread_exit(NULL);

printf("------------------------------------------------\n");
printf("\t\t End of simulation\n");

exit(0);
}
