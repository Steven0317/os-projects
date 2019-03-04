#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "main.h"


int main(int argc, char **argv)
{
/*input: <tracefile> <nframes> <lru|fifo|vms> <debug|quiet> */
if(argc < 5){
	printf("Insufficent argument count");
	return 1;
}

capacity = strtol(argv[2], NULL, 10);
mode = argv[4];
algo = argv[3];

if(strcmp(algo,"fifo") == 0){
	printf("Running a FIFO Cache\n");
	FIFO(argv[1]);
}
if(strcmp(algo, "lru") == 0){
	printf("Running a LRU Cache\n");
	LRU(argv[1]);
}
if(strcmp(algo, "vms") == 0) { 
	printf("Running Second Chance algorithm\n");
	VMS(argv[1]);
}
return 0;
}


/*
*	First In First Out Implementation
*   This is a basic queue that will kick 
*	out the oldest memory reference in the
*	queue, which is always the last node
*	in this particular implementation
*/
void FIFO(char * filename) {
	
	FILE *fp;
	unsigned int address;
	char instr;
	fp = fopen(filename, "r");
	
	if(fp == NULL){
		perror("Failed: ");
		exit(1);
	}
	
	while(fscanf(fp, "%x %c", &address, &instr) != EOF)
	{
		/*
		*	the last node in the list is the first node out in this FIFO
		*	cache, each node is inserted into the front and then we drop 
		*	the last node whenever the list is full and we need to replace
		*/
		
		if(!inMemory(address)){
			
			if(isFull()) {
				struct node *delete = deleteLast(address);
				free(delete);
			}
			insertFirst(address,instr);
			// increment when we have to read or 
			// since frame is not in memory
			read++;
			
			if(strcmp(mode,"debug") == 0){
				printf("%x  address moved into memory\n", address);
			}	
		}
		
		events++;
		
	}
	printf("Memory Frames %d\n", capacity);
	printf("Events in Trace %d\n", events);
	printf("Total Disk Writes %d\n", write);
	printf("Total Disk Reads: %d\n", read);
	
	
	
}


void LRU(char * filename){
	
	FILE *fp;
	unsigned int address;
	char instr;
	fp = fopen(filename, "r");
	
	if(fp == NULL){
		perror("Failed: ");
		exit(1);
	}
	
	while(fscanf(fp, "%x %c", &address, &instr) != EOF)
	{
		/*
		*	The last node in this list  will be the oldest
		*	referenced frame and thus the first to be kicked 
		*   out of the list and replaced, each time an in memory
		*   page is referenced it is moved back to front of the list
		*/
		if(!inMemory(address)){
			
			if(isFull()) {
				struct node *delete = deleteLast(address);
				free(delete);
			}
			
			insertFirst(address,instr);
			// increment when we have to read from disk
			// since frame is not in memory
			read++;

			if(strcmp(mode,"debug") == 0) {
				printf("%x  address moved into memory\n", address);
			}	
		}
		else{
			// locality reference move
			// frame back to head of list
			
			moveToHead(address);
		}
		
		events++;
	}
	printf("Memory Frames %d\n", capacity);
	printf("Events in Trace %d\n", events);
	printf("Total Disk Writes %d\n", write);
	printf("Total Disk Reads: %d\n", read);

	
}

void VMS(char * filename){
	FILE *fp;
	unsigned int address;
	char instr;
	fp = fopen(filename, "r");
	
	if(fp == NULL){
		perror("Failed: ");
		exit(1);
	}
	int i = 0;
	while(fscanf(fp, "%x %c", &address, &instr) != EOF)
	{	i++;
		if(i == 8)break;
		//determine which process the address belongs to
		int temp = (int) address;
		char num[10];
		itoa(address,num,16);
		
		/*
		*	two queues held for each contending process 
		*	first time evicted pages are placed in either a global 
		*	clean or dirty list that is of size nFrames/2 + 1, if a frame
		*	is needed the clean/ dirty pages are evicted to make room
		*/
		
		//new page reference not in sub lists
		if(!inFIFO(&firstHead,address) || !inFIFO(&secondHead,address)) {
			
			//if not in sub lists but in globals lists
			//remove from global and place back in sub list;
			if(inFIFO(&globalCleanHead,address) || inFIFO(&globalDirtyHead,address)) {
				
				if(inFIFO(&globalCleanHead,address)) {
					struct node *secChance = removeFromFIFO(&globalCleanHead, &globalCleanLast);
					//check if sub lists are full
					if(num[0] = '3') {
						//remove from first sub list;
						if(subQFull(&firstHead)) {
							struct node *intoGlobal = removeFromFIFO(&firstHead, &firstLast);
							insertIntoFIFO(&globalCleanHead, &globalCleanLast, intoGlobal->address, intoGlobal->instr);
							insertIntoFIFO(&firstHead,&firstLast, secChance->address, secChance->instr);
							free(secChance);
							free(intoGlobal);
						} else {
							insertIntoFIFO(&firstHead,&firstLast, secChance->address, secChance->instr);
							free(secChance);
						}
					} else {
						//remove from second sub list
						if(subQFull(&secondHead)) {
							struct node *intoGlobal = removeFromFIFO(&secondHead, &secondLast);
							insertIntoFIFO(&globalCleanHead, &globalCleanLast, intoGlobal->address, intoGlobal->instr);
							insertIntoFIFO(&secondHead,&secondLast, secChance->address, secChance->instr);
							free(secChance);
							free(intoGlobal);
						} else {
							insertIntoFIFO(&firstHead,&secondLast, secChance->address, secChance->instr);
							free(secChance);
						}
					}
				}else {
					struct node *secChance = removeFromFIFO(&globalDirtyHead,&globalDirtyLast);
					//check if sub lists are full
					if(num[0] = '3') {
						//remove from first sub list;
						if(subQFull(&firstHead)) {
							struct node *intoGlobal = removeFromFIFO(&firstHead, &firstLast);
							insertIntoFIFO(&globalCleanHead, &globalCleanLast, intoGlobal->address, intoGlobal->instr);
							insertIntoFIFO(&firstHead,&firstLast, secChance->address, secChance->instr);
							free(secChance);
							free(intoGlobal);
						} else {
							insertIntoFIFO(&firstHead,&firstLast, secChance->address, secChance->instr);
							free(secChance);
						}
					} else {
						//remove from second sub list
						if(subQFull(&secondHead)) {
							struct node *intoGlobal = removeFromFIFO(&secondHead, &secondLast);
							insertIntoFIFO(&globalCleanHead, &globalCleanLast, intoGlobal->address, intoGlobal->instr);
							insertIntoFIFO(&secondHead,&secondLast, secChance->address, secChance->instr);
							free(secChance);
							free(intoGlobal);
						} else {
							insertIntoFIFO(&firstHead,&secondLast, secChance->address, secChance->instr);
							free(secChance);
						}
					}
				}
			}

			// address is not in global either so it must be put into
			// appropriate sublist while maintaining appropraite buffers
			else {
				
				if(num[0] == '3') {
						if(isFull()) {
						struct node * intoGlobal = removeFromFIFO(&firstHead, &firstLast);
						//move from sublist into global
						//check if global is full, evicting
						//from main memory and global if need be
						if(subQFull(&firstHead)) {
							if(intoGlobal->instr == 'R') {
								if(subQFull(&globalCleanHead)) {
									struct node * tbd = removeFromFIFO(&globalCleanHead,&globalCleanLast);
									deleteNode(tbd);
									insertIntoFIFO(&globalCleanHead, &globalCleanLast, intoGlobal->address, intoGlobal->instr);
									free(intoGlobal);
								}else {
									insertIntoFIFO(&globalCleanHead, &globalCleanLast, intoGlobal->address, intoGlobal->instr);
									free(intoGlobal);
								}

								insertFirst(address,instr);
							}else {
								if(subQFull(&globalDirtyHead)) {
									struct node * tbd = removeFromFIFO(&globalDirtyHead,&globalDirtyLast);
									deleteNode(tbd);
									insertIntoFIFO(&globalDirtyHead, &globalDirtyLast, intoGlobal->address, intoGlobal->instr);
									free(intoGlobal);
								} else {
									insertIntoFIFO(&globalDirtyHead, &globalDirtyLast, intoGlobal->address, intoGlobal->instr);
									free(intoGlobal);
								}

								insertFirst(address,instr);
							}
						}else {
							//insert new reference into main memory and sublist
							insertFirst(address,instr);
							insertIntoFIFO(&firstHead, &firstLast, intoGlobal->address, intoGlobal->instr);
							free(intoGlobal);
						}
					} else {
						struct node * intoGlobal = removeFromFIFO(&secondHead, &secondLast);
						//move from sublist into global
						//check if global is full, evicting
						//from main memory and global if need be
						if(subQFull(&secondHead)) {
							if(intoGlobal->instr == 'R') {
								if(subQFull(&globalCleanHead)) {
									struct node * tbd = removeFromFIFO(&globalCleanHead,&globalCleanLast);
									deleteNode(tbd);
									insertIntoFIFO(&globalCleanHead, &globalCleanLast, intoGlobal->address, intoGlobal->instr);
									free(intoGlobal);
								}else {
									insertIntoFIFO(&globalCleanHead, &globalCleanLast, intoGlobal->address, intoGlobal->instr);
									free(intoGlobal);
								}
							}else {
								if(subQFull(&globalDirtyHead)) {
									struct node * tbd = removeFromFIFO(&globalDirtyHead,&globalDirtyLast);
									deleteNode(tbd);
									insertIntoFIFO(&globalDirtyHead, &globalDirtyLast, intoGlobal->address, intoGlobal->instr);
									free(intoGlobal);
								} else {
									insertIntoFIFO(&globalDirtyHead, &globalDirtyLast, intoGlobal->address, intoGlobal->instr);
									free(intoGlobal);
								}
							}
						}else {
							insertIntoFIFO(&secondHead, &secondLast, intoGlobal->address, intoGlobal->instr);
							free(intoGlobal);
						}
					}
				}
			}

			//put address in memory space and sublists
			if(!isFull() && !inFIFO(&firstHead, address) && !inFIFO(&secondHead, address)) {
				
				//check if sub lists have space
				if(num[0] == '3') {
					if(subQFull(&firstHead)) {
						//if clean global has element remove that first
						//else remove element from dirty
						if(!isSubEmpty(&globalCleanHead)) {
							struct node * tbd = removeFromFIFO(&globalCleanHead, &globalCleanLast);
							deleteNode(tbd);
							insertFirst(address,instr);
							insertIntoFIFO(&firstHead, &firstLast, address, instr);
						} else {
							struct node * tbd = removeFromFIFO(&globalDirtyHead, &globalDirtyLast);
							deleteNode(tbd);
							insertFirst(address,instr);
							insertIntoFIFO(&firstHead, &firstLast, address, instr);
						}
					//there is space in first sub list
					}else {
						insertFirst(address,instr);
						insertIntoFIFO(&firstHead, &firstLast, address, instr);
					}
				} else {
					if(subQFull(&secondHead)) {
						//if clean global has element remove that first
						//else remove element from dirty
						if(!isSubEmpty(&globalCleanHead)) {
							struct node * tbd = removeFromFIFO(&globalCleanHead, &globalCleanLast);
							deleteNode(tbd);
							insertFirst(address,instr);
							insertIntoFIFO(&firstHead, &firstLast, address, instr);
						} else {
							struct node * tbd = removeFromFIFO(&globalDirtyHead, &globalDirtyLast);
							deleteNode(tbd);
							insertFirst(address,instr);
							insertIntoFIFO(&secondHead, &secondLast, address, instr);
						}
					//there is space in second sub list
					}else {
						insertFirst(address,instr);
						insertIntoFIFO(&secondHead, &secondLast, address, instr);
					}
				}
			}
		}
		
		events++;
		
	}
	printf("Memory Frames %d\n", capacity);
	printf("Events in Trace %d\n", events);
	printf("Total Disk Writes %d\n", write);
	printf("Total Disk Reads: %d\n", read);
	
	printList(&secondHead);
	printList(&globalCleanHead);
}

void printList(struct node **thisHead){
	
	printf("\n[ ");
	struct node *current = *thisHead;
	while(current != NULL) {
		printf(" (%x %c) ", current->address, current->instr);
		current = current->next;
	}
	printf(" ]\n");
}

/*
* Everything below is for the 
* linked list DS, including helper functions
* this is as abstract as possible in order to 
* use for all three algorithms, use of a linked list 
* allows for faster/cleaner insertions and deletions
* over an array
*/

//check if list is empty
bool isEmpty() {
   return head == NULL;
}
//check if sublist is empty
bool isSubEmpty(struct node **thisHead) {
	return *thisHead == NULL;
}
//standard memory cache length
bool isFull() {
	return length() == capacity;
}
//sub queue length vms
bool subQFull(struct node **thisHead){
	return SubLength(thisHead) == capacity/2; 
}
//check if address is in memory
bool inMemory(unsigned int address){
	struct node *current;
	bool flag = false;
	for(current = head; current != NULL; current = current->next){
		if(current->address == address)
			flag = true;
	}
	return flag;
}
//determine if address is in a sub list
bool inFIFO(struct node **thisHead, unsigned int address) {
	struct node *current;
	bool flag = false;
	for(current = *thisHead; current != NULL; current = current->next){
		if(current->address == address)
			flag = true;
	}
	return flag;
}
//return the length of the  memory list
int length() {
   int length = 0;
   struct node *current;
	
   for(current = head; current != NULL; current = current->next){
      length++;
   }
   
   return length;
}
//return length of specified list
int SubLength(struct node **thisHead){
	int length = 0;
	struct node *current = *thisHead;
	
	while(current != NULL) {
		length++;
		current = current->next;
	}
   
   return length;
   
}
//move a specified address node to the head of a list
void moveToHead(unsigned int address){
	
	struct node *current = head;
	
	
	while(current != NULL){
		/*
		*	find the node that has the referenced address
		*   and move it to the front of the list
		*/
	
		
		if(current->address == address){
			struct node *temp = current;
			if(strcmp(mode, "debug") == 0) {
				printf("%x found in cache, moving to head node\n", current->address);
				
			}
			if(current != head) {

				if(current == last) {
					last = temp->prev;
					last->next = NULL;
					head->prev = current;
					current->next = head;
					current->prev = NULL;
					head = current;
				}else {
					current->prev->next = temp->next;
					temp->next->prev = current->prev;
					head->prev = current;
					current->next = head;
					current->prev = NULL;
					head = current;
				}

			}
		}
			
		current = current->next;	
	}
	
}

// deletes a specified node
void deleteNode(struct node *node){
	struct node *current = head;
	
	if(node->instr == 'W'){
		write++;
	}
	// current node will point to the deleted node
	while(current != NULL) {
		if(current->address == node->address)
			break;
	}
	if(current == head) {
		head->next->prev = NULL;
		head = current->next;
		free(node);
	}
	else if(current == last) {
		last = current->prev;
		last->next = NULL;
		free(node);
	}
	else if(current != head && current != last) {
		current->prev->next = node->next;
		current->next->prev = node->prev;
		free(node);
	}

}

//printer utility
void displayForward() {

   
   struct node *ptr = head;
	
   printf("\n[ ");
	
   while(ptr != NULL) {        
      printf("(%x,%c) ",ptr->address,ptr->instr);
      ptr = ptr->next;
   }
	
   printf(" ]");
}
//insert nodes to head of list
void insertFirst(unsigned int address, char instr) {

   struct node *link = (struct node*) malloc(sizeof(struct node));
   link->address = address;
   link->instr = instr;
	
   if(isEmpty()) {
      last = link;
   } else {
      head->prev = link;
   }

   link->next = head;

   head = link;
}

void insertIntoFIFO(struct node **thisHead, struct node **thisLast, unsigned int address, char instr) {
	
	struct node *link = (struct node*)malloc(sizeof(struct node));
	link->address = address;
	link->instr = instr;
	if(isSubEmpty(thisHead)) {
		*thisLast = link;
	} else {
		(**thisHead).prev = link;
	}
		link->next = *thisHead;
		
		*thisHead = link;
	
}
//delete tail node in main list(fifo & lru)
struct node *deleteLast(unsigned int address) {
   
	   struct node *tempLink = last;
		if(head->next == NULL) {
			head = NULL;
		} else {
			last->prev->next = NULL;
		}
		last = last->prev;
		
		if(strcmp(mode,"debug") == 0){
			printf("%x address moved out of memory\n", tempLink->address);
		}
		//increment writing to disk
		// on deletion frame
		if(tempLink->instr == 'W'){
			write++;
		}
		
		return tempLink;	
   
}

struct node *removeFromFIFO(struct node **thisHead, struct node **thisLast) {

	struct node *tempLink = *thisLast;	
	if((**thisHead).next == NULL) {
		*thisHead = NULL;
	} else {
		(**thisLast).prev->next = NULL;
	}
	*thisLast = (**thisLast).prev;
	
	if(strcmp(mode,"debug") == 0){
		printf("%x address moved out of memory\n", tempLink->address);
	}
	
	// return the to be deleted node
	// this is important forvms, fifo 
	// and lru just delete it upon return
	return tempLink;	
		
}
