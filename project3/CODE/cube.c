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

void 
command_line_usage()
{
  fprintf(stderr, "-size <size of cube> -teamA <size of team> -teamB <size of team> -seed <seed value>\n"); 
}

void 
kill_wizards(pthread_t *wizArr)
{
  /*
  * iterate over all wizard threads and  
  * kill thread, this should be called once a winner
  * is declared
  */
  int i;
  for(i = 0; i < numOfWizs; ++i)
  {
    pthread_cancel(wizArr[i]);
  }
}

int 
check_winner(struct cube* cube)
{
  int i, cA = 0, cB = 0;
	for (i = 0; i < cube->teamA_size; i++)
	{
		if(cube->teamA_wizards[i]->status == 1)
			cA++;
		
		if(cA == cube->teamA_size)
		{
			printf("\n\tTeam B won the game!\n");
			return 1;
		}
	}
	for (i = 0; i < cube->teamB_size; i++)
	{
		if(cube->teamB_wizards[i]->status == 1)
			cB++;
		
		if(cB == cube->teamB_size)
		{
			printf("\n\tTeam A won the game!\n");
			return 1;
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

      while((newx != x || newy != y) && !initflag)
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
  if(!initflag){
	  free(w);
	  return NULL;
    }
  }

  
  sem_init(&(w->frozenSmeaphore),0,1);

  return w;
}

int 
interface(void *cube_ref)
{
	struct cube* cube;
	char *line;
	char *command;   
	int i, j; 
	int ckwin = 0;
  

	cube = (struct cube *)cube_ref;  
	assert(cube);
  
	total = cube->teamA_size + cube->teamB_size;
	pthread_t wid[total];

	using_history();
	while (1)
    {
		sem_wait(&user);
		if(cube->game_status != 1 && check_winner(cube))
		{
			cube->game_status = 1;
			kill_wizards(wid);
			
			sem_post(&user); 
		}
			  
		line = readline("\ncube> ");
		if (line == NULL) continue;
		if (strlen(line) == 0) continue;
      
		add_history(line);

		i = 0;
		while (isspace(line[i])) i++;
		  
		command = &line[i];
		if (!strcmp(command, "exit"))
		{
			return 0;
		}
		else if (!strcmp(command, "show"))
		{
			sem_post(&user);
			print_cube(cube);
		}
		else if (!strcmp(command, "start"))  
		{
			if (cube->game_status == 1)
			{
				sem_post(&user);			  
				fprintf(stderr, "Game is over. Cannot be started again\n");
			}
			else if (cube->game_status == 0)
			{
				sem_post(&user);			  
				fprintf(stderr, "Game is in progress. Cannot be started again\n");
			}
			else
			{
				sem_post(&user);			  
				cube->game_status = 0;
			   
				for(i = 0, j = 0; i < total; i++)
				{
					if(i < cube->teamA_size)
					{
						pthread_create(&wid[i], NULL, wizard_func, cube->teamA_wizards[i]);
					}
					else
					{  
						pthread_create(&wid[i], NULL, wizard_func, cube->teamB_wizards[j]);
						j++;
					}
				}
			}
		}
		else if (!strcmp(command, "c" ))
		{
			if (cube->game_status == 1)
			{
				sem_post(&user);			  
				fprintf(stderr, "Game is over. Cannot be started again\n");
			}
			else
			{
				while(1)
				{
					if(check_winner(cube))
					{
						cube->game_status = 1;
						kill_wizards(wid);
						sem_post(&user);
						break;
					}
					
					sem_post(&wiz);
					sem_wait(&user);
				}
			}
		}
		else if (!strcmp(command, "s"))
		{
			if (cube->game_status == 1)
			{
				sem_post(&user);			  
				fprintf(stderr, "Game is over. Cannot be started again\n");
			}
			else
			{
				sem_post(&wiz);
			}
		}
		  
		else if (!strcmp(command, "stop"))
		{
			/* Stop the game */
			sem_post(&user);
			return 1;
		}
		else
		{
			sem_post(&user);
			fprintf(stderr, "unknown command %s\n", command);
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
  unsigned seed = DEFAULT_SEED;
  struct cube *cube;
  struct room *room;
  struct room **room_col;
  int res;
  struct wizard *wizard_descr;
  int i, j;


  /* Parse command line and fill:
     teamA_size, timeBsize, cube_size, and seed */

  i = 1;
  
  while(i < argc) 
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

	  /* Fill in */

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

  sem_init(&interfaceLockout,0,1);
  sem_init(&wizardLockout, 0, 0);
  /* Goes in the interface loop */
  res = interface(cube);

  exit(res);
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

  sem_getvalue(&(newroom->frozenSmeaphore), &(newroom->status));
  if(newroom->status == 0)
	{
		printf("Request denied, room locked!\n");
		sem_post(&interfaceLockout);
		sem_wait(&wizardLockout);
	  
		while(TRUE) 
		{
			/* 	Checks to see if wizard is frozen sleeps on frz until unfrozen, then sleeps
				on wiz until woken by user. checks again if frozen. */
			if(w->status == 1) 	// Frozen
			{ 
				sem_post(&interfaceLockout);
				sem_wait(&w->frozenSmeaphore);
				sem_wait(&wizardLockout);
				sem_post(&w->frozenSmeaphore);
			}
			else 				// not frozen
				break; 
		}
	  
		return 1;
	}
	else
	{
		return 0;
	}
  
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
  /*
  * TODO:
  *   Figure out what needs to be filled
  *   in here.
  */
  sem_post(&(oldroom->frozenSmeaphore));
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
  
  sem_wait(&(newroom->frozenSmeaphore));
  /* Sets self's location to current room */
  w->x = newroom->x;
  w->y = newroom->y;
}

int 
fight_wizard(struct wizard *self, struct wizard *other, struct room *room)
{
  int res;

  /* Computes the result of the fight */
  res = rand() % 2;
		  
  /* The opponent becomes frozen */
  if (res == 0)
    {
      printf("Wizard %c%d in room (%d,%d) freezes enemy %c%d\n",
	     self->team, self->id, room->x, room->y,
	     other->team, other->id);

      /*
      * set oppents status and waits on sem
      */
      other->status = 1;
      sem_wait(&(other->frozenSmeaphore));


    }

  /* Self freezes and release the lock */
  else
    {

      printf("Wizard %c%d in room (%d,%d) gets frozen by enemy %c%d\n",
	     self->team, self->id, room->x, room->y,
	     other->team, other->id);

      /*
      * Sets attackers status and waits on sem
      */
      self->status = 1;
      sem_wait(&(self->frozenSmeaphore));

    
    }
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
	     other->team, other->id);

      other->status = 0;
      sem_post(&(other->frozenSmeaphore));
      return 0;
    }

  /* The spell failed */
  printf("Wizard %c%d in room (%d,%d) fails to unfreeze friend %c%d\n",
	 self->team, self->id, room->x, room->y,
	 other->team, other->id);

  return 0;
}
