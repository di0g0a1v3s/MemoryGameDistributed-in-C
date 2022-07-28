/*****Diogo Martins Alves 86980 31/05/2019*****/
#ifndef _LIST_H
#define _LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "player.h"

/* type definition for structure to hold list item */
typedef struct _player_list player_list;


player_list *initPlayerList(void);
player_list *createNewNodePlayerList (player_list *lp, player* new_player);
player* getPlayerFromList (player_list *lp);
player_list *getNextElementPlayerList(player_list *lp);
void freePlayerList(player_list *lp);
int isEmptyPlayerList(player_list* lp);

#endif
