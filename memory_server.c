/*****Diogo Martins Alves 86980 31/05/2019*****/
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#include "memory_defs.h"
#include "board_library.h"
#include "player.h"
#include "play.h"
#include "player_list.h"

//possible game states
#define GAME_ON 1
#define GAME_OVER 2
#define GAME_WAITING 3

player_list* list_of_players;
int game_state;
int sock_fd;
pthread_mutex_t* mutexes;
pthread_mutex_t game_state_mutex;
int board_dim;

//auxiliary structure to pass arguments to threads
struct _aux{
  player* player;
  play play;
};

/*determines the current number of active players (those that didn't quit)*/
int numberActivePlayers()
{
  player_list* cursor = list_of_players;
  int count = 0;
  while(!isEmptyPlayerList(cursor))
  {
    if(getPlayerFromList(cursor)->state != QUIT)
      count++;
    cursor = getNextElementPlayerList(cursor);
  }
  return count;
}

/*updates the game state*/
void setGameState(int _game_state)
{
  pthread_mutex_lock(&game_state_mutex);

  if(_game_state == GAME_WAITING)
  {
    //if the game state is changed to GAME_WAITING (frozen), send this info to the players
    player_list* cursor = list_of_players;
    play_response game_frozen_resp;
    game_frozen_resp.code = CODE_GAME_FROZEN;
    while(!isEmptyPlayerList(cursor))
    {
      if(getPlayerFromList(cursor)->state != QUIT && getPlayerFromList(cursor)->state != WAITING_TO_JOIN)
      {
        int ret = write(getSockFd(getPlayerFromList(cursor)), &game_frozen_resp, sizeof(play_response));
      }
      cursor = getNextElementPlayerList(cursor);
    }
  }
  else if(_game_state == GAME_ON && game_state == GAME_WAITING)
  {
    //if the game state is changed from GAME_WAITING (frozen), to GAME_ON (unfrozen), send this info to players
    player_list* cursor = list_of_players;
    play_response game_unfrozen_resp;
    game_unfrozen_resp.code = CODE_GAME_UNFROZEN;
    while(!isEmptyPlayerList(cursor))
    {
      if(getPlayerFromList(cursor)->state != QUIT && getPlayerFromList(cursor)->state != WAITING_TO_JOIN)
      {
        int ret = write(getSockFd(getPlayerFromList(cursor)), &game_unfrozen_resp, sizeof(play_response));
      }
      cursor = getNextElementPlayerList(cursor);
    }
  }

  game_state = _game_state;
  pthread_mutex_unlock(&game_state_mutex);
}


/*function to make a player quit the game*/
void quitPlayer(player* player)
{
  if(player->state != QUIT)
  {
    //send to player permission to leave
    play_response resp;
    resp.code = CODE_PERMISSION_TO_LEAVE;
    write(getSockFd(player), &resp, sizeof(play_response));
  }

  player->state = QUIT; //update the player state
  close(player->sock_fd);
  printf("Player Quit. Current number of players: %d\n", numberActivePlayers());

  if(numberActivePlayers() < 2)
  {
    //if there are less than 2 players, freeze the game;
    setGameState(GAME_WAITING);
    printf("GAME FROZEN, WAITING FOR PLAYERS\n");
  }
}

/*returns the current state of the game*/
int getGameState()
{
  pthread_mutex_lock(&game_state_mutex);
  int _game_state = game_state;
  pthread_mutex_unlock(&game_state_mutex);
  return _game_state;
}

/*function that terminates execution of the server (when Ctrl+c is pushed)*/
void quitHandler(int dummy) {
  printf("\nTERMINATING...\n");



  player_list* cursor = list_of_players;

  while(!isEmptyPlayerList(cursor))
  {
    if(getPlayerFromList(cursor)->state != QUIT)
    {
      printf("Quitting Player...\n");
      quitPlayer(getPlayerFromList(cursor)); //quit all players
    }
    close(getPlayerFromList(cursor)->sock_fd); //close player sockets

    cursor = getNextElementPlayerList(cursor);
  }

  close(sock_fd);
  //free all allocated memory
  freePlayerList(list_of_players);
  free(mutexes);
  closeBoard();
  printf("TERMINATED.\n");
  exit(0);
}


/*returns the maximum number of points from any player*/
int maxPoints()
{
  int max = 0;
  player_list* cursor = list_of_players;
  while(!isEmptyPlayerList(cursor))
  {
    if(getPlayerFromList(cursor)->state != WAITING_TO_JOIN)
    {
      if(getPlayerFromList(cursor)->n_corrects > max)
        max = getPlayerFromList(cursor)-> n_corrects;
    }
    cursor = getNextElementPlayerList(cursor);
  }
  return max;
}


/*sends to the players the information of game over*/
void sendEndGameInfo()
{
  endgame_info info; //contains player score and wether player won or lost
  player_list* cursor = list_of_players;
  int max = maxPoints();
  while(!isEmptyPlayerList(cursor))
  {
    if(getPlayerFromList(cursor)->state != WAITING_TO_JOIN && getPlayerFromList(cursor)->state != QUIT)
    {
      if(getPlayerFromList(cursor)->n_corrects == max)
        info = newEndGameInfo(RESULT_WIN, getPlayerFromList(cursor)->n_corrects); //player won
      else
        info = newEndGameInfo(RESULT_LOST, getPlayerFromList(cursor)->n_corrects); //player lost

      int ret = write(getSockFd(getPlayerFromList(cursor)), &info, sizeof(endgame_info));
      if(ret == -1)
        quitPlayer(getPlayerFromList(cursor));
    }
    cursor = getNextElementPlayerList(cursor);
  }
  printf("End Game Info sent\n");
}


/*checks wether the game state should change to GAME_ON and updates everything accondingly*/
void checkAndUpdateGameOn()
{
  int i;
  //if the state is WAITING and there are 2 or more players
  if(getGameState() == GAME_WAITING && numberActivePlayers() >= 2 )
  {
    setGameState(GAME_ON);
    printf("GAME ON. Players: %d\n", numberActivePlayers());

    for(i = 0; i < board_dim*board_dim; i++)
      pthread_mutex_lock(&(mutexes[i])); //lock every place before sending the board

    board_place* board_cards_face_up = getBoardCardsFaceUp(); //board with only the cards UP or LOCKED (the DOWN cards have empty string)

    player_list* cursor = list_of_players;
    //send the board_cards_face_up to every WAITING_TO_JOIN player
    while(!isEmptyPlayerList(cursor))
    {
      if(getPlayerFromList(cursor)->state == WAITING_TO_JOIN)
      {
        int ret = write(getSockFd(getPlayerFromList(cursor)), board_cards_face_up, sizeof(board_place)*board_dim*board_dim);
        getPlayerFromList(cursor)->state = NO_PICK;
        if(ret == -1)
          quitPlayer(getPlayerFromList(cursor));

      }
      cursor = getNextElementPlayerList(cursor);
    }

    for(i = 0; i < board_dim*board_dim; i++)
      pthread_mutex_unlock(&(mutexes[i])); //unlock the card places

    free(board_cards_face_up);
    printf("Board sent to waiting players\n");

  }
}

/*resets all players in the list (state,play1 and n_corrects)*/
void resetPlayers()
{
  player_list* cursor = list_of_players;
  while(!isEmptyPlayerList(cursor))
  {
    if(getPlayerFromList(cursor)->state != QUIT)
    {
      resetPlayer(getPlayerFromList(cursor));
    }
    else
      getPlayerFromList(cursor)->n_corrects = 0;
    cursor = getNextElementPlayerList(cursor);
  }
}


/*translates 2D coordinates into 1D coordinate*/
int linConv(int i, int j){
  return j*board_dim+i;
}

/*routine that is executed when game over*/
void* endGameRoutine(void* args)
{
  printf("NEW GAME STARTING IN 10 SECS...\n");
  sleep(10); //wait 10 secs for next game to start
  resetPlayers(); //reset the players
  initBoard(board_dim); //generate a different board
  setGameState(GAME_WAITING);
  checkAndUpdateGameOn();
}

/*routine used for the thread that hides the first player pick after 5 secs*/
void* hideFistPickRoutine(void* args)
{
    struct _aux* arguments = args;
    player* player = arguments->player;
    play play = arguments->play;
    free(arguments);

    sleep(5);

    if(getGameState() == GAME_OVER)
      return NULL;

    //hide card only if current player first pick is the same as the pick received and the player state is still FIRST_PICK (or the player has left and that card is still up)
    if(playsAreSame(player->play1, play) && (player->state == FIRST_PICK || (player->state == QUIT && getBoardPlaceState(getXPlay(play), getYPlay(play)) == STATE_UP)))
    {
      play_response delayed_response;

      setBoardPlaceState(getXPlay(play), getYPlay(play), STATE_DOWN); //update card state in board
      delayed_response.code = CODE_HIDE_FIRST_PLAY;
      delayed_response.play1 = play;

      //send to all players info to hide the card
      player_list* cursor = list_of_players;
      while(!isEmptyPlayerList(cursor))
      {
        if(getPlayerFromList(cursor)->state != QUIT && getPlayerFromList(cursor)->state != WAITING_TO_JOIN)
        {
          int ret = write(getSockFd(getPlayerFromList(cursor)), &delayed_response, sizeof(play_response));
          if(ret == -1)
            quitPlayer(getPlayerFromList(cursor));
        }
        cursor = getNextElementPlayerList(cursor);
      }

      if(player->state == FIRST_PICK)
        player->state = NO_PICK; //update the player state
    }


}

/*routine used for the thread that hides both player picks after 2 secs if they are wrong*/
void* hideSecondPickRoutine(void* args)
{
    struct _aux* arguments = args;
    player* player = arguments->player;
    play play2 = arguments->play;
    free(arguments);

    sleep(2);

    if(getGameState() == GAME_OVER)
      return NULL;

    play_response delayed_response;

    //fist play is player->play1 and second play is the one from the arguments
    setBoardPlaceState(getXPlay(play2), getYPlay(play2), STATE_DOWN);//update card state in board
    setBoardPlaceState(getXPlay(player->play1), getYPlay(player->play1), STATE_DOWN);//update card state in board
    delayed_response.code = CODE_HIDE_BOTH_PLAYS;
    delayed_response.play1 = player->play1;
    delayed_response.play2 = play2;

    //send to all players info to hide both cards
    player_list* cursor = list_of_players;
    while(!isEmptyPlayerList(cursor))
    {
      if(getPlayerFromList(cursor)->state != QUIT && getPlayerFromList(cursor)->state != WAITING_TO_JOIN)
      {
        int ret = write(getSockFd(getPlayerFromList(cursor)), &delayed_response, sizeof(play_response));
        if(ret == -1)
          quitPlayer(getPlayerFromList(cursor));
      }
      cursor = getNextElementPlayerList(cursor);

      if(player->state == SECOND_PICK)
        player->state = NO_PICK; //update player state
    }


}


/*routine that runs in the thread specific to each player. Exits only when player quits*/
void* handlePlayerRoutine(void *args)
{
    play play;
    player* player = args;
    while(player->state != QUIT)
    {
      int ret = read(getSockFd(player), &play, sizeof(play)); //read a play from the client
      if(ret == sizeof(play))
      {
        //if the play is (-1,-1), it means the client wants to quit
        if(getXPlay(play) == -1 && getYPlay(play) == -1)
        {
          quitPlayer(player); //quit the player
          break;
        }
        else if(getGameState() == GAME_ON)
        {
          int first_pick_place_locked = 0;
          int first_pick_x, first_pick_y;
          if(player->state == FIRST_PICK && !(getXPlay(player->play1) == getXPlay(play) && getYPlay(player->play1) == getYPlay(play)))
          {
            first_pick_place_locked = 1;
            first_pick_x = getXPlay(player->play1);
            first_pick_y = getYPlay(player->play1);
          }

          if(first_pick_place_locked)
            pthread_mutex_lock(&(mutexes[linConv(first_pick_x,first_pick_y)]));
          pthread_mutex_lock(&(mutexes[linConv(getXPlay(play),getYPlay(play))])); //lock the place trying to be accessed
          play_response resp = boardPlay(play, player); //response to the play
          pthread_mutex_unlock(&(mutexes[linConv(getXPlay(play),getYPlay(play))]));
          if(first_pick_place_locked)
            pthread_mutex_unlock(&(mutexes[linConv(first_pick_x,first_pick_y)]));

          if(resp.code == CODE_FIRST_PLAY)
          {
            //set a thread to hide this card in 5 secs
            struct _aux* thread_args = (struct _aux*) malloc(sizeof(struct _aux));
            thread_args->player = player;
            thread_args->play = play;
            pthread_t delayed_thread;
            pthread_create(&delayed_thread, NULL, hideFistPickRoutine, (void*)thread_args);
          }
          if(resp.code == CODE_SECOND_PLAY_WRONG)
          {
            //set a thread to hide both cards in 2 secs
            struct _aux* thread_args = (struct _aux*) malloc(sizeof(struct _aux));
            thread_args->player = player;
            thread_args->play = play;
            pthread_t delayed_thread;
            pthread_create(&delayed_thread, NULL, hideSecondPickRoutine, (void*)thread_args);
          }

          player_list* cursor = list_of_players;

          //send the response to every player in game
          pthread_mutex_lock(&game_state_mutex);
          while(!isEmptyPlayerList(cursor) && game_state == GAME_ON)
          {
            if(getPlayerFromList(cursor)->state != QUIT && getPlayerFromList(cursor)->state != WAITING_TO_JOIN)
            {
              int ret = write(getSockFd(getPlayerFromList(cursor)), &resp, sizeof(play_response));
              if(ret == -1)
                quitPlayer(getPlayerFromList(cursor)); //quit the player if there is an error reading
            }
            cursor = getNextElementPlayerList(cursor);
          }
          pthread_mutex_unlock(&game_state_mutex);


          if(resp.code == CODE_GAME_OVER)
          {
            //if the game is over, send scores to the players
            pthread_mutex_lock(&game_state_mutex);
            printf("GAME OVER\n");
            game_state = GAME_OVER;
            sendEndGameInfo();
            pthread_mutex_unlock(&game_state_mutex);
            //thread to start new game in 10 secs
            pthread_t endgame_thread;
            pthread_create(&endgame_thread, NULL, endGameRoutine, NULL);
          }

        }

      }
      else if(ret == -1 && player != NULL)
      {
        //quit the player if there is an error reading
        player->state = QUIT;
        printf("Player Quit. Current number of players: %d\n", numberActivePlayers());


        if(numberActivePlayers() < 2)
        {
          //if there are less than 2 active players, freeze the game
          setGameState(GAME_WAITING);
          printf("GAME FROZEN, WAITING FOR PLAYERS\n");
        }
        break;
      }
    }

}



int main(int argc, char* argv[]){

  struct sockaddr_in local_addr;
  int i;

  //define the function to be executed when ctrl+c is pushed
  signal(SIGINT, quitHandler);

  //the argument must be an even integer
  if(argc < 2)
  {
    printf("Second argument must be the dimension of the board\n");
    exit(-1);
  }

  if(sscanf(argv[1], "%d", &board_dim) != 1 || board_dim%2 != 0)
  {
    printf("Second argument must be an even integer\n");
    exit(-1);
  }

  //initialize the mutexes (1 for each position in the board)
  mutexes = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t)*board_dim*board_dim);
  for(i = 0; i < board_dim*board_dim; i++)
    pthread_mutex_init(&(mutexes[i]), NULL);

  //initialize the game_state mutex
  pthread_mutex_init(&game_state_mutex, NULL);

  //create the socket
  sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd == -1){
    perror("Socket: ");
    exit(-1);
  }

  //bind the socket
  local_addr.sin_family = AF_INET;
  local_addr.sin_port = htons(MEMORY_PORT);
  local_addr.sin_addr.s_addr = INADDR_ANY;
  int err = bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(local_addr));
  if(err == -1) {
    perror("Bind");
    exit(-1);
  }
  printf("Socket created and binded \n");


  //initialize the game
  setGameState(GAME_WAITING);
  initBoard(board_dim);

  //create a list in which players will be stored
  list_of_players = initPlayerList();

  //listen to players
  listen(sock_fd, MAX_PLAYERS);
  printf("WAITING FOR PLAYERS TO JOIN...\n");


  while(1)
  {
    player* new_player;

    //accept player
    int c = sizeof(struct sockaddr_in);
    struct sockaddr addr;
    int player_sock_fd = accept(sock_fd, &addr, &c);
    if(player_sock_fd == -1)
      break;

    //create a player and send the board dimension
    new_player = createNewPlayer(player_sock_fd);
    int ret = write(getSockFd(new_player), &board_dim, sizeof(int));
    if(ret == -1)
    {
      free(new_player);
      continue;
    }

    //create a thread for the player that runs the handlePlayerRoutine
    pthread_t player_thread;
    pthread_create(&player_thread, NULL, handlePlayerRoutine, (void*)new_player);
    //add the player to the list
    list_of_players = createNewNodePlayerList(list_of_players, new_player);
    printf("NEW PLAYER. ACTIVE PLAYERS: %d\n", numberActivePlayers());

    if(getGameState() == GAME_ON && numberActivePlayers() >= 2)
    {
      //if the game is running:
      //send the board with only the cards facing up or locked to the new player

      for(i = 0; i < board_dim*board_dim; i++)
        pthread_mutex_lock(&(mutexes[i])); //lock every place before sending the board
      board_place* board_cards_face_up = getBoardCardsFaceUp();
      int ret = write(getSockFd(new_player), board_cards_face_up, sizeof(board_place)*board_dim*board_dim);
      for(i = 0; i < board_dim*board_dim; i++)
        pthread_mutex_unlock(&(mutexes[i])); //unlock the card places

      new_player->state = NO_PICK; //update player state
      if(ret == -1)
        quitPlayer(new_player);
      free(board_cards_face_up);
      printf("Board sent to new player\n");
    }
    checkAndUpdateGameOn(); //check if the game should be on, and update
  }

}
