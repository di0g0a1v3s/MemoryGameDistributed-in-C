/*****Diogo Martins Alves 86980 31/05/2019*****/
#ifndef _PLAY_H
#define _PLAY_H

/*structure that defines a pick that can be made by a player*/
typedef struct play{
  int id;
  int coordinates[2];
}play;

play newPlay(int x, int y);
int getXPlay(play play);
int getYPlay(play play);
int playsAreSame(play play_a, play play_b);
#endif
