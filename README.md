# MemoryGameDistributed-in-C
 Implementation of the memory game in C using distributed computing - threads, sockets, synchronization mechanisms, etc.

In this project the goal is to implement a simple distributed multiplayer concentration game (https://en.wikipedia.org/wiki/Concentration_(game)).
The game is composed of a board with a set of cards whose value are hidden from the players, for each card there is another one with the similar value. The main objective is to pick and pair cards with the same value.
When a player picks one card, it will be turned up and shown to every user, after a certain amount of time the card will be turned down. When a player select two cards with the same value, these cards will be kept shown until the end of the game. The game terminates when all cards are matched.
In order to allow multiple players student should implement a server that stores the board and receives the pick orders from the various users. The users need to execute a client, that will connect to the server and sends the pick order to the server. These clients will also show the state of the board (with shown and hidden cards). When a card is turned up or down all clients should be notified and the board representation should be updated.
Student should also implement a simple client (with no user interface) to automatically play.

![image](https://user-images.githubusercontent.com/60743836/181650787-d2fc4e85-587c-430a-b17b-1a176f86849d.png)

## Architecture
The main processes of the system are:

• Server

• UI clients

• Bot clients

In order to implement these main processes, students should use and develop the
following modules:

• Board library

• UI library

• Communication library (for the clients)

• Player management code (on the server)

• Game logic (on the server)

• Synchronization (on the server and client)

• Player logic on the Bot

The Board library and UI library are provided by the teaching staff (shown in green on the
next figure). These libraries contain the code to manege a board (initialization, check and
pay validation) and present it in a graphical window. Students can change this code, but
trying to maintain the same structure.
Multiple users or bots can be playing simultaneously against each other if connected to the
same server.

## Game rules
![image](https://user-images.githubusercontent.com/60743836/181650888-d5ff255b-dafd-42f0-8f5d-fa3a2f827f28.png)
The original game can uses two decks of playing cards that are laid on a table facing
down. In turns, each player chooses two cards and turns them up, if they are equal (same
same rank and color then that player wins the pair and plays again.
The distributed version of the game will allow multiple user/players to play on a single
board simultaneously. The players do not need to wait for their turn to pick a card on the
board. All player will be able to pick cards simultaneously at any time.
Any change made and seen by one player (card turned up or down, or matched pair) will
be forwarded to all other players.
The user should pick two cards in order to verify if they are equal. After picking two cards
they will be compared. After picking two cards the player can continue playing.
When picking one card, it will be turned face up, and presented to all players.
Simultaneous and concurrently other users can play by picking cards turned down.

#### Minimum number of players
The minimum number of player is 2. When the first player connects he will wait for another
player to connect.
When there is only one connected player the game will freeze, until another player
connects.

#### Player/user connection
When the first player connects to the server he will wait for another one.
When the second player connects, both players can start playing.
When a player connects during a game, he will:

• receive the state of the board (cards faced up)

• start receiving all updates (cards being turned dow or up)

• send picks to the server

Each player will be assigned a different color. This color is defined by the server.

#### Board
Each board is a square with DIMxDIM cards.
When the server starts, the DIM value is assigned from the command line (argv). All
created boards will have this dimension.
Before a game is started the server initializes the board with the determined size and with
random cards.
To simplify, each card contains a 2 characters string. Each string is assigned to a pair of
cards.

#### Cards
Each card of the board can be in various states:

• DOWN – No user has picked it

• UP – one user picked it.

• LOCKED – Cards that were previously paired with
other cards

When a user picks a card it is turned UP and assigned to
that user. This card can not be picked while UP.

![image](https://user-images.githubusercontent.com/60743836/181651117-9ff97ca8-2322-4fbc-8679-725da4cd9fdf.png)


#### Card picks
The user can only pick DOWN cards.
The user is only allowed to pick two cards in sequence:

• The first pick card and the second pick card

###### First picks

The user can only perform the first pick when he does not have any UP card assigned to
him.
After picking a card, the picked card it will be UP for 5 seconds.
During these 5 seconds the user can perform the second pick.
If the user does not perform the second pick these 5 second the card will be turned
DOWN, and the user will be able to do a First pick.

###### Second picks

During the 5 seconds the first pick card is UP, the user can do the second pick.
During this period, if the user picks an UP or LOCKED card, the first picked card will be
turned DOWN. In this case the user will be able to immediately repeat a first pick.
When the user picks an DOWN card, this card will be turned UP.
If the two cards (first pick and second pick cards) are different, both will be kept UP for 2
seconds. After 2 seconds both cards will be turned DOWN.
If the two cards are equal, they will both be LOCKED. The user will be able to do a new
first pick immediately.

![image](https://user-images.githubusercontent.com/60743836/181651208-5733e7cd-8f8c-4257-a92c-d86adb0344d5.png)
![image](https://user-images.githubusercontent.com/60743836/181651233-0b3de092-2586-49e4-b011-f61cac978206.png)

#### End of Game
The game concludes when all cards have bean paired and are in the LOCKED state.
The winner of the game is the player (or players) that paired more cards.

![image](https://user-images.githubusercontent.com/60743836/181651298-759eba7e-820c-45bc-bbc6-f2a89939b093.png)

#### Multiple players
The system should also implement the following rules related to the possibility of multiple
users playing on the same board.

###### Card picking
Each card can only be picked (UP) and paired (LOCKED) at a certain instance by one
player. During the period of time that card can not be picked by any user.
It two users (each on his computer) pick the same card at the same time, the system has
to guarantee that such card is only assigned to one of them.

###### Card colors
The background color depends on the user that last picked such card.
In the previous example there are 4 players picking cards:

• Blue Green yellow Pink

The system should assign a different color to each user.
All picks (card turned UP) or matches (cards LOCKED) done by a user should be
represented by his color on all the players clients.
All cards turned DOWN are printed in white (with no text).

<div>
<img src="https://user-images.githubusercontent.com/60743836/181651380-39810968-dc46-4401-817f-c11c7f24e56e.png" height ="200px"/>
</div>

## Server functionalities
The server allows clients to connect to play on a single board.
At a certain moment only one board is active and all the clients play on the same board.
The size of the board is defined at start by one of the program arguments (argv).
The server should create a socket and listen on all addresses on the port 3000.
The server should implement the game rules described previously.
The server should be multi-threaded.
When a game ends (all cards matched), all clients are notified abut it. The winners receive
receive information that they won.
After 10 second a new board is created and all the clients should create a new window.
From this moment on, the clients can start playing.
For debugging the server can show the evolution of the board in a graphical windows.

## Clients
Students should implement two different clients that will interact with the server. Some of
the code (communication layer) should be shared and reused between both applications.

#### UI Client
The UI Client is a program that connects to a server and allows a user to play against
other users.
The UI Client receives as an argument the address of the server.
After receiving information about the size of the board, this client:

• creates one window

• receives the state of the board

• processes the SDL events

• receives the messages from the server

When a mouse click event is received from the SDL library this client should send a
message to the server corresponding to a card pick.
When the client receives a message from the server, it must update the board window.
When a game ends, the client is notified, prints on the console the received information,
and, after 10 seconds, creates a new board window to start the game.

#### Bot Client
The Bot client is a program that connects to a server and plays automatically, without
using any graphical element.
This program receives as an argument the address of the server, and starts sending picks
to the server.
When the game ends it should print this information of the screen and start a new game.
The logic of this program should be programmed by the students and can range from
simple random picks to a more complex heuristic with the storage of the board state.
This bot does not need to follow some of the rules imposed by the UI.
