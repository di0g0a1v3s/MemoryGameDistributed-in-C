/*****Diogo Martins Alves 86980 31/05/2019*****/
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

int screen_width;
int screen_height;
int n_ronw_cols;
int row_height;
int col_width;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
pthread_mutex_t renderer_mutex;
TTF_Font * font;

/*routine that updates the UI (updated by an alarm every 30ms)*/
void renderRoutine(int signum)
{
	pthread_mutex_lock(&renderer_mutex);
	SDL_RenderPresent(renderer);
	pthread_mutex_unlock(&renderer_mutex);
}

/*writes text in a card in position (x,y) with color (r,g,b)*/
void writeCard(int  board_x, int board_y, char * text, int r, int g, int b){
	SDL_Rect rect;

	rect.x = board_x * col_width;
	rect.y = board_y * row_height;
	rect.w = col_width+1;
	rect.h = row_height+1;

	int text_x = board_x * col_width;
	int text_y = board_y * row_height;

	SDL_Color color = { r, g, b };
 	SDL_Surface * surface = TTF_RenderText_Solid(font, text, color);

	pthread_mutex_lock(&renderer_mutex);

	SDL_Texture* Background_Tx = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface); /* we got the texture now -> free surface */
	SDL_RenderCopy(renderer, Background_Tx, NULL, &rect);

	pthread_mutex_unlock(&renderer_mutex);


}

/*paints card in position (x,y) with color (r,g,b)*/
void paintCard(int  board_x, int board_y , int r, int g, int b){

	SDL_Rect rect;

	rect.x = board_x * col_width;
	rect.y = board_y * row_height;
	rect.w = col_width+1;
	rect.h = row_height+1;

	pthread_mutex_lock(&renderer_mutex);

	SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &rect);

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawRect(renderer, &rect);

	pthread_mutex_unlock(&renderer_mutex);

}

/*paints a card white in position (x,y)*/
void clearCard(int  board_x, int board_y){
	paintCard(board_x, board_y , 255, 255, 255);

}

/*converts a mouse position into a board position*/
void getBoardCard(int mouse_x, int mouse_y, int * board_x, int *board_y){
	*board_x = mouse_x / col_width;
	*board_y = mouse_y / row_height;
}

/*initializes the UI*/
int createBoardWindow(int width, int height,  int dim){

	screen_width = width;
	screen_height = height;
	n_ronw_cols = dim;
	row_height = height /n_ronw_cols;
	col_width = width /n_ronw_cols;
	screen_width = n_ronw_cols * col_width +1;
	screen_height = n_ronw_cols *row_height +1;

	font = TTF_OpenFont("arial.ttf", row_height);

	pthread_mutex_init(&renderer_mutex, NULL);

	if (SDL_CreateWindowAndRenderer(screen_width, screen_height, 0, &window, &renderer)  != 0) {
		printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
		exit(-1);
	}

	//sets the alarm to update the UI every 30ms
	signal(SIGALRM, renderRoutine);
	ualarm(30000,30000);

	pthread_mutex_lock(&renderer_mutex);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);


	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	int i = 0;
	for (i = 0; i <n_ronw_cols+1; i++){
		SDL_RenderDrawLine(renderer, 0, i*row_height, screen_width, i*row_height);
	}

	for (i = 0; i <n_ronw_cols+1; i++){
		SDL_RenderDrawLine(renderer, i*col_width, 0, i*col_width, screen_height);
	}
	pthread_mutex_unlock(&renderer_mutex);

}

/*closes the window*/
void closeBoardWindow(){
	if (renderer) {
		SDL_DestroyRenderer(renderer);
	}
	if (window) {
		SDL_DestroyWindow(window);
	}
	SDL_Quit();
}
