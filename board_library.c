/*****Diogo Martins Alves 86980 31/05/2019*****/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "board_library.h"


int dim_board; //dimension of the board
int n_corrects; //total number of locked cards
board_place* board = NULL;

/*translates 2D coordinates into 1D coordinate*/
int linearConv(int i, int j){
  return j*dim_board+i;
}

/*return a structure of type endgame_info containing the arguments received*/
endgame_info newEndGameInfo(int result, int points)
{
  endgame_info info;
  info.result = result;
  info.points = points;
  return info;
}

/*returns the result of a endgame_info structure*/
int getEndGameInfoResult(endgame_info info)
{
  return info.result;
}

/*returns the points of a endgame_info structure*/
int getEndGameInfoPoints(endgame_info info)
{
  return info.points;
}

/*returns the state of the card in the position (x,y)*/
int getBoardPlaceState(int x, int y)
{
  return board[linearConv(x,y)].state;
}

/*prints the board showing only the cards facing up or locked*/
void printBoardCardsUp()
{
  int i;
  printf("BOARD:\n");
  for(i = 0; i < dim_board*dim_board; i++)
  {
    if(board[i].state == STATE_UP || board[i].state == STATE_LOCKED)
      printf("| %s |", board[i].v);
    else
      printf("|   |");
    if(i%dim_board == dim_board-1)
      printf("\n");
  }
  printf("\n\n");
}

/*returns the string of the card in position (i,j)*/
char * getBoardPlaceStr(int i, int j){
  return board[linearConv(i, j)].v;
}

/*frees the memory allocated to the board*/
void closeBoard()
{
  if(board != NULL)
    free(board);
}

/*returns a copy of the board, but only with the cards facing up or locked*/
board_place* getBoardCardsFaceUp()
{
    board_place* board_cards_up = malloc(sizeof(board_place)*dim_board*dim_board);
    int j,i;
    for(i = 0; i < (dim_board*dim_board); i++)
    {
      if(board[i].state == STATE_UP || board[i].state == STATE_LOCKED)
      {
        board_cards_up[i].state = board[i].state;
        strcpy(board_cards_up[i].v, board[i].v);
        for(j = 0; j < 3;j++)
        {
          board_cards_up[i].player_color[j] = board[i].player_color[j];
        }
      }
      else if(board[i].state == STATE_DOWN)
      {
        board_cards_up[i].state = STATE_DOWN;
        strcpy(board_cards_up[i].v, "");
      }
    }
    return board_cards_up;
}

/*change card state in position (x,y) to state*/
void setBoardPlaceState(int x, int y, int state)
{
  board[linearConv(x, y)].state = state;
}

/*change card color in position (x,y)*/
void setBoardPlaceColor(int x, int y, int* color)
{
  board[linearConv(x, y)].player_color[0] = color[0];
  board[linearConv(x, y)].player_color[1] = color[1];
  board[linearConv(x, y)].player_color[2] = color[2];
}

/*initialize/reset the board*/
void initBoard(int dim){
  int count  = 0;
  int i, j;
  char * str_place;

  dim_board = dim;
  n_corrects = 0;

  if(board == NULL)
    board = malloc(sizeof(board_place)*dim*dim);

  for( i=0; i < (dim_board*dim_board); i++){
    board[i].v[0] = '\0';
    board[i].state = STATE_DOWN;
  }
  char c1, c2;
  for (c1 = 'a' ; c1 < ('a'+dim_board); c1++){
    for (c2 = 'a' ; c2 < ('a'+dim_board); c2++){
      do{
        i = random()% dim_board;
        j = random()% dim_board;
        str_place = getBoardPlaceStr(i, j);
      }while(str_place[0] != '\0');
      str_place[0] = c1;
      str_place[1] = c2;
      str_place[2] = '\0';
      do{
        i = random()% dim_board;
        j = random()% dim_board;
        str_place = getBoardPlaceStr(i, j);
      }while(str_place[0] != '\0');
      str_place[0] = c1;
      str_place[1] = c2;
      str_place[2] = '\0';
      count += 2;
      if (count == dim_board*dim_board)
        return;
    }
  }
}

/*processes a play in the board*/
play_response boardPlay(play play, player* player){

  play_response resp;
  resp.code = 10;

  if(player->state == SECOND_PICK)
  {
    //player makes a pick while waiting for his turn after a wrong picked pair
    resp.play1 = play;
    resp.code = CODE_WAITING_WRONG_PLAY_TIME_UP;
    return resp;
  }

  if(getBoardPlaceState(getXPlay(play), getYPlay(play)) == STATE_UP || getBoardPlaceState(getXPlay(play), getYPlay(play)) == STATE_LOCKED){
    if(player->state == FIRST_PICK)
    {
      //player picks an up or locked card, having made the first pick already -> hide his first pick
      resp.code = CODE_HIDE_FIRST_PLAY;
      resp.play1 = player->play1;
      setBoardPlaceState(getXPlay(resp.play1), getYPlay(resp.play1), STATE_DOWN);
      player->state = NO_PICK;
    }
    else
    {
      //player picks an up or locked card when he didn't yet make his first pick
      resp.play1 = play;
      resp.code = CODE_FILLED;
    }
  }
  else
  {
    if(player->state == NO_PICK)
    {
      //player makes his first pick
      resp.code = CODE_FIRST_PLAY;

      player->play1 = play;
      resp.play1 = play;
      strcpy(resp.str_play1, getBoardPlaceStr(getXPlay(play), getYPlay(play)));
      setBoardPlaceState(getXPlay(play), getYPlay(play), STATE_UP);
      setBoardPlaceColor(getXPlay(play), getYPlay(play), player->color);
      player->state = FIRST_PICK;
    }
    else if(player->state == FIRST_PICK)
    {
      //player makes his second pick
      char * first_str = getBoardPlaceStr(getXPlay(player->play1), getYPlay(player->play1));
      char * secnd_str = getBoardPlaceStr(getXPlay(play), getYPlay(play));

      resp.play1 = player->play1;
      strcpy(resp.str_play1, first_str);
      resp.play2 = play;
      strcpy(resp.str_play2, secnd_str);

      if (strcmp(first_str, secnd_str) == 0)
      {
        //the pair second pick and first pick is correct
        player->n_corrects += 2;
        n_corrects += 2;
        if (n_corrects == dim_board * dim_board)
          resp.code = CODE_GAME_OVER;
        else
          resp.code = CODE_SECOND_PLAY_RIGHT;

        setBoardPlaceState(getXPlay(resp.play1), getYPlay(resp.play1), STATE_LOCKED);
        setBoardPlaceState(getXPlay(resp.play2), getYPlay(resp.play2), STATE_LOCKED);
        setBoardPlaceColor(getXPlay(resp.play1), getYPlay(resp.play1), player->color);
        setBoardPlaceColor(getXPlay(resp.play2), getYPlay(resp.play2), player->color);
        player->state = NO_PICK;
      }
      else
      {
        //the pair second pick and first pick is incorrect
        resp.code = CODE_SECOND_PLAY_WRONG;
        setBoardPlaceState(getXPlay(resp.play2), getYPlay(resp.play2), STATE_UP);
        setBoardPlaceColor(getXPlay(resp.play2), getYPlay(resp.play2), player->color);
        player->state = SECOND_PICK;
      }

    }
    int i;
    for(i = 0; i < 3; i++)
      resp.player_color[i] = player->color[i]; //assign the response color to the player color
  }

  return resp;
}
