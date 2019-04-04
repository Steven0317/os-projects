#ifndef _CUBE_H
#define _CUBE_H

#define SOLUTION
#include <semaphore.h>
#include <pthread.h>

struct cube;

/*
* thread and semaphore  declarations
*/

sem_t singleStep;
sem_t interfaceLocker;
sem_t ATeam;
sem_t BTeam;
pthread_mutex_t mutexRoom;
int aTeamFrozen;
int bTeamFrozen;

struct wizard {
	int x;
	int y;
	int id;
	char team;
	int status; /* 0: alive, 1: frozen */
	struct cube *cube;

};

struct room {
	int x;
	int y;
	struct wizard *wizards[2];

	
};

struct cube {
	int size;
	int teamA_size;
	int teamB_size;
	struct wizard **teamA_wizards;
	struct wizard **teamB_wizards;
	int game_status;

	/* Pointer to a two-dimensional array of rooms */
	struct room ***rooms;

};

extern void print_wizard(struct wizard *);
extern void incrementFCount(const struct wizard *);
extern void decrementFCount(const struct wizard *);
extern void print_cube(struct cube *);
extern int check_winner(struct cube *);


extern void dostuff();
extern struct room * choose_room(struct wizard*);
extern int try_room(struct wizard *, struct room *, struct room*);
extern void switch_rooms(struct wizard *, struct room *, struct room*);
extern struct wizard * find_opponent(struct wizard*, struct room *);
extern int fight_wizard(struct wizard *, struct wizard *, struct room *);
extern int free_wizard(struct wizard *, struct wizard *, struct room *);

#endif
