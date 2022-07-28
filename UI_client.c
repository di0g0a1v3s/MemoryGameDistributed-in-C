/*****Diogo Martins Alves 86980 31/05/2019*****/
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <signal.h>

#include "board_library.h"
#include "UI_library.h"
#include "play.h"
#include "memory_defs.h"

//possible game states
#define GAME_ON 1
#define GAME_OVER 2
#define GAME_QUIT 3


int sock_fd;
int game_state;
int board_dim;

/*translates 2D coordinates into 1D coordinate*/
int linConv(int i, int j){
  return j*board_dim+i;
}

/*function used to make the client quit the game*/
void quitClient()
{
  //send the play (-1,-1), requesting the server to leave
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
void quitHandler(int dummy) {

  quitClient();
}


/*routine that runs in a thread and constantly reads a play from the UI and sends it to the server, until the client quits*/
void* UIReadRoutine(void* args)
{
  SDL_Event event;
  while (game_state != GAME_QUIT)
  {
  	while (SDL_PollEvent(&event))
    {
  		switch (event.type)
      {
        //client clicks que x
  			case SDL_QUIT:
        {
          quitClient();
          break;
  			}
        //client clicks in a position in the board
  			case SDL_MOUSEBUTTONDOWN:
        {
          if(game_state == GAME_ON)
          {
    				int x,y;
    				getBoardCard(event.button.x, event.button.y, &x, &y); //convert mouse position to board position
            play play = newPlay(x,y);
            //send the play to the server
            int ret = write(sock_fd, &play, sizeof(play));
            if(ret == -1)
              quitClient();
          }
          break;
  		  }
  	  }
      if(game_state == GAME_QUIT)
        break;
  	}
  }
}


/*updates the UI Window based on the response from the server*/
void updateWindow(play_response resp)
{
  switch (resp.code) {
    case CODE_FIRST_PLAY:
      paintCard(getXPlay(resp.play1), getYPlay(resp.play1) , resp.player_color[0], resp.player_color[1], resp.player_color[2]);
      writeCard(getXPlay(resp.play1), getYPlay(resp.play1) , resp.str_play1, 200, 200, 200);
      break;
    case CODE_SECOND_PLAY_RIGHT:
    case CODE_GAME_OVER:
      paintCard(getXPlay(resp.play1), getYPlay(resp.play1), resp.player_color[0], resp.player_color[1], resp.player_color[2]);
      writeCard(getXPlay(resp.play1), getYPlay(resp.play1), resp.str_play1, 0, 0, 0);
      paintCard(getXPlay(resp.play2), getYPlay(resp.play2), resp.player_color[0], resp.player_color[1], resp.player_color[2]);
      writeCard(getXPlay(resp.play2), getYPlay(resp.play2), resp.str_play2, 0, 0, 0);
      break;
    case CODE_SECOND_PLAY_WRONG:
      paintCard(getXPlay(resp.play1), getYPlay(resp.play1), resp.player_color[0], resp.player_color[1], resp.player_color[2]);
      writeCard(getXPlay(resp.play1), getYPlay(resp.play1), resp.str_play1, 255, 0, 0);

      paintCard(getXPlay(resp.play2), getYPlay(resp.play2), resp.player_color[0], resp.player_color[1], resp.player_color[2]);
      writeCard(getXPlay(resp.play2), getYPlay(resp.play2), resp.str_play2, 255, 0, 0);
      break;
    case CODE_HIDE_FIRST_PLAY:
      clearCard(getXPlay(resp.play1), getYPlay(resp.play1));
      break;
    case CODE_HIDE_BOTH_PLAYS:
      clearCard(getXPlay(resp.play1), getYPlay(resp.play1));
      clearCard(getXPlay(resp.play2), getYPlay(resp.play2));
      break;
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
      //when a valid reply is received, update the window
      updateWindow(resp);
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

/*fills the UI with the cards already known in the array board_cards_face_up*/
void fillKnownCards(board_place* board_cards_face_up)
{
  int x,y, i;
  for(i = 0; i < board_dim*board_dim; i++)
  {
    if(board_cards_face_up[i].state == STATE_LOCKED)
    {
      //paint and write locked cards
      paintCard(i%board_dim,i/board_dim, board_cards_face_up[i].player_color[0], board_cards_face_up[i].player_color[1], board_cards_face_up[i].player_color[2]);
      writeCard(i%board_dim,i/board_dim, board_cards_face_up[i].v, 0, 0, 0);
    }
    else if(board_cards_face_up[i].state == STATE_UP)
    {
      //paint and write up cards
      paintCard(i%board_dim,i/board_dim, board_cards_face_up[i].player_color[0], board_cards_face_up[i].player_color[1], board_cards_face_up[i].player_color[2]);
      writeCard(i%board_dim,i/board_dim, board_cards_face_up[i].v, 200, 200, 200);
    }
    else if(board_cards_face_up[i].state == STATE_DOWN)
    {
      //clear down cards
      clearCard(i%board_dim,i/board_dim);
    }
  }
}



int main(int argc, char * argv[]){
  struct sockaddr_in server_addr;
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

  if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
    printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    exit(-1);
  }
  if(TTF_Init()==-1) {
    printf("TTF_Init: %s\n", TTF_GetError());
    exit(2);
  }

  //alloc memory for the board with cards face up
  board_place* board_cards_face_up = (board_place*)malloc(sizeof(board_place)*board_dim*board_dim);

  game_state = GAME_OVER;

  createBoardWindow(300, 300, board_dim);

  //create thread to send plays to server
  pthread_t UI_read_thread;
  pthread_create(&UI_read_thread, NULL, UIReadRoutine, NULL);



  while(game_state != GAME_QUIT)
  {
    printf("Wait for game to start...\n");
    //receive the board with the cards face up or locked
    int ret = read(sock_fd, board_cards_face_up, sizeof(board_place)*board_dim*board_dim);
    if(ret == sizeof(board_place)*board_dim*board_dim)
    {
      fillKnownCards(board_cards_face_up);
      printf("GAME ON\n");
      game_state = GAME_ON;

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
  pthread_join(UI_read_thread, NULL);
  //closes the UI window
  closeBoardWindow();
  //close socket
  close(sock_fd);
  printf("THANK YOU FOR PLAYING!\n");
  //free internal board memory
  free(board_cards_face_up);
  exit(0);




}
