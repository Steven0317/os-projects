#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <assert.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "cube.h"
#include "wizard.h"

#define DEFAULT_CUBE_SIZE 4
#define DEFAULT_TEAM_SIZE 5
#define DEFAULT_SEED 1
#define HOWBUSY 100000000
#define TRUE 1
#define FALSE 0

//thread and mutex stuff
pthread_t *aTeam = NULL;
pthread_t *bTeam = NULL;



void
command_line_usage()
{
	fprintf(stderr, "-size <size of cube> -teamA <size of team> -teamB <size of team> -seed <seed value>\n");
}

/*
*	checks if the amount of frozen wizards
*	per teams is equal to team size, sets 
*	game status to complete if so or 0
*/
int
check_winner(struct cube* cube)
{
	
	int i;
	if (aTeamFrozen == cube->teamA_size) {
		cube->game_status = 1;
		printf("Team B won the game!\n");
	}
	else if (bTeamFrozen == cube->teamB_size)
	{
		cube->game_status = 1;
		printf("Team A won the game!\n");
	}

	if (cube->game_status == 1)
	{
		for (i = 0; i < cube->teamA_size + cube->teamB_size; i++)
		{
			sem_post(&singleStep);
		}
	}


	return 0;
}

void
print_cube(struct cube *cube)
{
	int i;
	int j;
	int k;

	assert(cube);

	for (i = 0; i < cube->size; i++)
	{
		printf("+");
		for (j = 0; j < cube->size; j++)
		{
			printf("--+");
		}
		printf("\n|");

		for (j = 0; j < cube->size; j++)
		{
			/* Print the status of wizards in this room here */
			for (k = 0; k < 2; k++)
			{
				if (cube->rooms[j][i]->wizards[k] != NULL)
				{
					if (cube->rooms[j][i]->wizards[k]->status)
					{
						printf("%c", tolower(cube->rooms[j][i]->wizards[k]->team));
					}
					else
					{
						printf("%c", toupper(cube->rooms[j][i]->wizards[k]->team));
					}
				}
				else
				{
					printf(" ");
				}
			}
			printf("|");
		}
		printf("\n");
	}
	printf("+");
	for (j = 0; j < cube->size; j++)
	{
		printf("--+");
	}
	printf("\n");
	return;
}

struct wizard *init_wizard(struct cube* cube, char team, int id)
{
	int x, newx;
	int y, newy;
	int initflag;
	struct wizard *w;

	w = (struct wizard *)malloc(sizeof(struct wizard));
	assert(w);

	initflag = FALSE;

	w->team = team;
	w->id = id;
	w->status = 0;
	w->cube = cube;

	x = rand() % cube->size;
	y = rand() % cube->size;

	if (cube->rooms[x][y]->wizards[0] == NULL)
	{
		cube->rooms[x][y]->wizards[0] = w;
		w->x = x;
		w->y = y;
	}
	else if (cube->rooms[x][y]->wizards[1] == NULL)
	{
		cube->rooms[x][y]->wizards[1] = w;
		w->x = x;
		w->y = y;
	}
	else
	{
		newx = (x + 1) % cube->size;
		if (newx == 0) newy = (y + 1) % cube->size;
		else newy = y;

		while ((newx != x || newy != y) && !initflag)
		{
			if (cube->rooms[newx][newy]->wizards[0] == NULL)
			{
				cube->rooms[newx][newy]->wizards[0] = w;
				w->x = newx;
				w->y = newy;
				initflag = TRUE;
			}
			else if (cube->rooms[newx][newy]->wizards[1] == NULL)
			{
				cube->rooms[newx][newy]->wizards[1] = w;
				w->x = newx;
				w->y = newy;
				initflag = TRUE;
			}
			else
			{
				newx = (newx + 1) % cube->size;

				if (newx == 0)
				{
					newy = (newy + 1) % cube->size;
				}
			}
		}
		if (!initflag) {
			free(w);
			return NULL;
		}
	}

	return w;
}


int
interface(void *cube_ref)
{
	struct cube* cube;
	char *line;
	char *command;
	int i;
	int j;

	cube = (struct cube *)cube_ref;
	assert(cube);

	using_history();
	while (TRUE)
	{
		sem_wait(&interfaceLocker);

		line = readline("cube> ");
		if (line == NULL)
		{
			sem_post(&interfaceLocker);
			continue;
		}
		if (strlen(line) == 0)
		{
			sem_post(&interfaceLocker);
			continue;
		}

		add_history(line);

		i = 0;
		while (isspace(line[i])) i++;

		command = &line[i];
		if (!strcmp(command, "exit"))
		{
			cube->game_status = 1;
			for (j = 0; j < cube->teamA_size + cube->teamB_size; j++)
			{
				sem_post(&singleStep);
			}
			return 0;
		}
		else if (!strcmp(command, "show"))
		{
			print_cube(cube);
			sem_post(&interfaceLocker);
		}
		else if (!strcmp(command, "start"))
		{
			if (cube->game_status == 1)
			{
				fprintf(stderr, "Game cannot be started again\n");
				sem_post(&interfaceLocker);

			}
			else if (cube->game_status == 0)
			{
				fprintf(stderr, "Game is in progress. Cannot be started again\n");
				sem_post(&interfaceLocker);

			}
			else
			{
				cube->game_status = 0;

			  	/*
				  *	Start game and initalize semaphores
				  * /locks
				*/
			  
				pthread_mutex_init(&mutexRoom, NULL);
				sem_init(&singleStep, 0, 0);
				sem_init(&ATeam, 0, 1);
				sem_init(&BTeam, 0, 1);
				sem_post(&interfaceLocker);


				//create the threads for team A & B
				for (j = 0; j < cube->teamA_size; j++) {
					pthread_create(&aTeam[j], NULL, wizard_func, (void*)cube->teamA_wizards[j]);
				}

				
				for (j = 0; j < cube->teamB_size; j++) {
					pthread_create(&bTeam[j], NULL, wizard_func, (void*)cube->teamB_wizards[j]);
				}
			}
		}
		else if (!strcmp(command, "s"))
		{
			if (cube->game_status == 0)
			{
				/*
				*	s command signals single step mode
				*	locking out others while waiting for 
				*	user input	
				*/
				sem_post(&singleStep);
			}
			else if (cube->game_status == 1)
			{
				fprintf(stderr, "Game cannot be started again\n");
				sem_post(&interfaceLocker);
			}
			else
			{
				fprintf(stderr, "Game not started. Enter Start.\n");
				sem_post(&interfaceLocker);
			}
		}
		else if (!strcmp(command, "c"))
		{
			if (cube->game_status == 0)
			{
				/*
				*	c command runs the game uninterrupted	
				*	until it completes
				*/
				do {
					sem_post(&singleStep);
					sem_wait(&interfaceLocker);
				} while (cube->game_status != 1);

				// return to cube interface
				sem_post(&interfaceLocker);
			}
			else if (cube->game_status == 1)
			{
				fprintf(stderr, "Game cannot be started again\n");
				sem_post(&interfaceLocker);
			}
			else
			{
				fprintf(stderr, "Game not started. Enter Start.\n");
				sem_post(&interfaceLocker);
			}
		}
		else if (!strcmp(command, "stop"))
		{
			/* Stop the game */
			return 1;
		}
		else
		{
			fprintf(stderr, "unknown command %s\n", command);
			sem_post(&interfaceLocker);

		}

		free(line);
	}

	return 0;
}

int
main(int argc, char** argv)
{
	int cube_size = DEFAULT_CUBE_SIZE;
	int teamA_size = DEFAULT_TEAM_SIZE;
	int teamB_size = DEFAULT_TEAM_SIZE;
	aTeamFrozen = 0;
	bTeamFrozen = 0;
	unsigned seed = DEFAULT_SEED;
	struct cube *cube;
	struct room *room;
	struct room **room_col;
	int res;
	struct wizard *wizard_descr;
	int i, j;

	/*
	*	threads for A & B and interface sem init
	*/

	aTeam = (pthread_t*)malloc(DEFAULT_TEAM_SIZE * sizeof(pthread_t));
	bTeam = (pthread_t*)malloc(DEFAULT_TEAM_SIZE * sizeof(pthread_t));;
	sem_init(&interfaceLocker, 0, 1);

	assert(aTeam);
	assert(bTeam);

	/* Parse command line and fill:
	   teamA_size, timeBsize, cube_size, and seed */

	i = 1;
	while (i < argc)
	{
		if (!strcmp(argv[i], "-size"))
		{
			i++;
			if (argv[i] == NULL)
			{
				fprintf(stderr, "Missing cube size\n");
				command_line_usage();
				exit(-1);
			}
			cube_size = atoi(argv[i]);
			if (cube_size == 0)
			{
				fprintf(stderr, "Illegal cube size\n");
				exit(-1);
			}
		}
		else if (!strcmp(argv[i], "-teamA"))
		{
			i++;
			if (argv[i] == NULL)
			{
				fprintf(stderr, "Missing team size\n");
				command_line_usage();
				exit(-1);
			}
			teamA_size = atoi(argv[i]);
			if (teamA_size == 0)
			{
				fprintf(stderr, "Illegal team size\n");
				exit(-1);
			}
			pthread_t *temp = realloc(aTeam, teamA_size * sizeof(pthread_t));
			aTeam = temp;
		}
		else if (!strcmp(argv[i], "-teamB"))
		{
			i++;
			if (argv[i] == NULL)
			{
				fprintf(stderr, "Missing team size\n");
				command_line_usage();
				exit(-1);
			}
			teamB_size = atoi(argv[i]);
			if (teamB_size == 0)
			{
				fprintf(stderr, "Illegal team size\n");
				exit(-1);
			}
			pthread_t *temp = realloc(bTeam, teamB_size * sizeof(pthread_t));
			bTeam = temp;
		}
		else if (!strcmp(argv[i], "-seed"))
		{
			i++;
			if (argv[i] == NULL)
			{
				fprintf(stderr, "Missing seed value\n");
				command_line_usage();
				exit(-1);
			}
			seed = atoi(argv[i]);
			if (seed == 0)
			{
				fprintf(stderr, "Illegal seed value\n");
				exit(-1);
			}
		}
		else
		{
			fprintf(stderr, "Unknown command line parameter %s\n", argv[i]);
			command_line_usage();
			exit(-1);
		}
		i++;
	}

	/* Sets the random seed */
	srand(seed);

	/* Checks that the number of wizards does not violate
	   the "max occupancy" constraint */
	if ((teamA_size + teamB_size) > ((cube_size * cube_size) * 2))
	{
		fprintf(stderr, "Sorry but there are too many wizards!\n");
		exit(1);
	}

	/* Creates the cube */
	cube = (struct cube *)malloc(sizeof(struct cube));
	assert(cube);
	cube->size = cube_size;
	cube->game_status = -1;

	/* Creates the rooms */
	cube->rooms = malloc(sizeof(struct room **) * cube_size);
	assert(cube->rooms);

	for (i = 0; i < cube_size; i++)
	{
		/* Creates a room column */
		room_col = malloc(sizeof(struct room *) * cube_size);
		assert(room_col);

		for (j = 0; j < cube_size; j++)
		{
			/* Creates a room */
			room = (struct room *)malloc(sizeof(struct room));
			assert(room);
			room->x = i;
			room->y = j;
			room->wizards[0] = NULL;
			room->wizards[1] = NULL;
			room_col[j] = room;
		}

		cube->rooms[i] = room_col;
	}

	/* Creates the wizards and positions them in the cube */
	cube->teamA_size = teamA_size;
	cube->teamA_wizards = (struct wizard **)malloc(sizeof(struct wizard *) *
		teamA_size);
	assert(cube->teamA_wizards);

	cube->teamB_size = teamB_size;
	cube->teamB_wizards = (struct wizard **)malloc(sizeof(struct wizard *) *
		teamB_size);

	assert(cube->teamB_wizards);

	/* Team A */
	for (i = 0; i < teamA_size; i++)
	{
		if ((wizard_descr = init_wizard(cube, 'A', i)) == NULL)
		{
			fprintf(stderr, "Wizard initialization failed (Team A number %d)\n", i);
			exit(1);
		}
		cube->teamA_wizards[i] = wizard_descr;
	}

	/* Team B */

	for (i = 0; i < teamB_size; i++)
	{
		if ((wizard_descr = init_wizard(cube, 'B', i)) == NULL)
		{
			fprintf(stderr, "Wizard initialization failed (Team B number %d)\n", i);
			exit(1);
		}
		cube->teamB_wizards[i] = wizard_descr;
	}


	/* Goes in the interface loop */
	res = interface(cube);

	/*
	*	thread clean up and deallocation 
	*	of memory segments
	*/
	for (i = 0; i < cube->teamA_size; i++) {
		pthread_join(aTeam[i], NULL);
	}

	for (i = 0; i < cube->teamB_size; i++) {
		pthread_join(bTeam[i], NULL);
	}

	free(aTeam);
	free(bTeam);
	pthread_mutex_destroy(&mutexRoom);
	exit(res);
}
/*
*	increments cunt of frozen wizards this needs to be atomic
*	since these values are read by a different thread in the game 
*	check win condition 
*/
void 
incrementFCount(const struct wizard * wiz)
{

	if (tolower(wiz->team) == 'a')
	{
		sem_wait(&ATeam);
		aTeamFrozen++;
		sem_post(&ATeam);
	}
	else
	{
		sem_wait(&BTeam);
		bTeamFrozen++;
		sem_post(&BTeam);
	}

}

/*
*	decrements the count value, same principles
*	from above apply here
*/
void 
decrementFCount(const struct wizard * wiz)
{

	if (tolower(wiz->team) == 'a')
	{
		sem_wait(&ATeam);
		aTeamFrozen--;
		sem_post(&ATeam);
	}
	else
	{
		sem_wait(&BTeam);
		bTeamFrozen--;
		sem_post(&BTeam);
	}

}


void dostuff()
{
	int i;
	int wait;

	wait = rand() % HOWBUSY;

	for (i = 0; i < wait; i++) {}

	return;
}

struct room *
	choose_room(struct wizard* w)
{
	int newx = 0;
	int newy = 0;

	/* The two values cannot be both 0 - no move - or 1 - diagonal move */
	while (newx == newy)
	{
		newx = rand() % 2;
		newy = rand() % 2;
	}
	if ((rand() % 2) == 1) {
		newx = 0 - newx;
		newy = 0 - newy;
	}

	return w->cube->rooms[(w->x + w->cube->size + newx) % w->cube->size][(w->y + w->cube->size + newy) % w->cube->size];
}

int
try_room(struct wizard *w, struct room *oldroom, struct room* newroom)
{
	/*
	* check both spots of a room to find a spot
	*/
	int i;
	for (i = 0; i < 2; i++)
	{
		if(newroom->wizards[i] == NULL)
		{	
			return 0;
		}
	}

	return 1;
}

struct wizard *
	find_opponent(struct wizard* self, struct room *room)
{
	struct wizard *other = NULL;

	/* Updates room wizards and determines opponent */
	if ((room->wizards[0] == self))
	{
		other = room->wizards[1];
	}
	else if (room->wizards[1] == self)
	{
		other = room->wizards[0];
	}

	return other;
}

void
switch_rooms(struct wizard *w, struct room *oldroom, struct room* newroom)
{
	struct wizard *other;

	/* Removes self from old room */
	if (oldroom->wizards[0] == w)
	{
		oldroom->wizards[0] = NULL;
	}
	else if (oldroom->wizards[1] == w)
	{
		oldroom->wizards[1] = NULL;
	}
	else /* This should never happen */
	{
		printf("Wizard %c%d in room (%d,%d) can't find self!\n",
			w->team, w->id, oldroom->x, oldroom->y);
		print_cube(w->cube);
		exit(1);
	}

	/* Updates room wizards and determines opponent */
	if (newroom->wizards[0] == NULL)
	{
		newroom->wizards[0] = w;
		other = newroom->wizards[1];
	}
	else if (newroom->wizards[1] == NULL)
	{
		newroom->wizards[1] = w;
		other = newroom->wizards[0];
	}
	else /* This should never happen */
	{
		printf("Wizard %c%d in room (%d,%d) gets in a room already filled with people!\n",
			w->team, w->id, newroom->x, newroom->y);
		print_cube(w->cube);
		exit(1);
	}

	/* Sets self's location to current room */
	w->x = newroom->x;
	w->y = newroom->y;
}

int
fight_wizard(struct wizard *self, struct wizard *other, struct room *room)
{
	int res;
	int i;

	/* Computes the result of the fight */
	res = rand() % 2;

	/* The opponent becomes frozen */
	if (res == 0)
	{
		printf("Wizard %c%d in room (%d,%d) freezes enemy %c%d\n",
			self->team, self->id, room->x, room->y,
			other->team, other->id);

		
		other->status = 1;
		other->team = tolower(other->team);
		incrementFCount(other);
	}

	/* Self freezes and release the lock */
	else
	{
		printf("Wizard %c%d in room (%d,%d) gets frozen by enemy %c%d\n",
			self->team, self->id, room->x, room->y,
			other->team, other->id);


		self->status = 1;
		self->team = tolower(self->team);
		incrementFCount(self);

	}

	check_winner(self->cube);


	return 0;
}

int
free_wizard(struct wizard *self, struct wizard *other, struct room* room)
{
	int res;

	/* Computes the results of the unfreeze spell */
	res = rand() % 2;

	/* The friend is unfrozen */
	if (res == 0)
	{
		printf("Wizard %c%d in room (%d,%d) unfreezes friend %c%d\n",
			self->team, self->id, room->x, room->y,
			toupper(other->team), other->id);

		/*
		* reset status and decrease frozen counter
		*/
		other->status = 0;
		other->team = toupper(other->team);
		decrementFCount(other);
	}
	else
	{
		/* The spell failed */
		printf("Wizard %c%d in room (%d,%d) fails to unfreeze friend %c%d\n",
			self->team, self->id, room->x, room->y,
			toupper(other->team), other->id);
	}

	return 0;
}
