all: memory_server UI_client bot_client clean

memory_server: memory_server.o board_library.o play.o player.o player_list.o
	gcc -o memory_server memory_server.o board_library.o play.o player.o player_list.o -lpthread

UI_client: UI_client.o board_library.o play.o UI_library.o
	gcc -o UI_client UI_client.o board_library.o play.o UI_library.o -lpthread -lSDL2 -lSDL2_ttf

bot_client: bot_client.o board_library.o play.o
	gcc -o bot_client bot_client.o board_library.o play.o -lpthread


memory_server.o: memory_server.c memory_defs.h board_library.h player.h play.h player_list.h
	gcc -o memory_server.o -c memory_server.c

board_library.o: board_library.c board_library.h
	gcc -o board_library.o -c board_library.c

play.o: play.c play.h
	gcc -o play.o -c play.c

player.o: player.c player.h
	gcc -o player.o -c player.c

player_list.o: player_list.c player_list.h player.h
	gcc -o player_list.o -c player_list.c

UI_client.o: UI_client.c board_library.h UI_library.h play.h memory_defs.h
	gcc -o UI_client.o -c UI_client.c

UI_library.o: UI_library.c
	gcc -o UI_library.o -c UI_library.c

bot_client.o: bot_client.c board_library.h play.h memory_defs.h
	gcc -o bot_client.o -c bot_client.c

clean:
	rm -rf *.o
