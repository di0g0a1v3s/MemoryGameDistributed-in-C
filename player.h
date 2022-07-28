/*****Diogo Martins Alves 86980 31/05/2019*****/
#ifndef _PLAYER_H
#define _PLAYER_H

#include "play.h"

//possible player states
#define NO_PICK 0 //player hasn't made his first pick
#define FIRST_PICK 1 //player has made his first pick
#define SECOND_PICK 2 //player has made his second pick and is waiting to play
#define QUIT 4 //player has quit
#define WAITING_TO_JOIN 5 //player is waiting to join the game

//structure that defines a player
typedef struct _player{
  int color[3];
  int state;
  int sock_fd;
  play play1;
  int n_corrects;
}player;

player* createNewPlayer(int sock_fd);
int getSockFd(player* player);
void setServerThreadPlayer(player* player, pthread_t server_thread);
void resetPlayer(player* player);

#endif
