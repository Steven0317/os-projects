#ifndef MAIN_H_
#define MAIN_H_


int capacity = 0;
unsigned int read = 0;
unsigned int write = 0;
unsigned int events = 0;
char *mode;
char *algo;

struct node *head = NULL;
struct node *last = NULL;


//used for implementing a second chance algorithm
struct node *globalCleanHead = NULL;
struct node *globalCleanLast = NULL;
struct node *globalDirtyHead = NULL;
struct node *globalDirtyLast = NULL;
struct node *firstHead = NULL;
struct node *firstLast = NULL;
struct node *secondHead = NULL;
struct node *secondLast = NULL;

struct node{
	unsigned int address;
	char instr;
	struct node *prev, *next;
};

void printList(struct node **thisHead);

bool isEmpty();
bool isSubEmpty(struct node **thisHead);
bool isFull();
bool subQFull(struct node **thisHead);
int length();
int SubLength(struct node **thisHead);
void displayForward();
void insertFirst(unsigned int address, char instr);
void insertIntoFIFO(struct node **thisHead, struct node **thisLast, unsigned int address, char instr);
void FIFO(char * filename);
void LRU(char * filename);
void VMS(char * filename);
bool inMemory(unsigned int address);
bool inFIFO(struct node **thisHead, unsigned int address);
void moveToHead(unsigned int address);
struct node* deleteLast(unsigned int address);
struct node* removeFromFIFO(struct node **thisHead, struct node **thisLast);
void deleteNode(struct node *node);
#endif