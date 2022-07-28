/*****Diogo Martins Alves 86980 31/05/2019*****/
#include <stdio.h>
#include <stdlib.h>


#include "player_list.h"
#include "player.h"

/* Linked list  */
struct _player_list {
  player* this_player;
  struct _player_list *next;
};


/*initializes list*/
player_list *initPlayerList(void) {

  return NULL;
}


/*creates and returns a new node that can later be added to the list*/
player_list *createNewNodePlayerList (player_list *lp, player* new_player) {
  player_list *newNode;

  newNode = (player_list*) malloc(sizeof(player_list));
  if(newNode!=NULL) {

    newNode->this_player = new_player;
    newNode->next = lp;
    lp = newNode;
  }
  return lp;
}


/*returns the player from the list node*/
player* getPlayerFromList (player_list *lp) {

  return lp -> this_player;
}

/*returns a pointer to the next element of the list*/
player_list *getNextElementPlayerList(player_list *lp) {

  return lp -> next;
}



/*free list, including the players*/
void freePlayerList(player_list *lp) {
  player_list *aux, *newhead;  /* auxiliar pointers to travel through the list */

  for(aux = lp; aux != NULL; aux = newhead) {
    newhead = aux->next;
    free(aux->this_player);
    free(aux);
  }

  return;
}

/*returns 1 if the list is empty*/
int isEmptyPlayerList(player_list* lp)
{
  if(lp == NULL)
    return 1;
  else
    return 0;
}
