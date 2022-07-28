/*****Diogo Martins Alves 86980 31/05/2019*****/
#include "play.h"

int max_id_play = 0;

/*function to create a new play object*/
play newPlay(int x, int y)
{
  play new_play;
  new_play.coordinates[0] = x;
  new_play.coordinates[1] = y;
  max_id_play++;
  new_play.id = max_id_play; //the identifier is unique
  return new_play;
}

/*retrieve the x coordinate of the play*/
int getXPlay(play play)
{
  return play.coordinates[0];
}
/*retreive the y coordinate of the play*/
int getYPlay(play play)
{
  return play.coordinates[1];
}

/*returns 1 is the two plays are the same*/
int playsAreSame(play play_a, play play_b)
{
  if(play_a.id == play_b.id && play_a.coordinates[0] == play_b.coordinates[0] && play_a.coordinates[1] == play_b.coordinates[1])
    return 1;
  return 0;
}
