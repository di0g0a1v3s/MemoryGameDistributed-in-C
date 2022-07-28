/*****Diogo Martins Alves 86980 31/05/2019*****/
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <signal.h>

#include "board_library.h"
#include "play.h"
#include "memory_defs.h"

//possible game states
#define GAME_ON 1
#define GAME_OVER 2
#define GAME_QUIT 3

int sock_fd;
int game_state;
int board_dim;
board_place* board;

/*translates 2D coordinates into 1D coordinate*/
int linConv(int i, int j){
  return j*board_dim+i;
}

/*function used to make the client quit the game*/
void quitClient()
{
  //send the play (-1,-1), requesting to leave
  printf("\nQUITTING...\n");
  play play = newPlay(-1,-1);
  int ret = write(sock_fd, &play, sizeof(play));
  //if the game is over or there is an error in the write, the client doesn't wait for the server reply to leave
  if(game_state == GAME_OVER || ret == -1)
  {
    game_state = GAME_QUIT;
  }
}

/*function that terminates execution of the client (when Ctrl+c is pushed)*/
void quitHandler(int dummy)
{
  quitClient();
}

/*updates the board stored internally with all the known cards*/
void updateInternalBoard(play_response resp)
{
  switch (resp.code) {
    case CODE_FIRST_PLAY:
      board[linConv(getXPlay(resp.play1), getYPlay(resp.play1))].state = STATE_UP; //update state
      if(strlen(board[linConv(getXPlay(resp.play1), getYPlay(resp.play1))].v) != 2)
        strcpy(board[linConv(getXPlay(resp.play1), getYPlay(resp.play1))].v, resp.str_play1); //update string
      break;
    case CODE_SECOND_PLAY_RIGHT:
    case CODE_GAME_OVER:
      board[linConv(getXPlay(resp.play1), getYPlay(resp.play1))].state = STATE_LOCKED;
      if(strlen(board[linConv(getXPlay(resp.play1), getYPlay(resp.play1))].v) != 2)
        strcpy(board[linConv(getXPlay(resp.play1), getYPlay(resp.play1))].v, resp.str_play1);
      board[linConv(getXPlay(resp.play2), getYPlay(resp.play2))].state = STATE_LOCKED;
      if(strlen(board[linConv(getXPlay(resp.play2), getYPlay(resp.play2))].v) != 2)
        strcpy(board[linConv(getXPlay(resp.play2), getYPlay(resp.play2))].v, resp.str_play2);
      break;
    case CODE_SECOND_PLAY_WRONG:
      board[linConv(getXPlay(resp.play1), getYPlay(resp.play1))].state = STATE_UP;
      if(strlen(board[linConv(getXPlay(resp.play1), getYPlay(resp.play1))].v) != 2)
        strcpy(board[linConv(getXPlay(resp.play1), getYPlay(resp.play1))].v, resp.str_play1);
      board[linConv(getXPlay(resp.play2), getYPlay(resp.play2))].state = STATE_UP;
      if(strlen(board[linConv(getXPlay(resp.play2), getYPlay(resp.play2))].v) != 2)
        strcpy(board[linConv(getXPlay(resp.play2), getYPlay(resp.play2))].v, resp.str_play2);
      break;
    case CODE_HIDE_FIRST_PLAY:
      board[linConv(getXPlay(resp.play1), getYPlay(resp.play1))].state = STATE_DOWN;
      break;
    case CODE_HIDE_BOTH_PLAYS:
      board[linConv(getXPlay(resp.play1), getYPlay(resp.play1))].state = STATE_DOWN;
      board[linConv(getXPlay(resp.play2), getYPlay(resp.play2))].state = STATE_DOWN;
      break;
  }
}

/*finds and returns 1 if there is a pair of DOWN cards that match*/
int findPair(play* play1, play* play2)
{
  int i,j;
  for(i = 0; i < board_dim*board_dim; i++)
  {
    for(j = 0; j < board_dim*board_dim; j++)
    {
      //searches the internal board for two DOWN cards with matching strings
      if(i!=j && board[i].state == STATE_DOWN && board[j].state == STATE_DOWN && strcmp(board[i].v,board[j].v) == 0 && strlen(board[i].v) == 2)
      {
        //on success, creates two plays and returns 1
        *play1 = newPlay(i%board_dim, i/board_dim);
        *play2 = newPlay(j%board_dim, j/board_dim);
        return 1;
      }
    }
  }
  return 0; //on failure, returns 0
}

/*finds and returns 1 if there is an unknown DOWN card*/
int firstUnknown(play* play)
{
  int i;
  for(i = 0; i < board_dim*board_dim; i++)
  {
    //searches the internal board for a DOWN card, whose string is not known
    if(board[i].state == STATE_DOWN && strlen(board[i].v) != 2)
    {
      //create a play and return 1 on success
      *play = newPlay(i%board_dim, i/board_dim);
      return 1;
    }
  }
  return 0; //on failure, returns 0
}


/*routine that runs in a thread and constantly sends plays to the server, until the client quits*/
void* sendPlaysRoutine(void* args)
{
  play play1, play2;
	while (game_state != GAME_QUIT)
  {
    if(game_state == GAME_ON)
    {
      //try to find a matching pair
      if(findPair(&play1, &play2) == 1)
      {
        //send first pick
          int ret = write(sock_fd, &play1, sizeof(play));
          if(ret == -1)
          {
            quitClient();
            continue;
          }
          usleep(250000); //wait 0.25 sec between each pick
          //send second pick
          ret = write(sock_fd, &play2, sizeof(play));
          if(ret == -1)
          {
            quitClient();
            continue;
          }
      }
      else if(firstUnknown(&play1) == 1)
      {
          //if no pair is found, the fist pick will be an unknown card and the second pick will be a random card
          int ret = write(sock_fd, &play1, sizeof(play));
          if(ret == -1)
          {
            quitClient();
            continue;
          }
          usleep(250000); //wait 0.25 sec between each pick
          //second pick is random
          play2 = newPlay(rand()%board_dim, rand()%board_dim);
          ret = write(sock_fd, &play2, sizeof(play));
          if(ret == -1)
          {
            quitClient();
            continue;
          }
      }
      usleep(250000); //wait 0.25 sec between each pick
    }

	}


}

/*routine that runs in a thread and constantly receives responses from the server, until the client quits or the game is over*/
void* receiveResponsesRoutine(void* args)
{
  play_response resp;
  while(game_state == GAME_ON)
  {
    int ret = read(sock_fd, &resp, sizeof(play_response));
    if(ret == sizeof(play_response))
    {
      //when a valid reply is received, update the internal board
      updateInternalBoard(resp);
      if(resp.code == CODE_GAME_OVER)
        game_state = GAME_OVER; //game over
      else if(resp.code == CODE_PERMISSION_TO_LEAVE)
      {
        game_state = GAME_QUIT; //quit game
      }
      else if(resp.code == CODE_GAME_FROZEN)
        printf("GAME FROZEN\n"); //game frozen
      else if(resp.code == CODE_GAME_UNFROZEN)
        printf("GAME UNFROZEN\n"); //game unfrozen
    }
    else if(ret == -1)
      quitClient(); //quit if there is an error reading from server

  }
}


int main(int argc, char * argv[]){

  struct sockaddr_in server_addr;
  srand(time(NULL) + getpid());


  //second argument must be server address
  if (argc < 2){
    printf("Second argument should be server address\n");
    exit(-1);
  }


  //create the socket
  sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd == -1){
    perror("socket: ");
    exit(-1);
  }
  //connect to server
  server_addr.sin_family = AF_INET;
	server_addr.sin_port= htons(MEMORY_PORT);
	inet_aton(argv[1], &server_addr.sin_addr);
  if(connect(sock_fd,
			(const struct sockaddr *) &server_addr,
			sizeof(server_addr)) == -1){
				printf("Error connecting\n");
				exit(-1);
	}
  printf("Successfully connected to server\n");

  //define the function to be executed when ctrl+c is pushed
  signal(SIGINT, quitHandler);
  //receive the board dimension from the server
  int ret = read(sock_fd, &board_dim, sizeof(int));
  if(ret != sizeof(int) || board_dim <= 0)
    exit(0);
  printf("Board dimension: %d\n", board_dim);

  //alloc memory for the board with cards face up
  board_place* board_cards_face_up = (board_place*)malloc(sizeof(board_place)*board_dim*board_dim);

  game_state = GAME_OVER;

  //create thread to send plays to server
  pthread_t send_plays_thread;
  pthread_create(&send_plays_thread, NULL, sendPlaysRoutine, NULL);

  while(game_state != GAME_QUIT)
  {
    printf("Wait for game to start...\n");
    //receive the board with the cards face up or locked
    int ret = read(sock_fd, board_cards_face_up, sizeof(board_place)*board_dim*board_dim);
    if(ret == sizeof(board_place)*board_dim*board_dim)
    {
      printf("GAME ON\n");
      game_state = GAME_ON;
      //use the board received as the internal board
      board = board_cards_face_up;

      //create a thread to receive the responses from the server
      pthread_t receive_responses_thread;
      pthread_create(&receive_responses_thread, NULL, receiveResponsesRoutine, NULL);

      //wait for this thread to finish
      pthread_join(receive_responses_thread, NULL);


      if(game_state == GAME_OVER)
      {
        //if the thread left from the game being over
        endgame_info info;
        //read the endgame info from the server
        int ret = read(sock_fd, &info, sizeof(endgame_info));
        if(ret == sizeof(endgame_info))
        {
          //display endgame info
          printf("GAME OVER: Points: %d. ", getEndGameInfoPoints(info));
          switch (getEndGameInfoResult(info)) {
            case RESULT_WIN:
              printf("YOU WON!\n");
              break;
            case RESULT_LOST:
              printf("YOU LOST!\n");
              break;
          }
        }
        printf("NEXT GAME STARTING IN 10 SEC...\n");
      }
      else if(game_state == QUIT)
      {
        //if the thread left from the player having quit
        printf("LEAVING (quit)\n");
        break;
      }
    }
    else
    {
      //error receiving board -> terminate
      printf("LEAVING\n");
      close(sock_fd);
      free(board_cards_face_up);
      exit(0);
    }

  }
  //wait for send thread to join
  pthread_join(send_plays_thread, NULL);
  //close socket
  close(sock_fd);
  printf("THANK YOU FOR PLAYING!\n");
  //free internal board memory
  free(board_cards_face_up);
  exit(0);




}
