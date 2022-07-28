/*****Diogo Martins Alves 86980 31/05/2019*****/
#include <stdlib.h>
#include "player.h"
#include "play.h"
//board_place states
#define STATE_DOWN 0
#define STATE_UP 1
#define STATE_LOCKED 2
//endgame results
#define RESULT_WIN 0
#define RESULT_LOST 1
//play_response codes
#define CODE_FILLED 0
#define CODE_FIRST_PLAY 1
#define CODE_SECOND_PLAY_RIGHT 2
#define CODE_GAME_OVER 3
#define CODE_SECOND_PLAY_WRONG -2
#define CODE_HIDE_FIRST_PLAY -3
#define CODE_HIDE_BOTH_PLAYS -4
#define CODE_WAITING_WRONG_PLAY_TIME_UP -5
#define CODE_PERMISSION_TO_LEAVE -6
#define CODE_GAME_FROZEN -7
#define CODE_GAME_UNFROZEN -8

//structure containing information about the game score
typedef struct endgame_info
{
  int result;
  int points;
}endgame_info;

//structure that represents one card
typedef struct board_place{
  char v[3]; //card string
  int state; //state of the card
  int player_color[3]; //color of the player who currently owns the card
} board_place;

//structure used by the server to make a response to the clients
typedef struct play_response{
  int code; //code of the response
  play play1; //first player pick
  play play2; //second player pick
  char str_play1[3], str_play2[3]; //cards strings
  int player_color[3]; //color of the player who played
} play_response;

char *getBoardPlaceStr(int i, int j);
void initBoard(int dim);
play_response boardPlay(play play, player* player);
board_place* getBoardCardsFaceUp();
void setBoardPlaceState(int x, int y, int state);
void setBoardPlaceColor(int x, int y, int* color);
void printBoardCardsUp();
int getBoardPlaceState(int x, int y);
endgame_info newEndGameInfo(int result, int points);
int getEndGameInfoPoints(endgame_info info);
int getEndGameInfoResult(endgame_info info);
void closeBoard();
