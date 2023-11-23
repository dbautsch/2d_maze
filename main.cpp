#include <windows.h>

#include "game.h"
#include "stack.h"
#include "maze-generator.h"

int APIENTRY WinMain(HINSTANCE, HINSTANCE,
					 LPSTR, int)
{
	
	Game* game = new Game;
    game->InitGame();

    //MazeGenerator m;
    //m.SetSize(5);
    //m.GenerateMaze();

	return 0;
}
