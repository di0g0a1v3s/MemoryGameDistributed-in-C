/*****Diogo Martins Alves 86980 31/05/2019*****/
#include <stdlib.h>
#include <stdio.h>
#include "player.h"

#define STEP 32

/*previous color assigned to a player*/
int previous_color[3] = {255, 0, 0};
int cycle = 0;

/*returns a new (unique) color*/
void newColor(int* r, int* g, int* b)
{
  //color sequence: 255 0 0 -> 0 255 0 -> 0 0 255 -> 255 255 0 -> 0 255 255 -> 255 0 255 -> (255-STEP) 0 0 -> 0 (255-STEP) 0 -> ... -> 254 0 0 -> ...
  if(previous_color[1] == 0 && previous_color[2] == 0)
  {
    previous_color[1] = previous_color[0];
    previous_color[0] = 0;
  }
  else if(previous_color[0] == 0 && previous_color[2] == 0)
  {
    previous_color[2] = previous_color[1];
    previous_color[1] = 0;
  }
  else if(previous_color[0] == 0 && previous_color[1] == 0)
  {
    previous_color[0] = previous_color[2];
    previous_color[1] = previous_color[2];
    previous_color[2] = 0;
  }
  else if(previous_color[2] == 0)
  {
    previous_color[2] = previous_color[0];
    previous_color[0] = 0;
  }
  else if(previous_color[0] == 0)
  {
    previous_color[0] = previous_color[1];
    previous_color[1] = 0;
  }
  else if(previous_color[1] == 0)
  {
    previous_color[0] -= STEP;
    previous_color[2] = 0;
  }

  if(previous_color[0] < 0 || previous_color[1] < 0 || previous_color[2] < 0)
  {
      cycle++;
      previous_color[0]  = 255 - cycle;
      previous_color[1] = 0;
      previous_color[2] = 0;
  }

  *(r) = previous_color[0];
  *(g) = previous_color[1];
  *(b) = previous_color[2];

}

/*constructs and returns a new player*/
player* createNewPlayer(int sock_fd)
{
  player* new_player = (player*) malloc(sizeof(player));
  new_player->sock_fd = sock_fd;
  newColor(&(new_player->color[0]), &(new_player->color[1]), &(new_player->color[2]));
  new_player->state = WAITING_TO_JOIN;
  new_player->play1 = newPlay(-1,-1);
  new_player->n_corrects = 0;

  return new_player;
}

/*resets the state, play1 and n_corrects of a player to its default values*/
void resetPlayer(player* player)
{
  if(player != NULL)
  {
    player->state = WAITING_TO_JOIN;
    player->play1 = newPlay(-1,-1);
    player->n_corrects = 0;
  }
}

/*returns the socket associated with the player*/
int getSockFd(player* player)
{
  return player->sock_fd;
}
