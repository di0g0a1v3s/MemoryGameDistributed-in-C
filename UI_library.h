/*****Diogo Martins Alves 86980 31/05/2019*****/
void writeCard(int  board_x, int board_y, char * text, int r, int g, int b);
void paintCard(int  board_x, int board_y , int r, int g, int b);
void clearCard(int  board_x, int board_y);
void getBoardCard(int mouse_x, int mouse_y, int * board_x, int *board_y);
int createBoardWindow(int width, int height,  int dim);
void closeBoardWindow();
