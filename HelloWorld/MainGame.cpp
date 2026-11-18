//#define PLAY_IMPLEMENTATION
//#define PLAY_USING_GAMEOBJECT_MANAGER
//#include "Constants.h"
//#include "Play.h"

#include "game.h"
using namespace Play;


// The entry point for a PlayBuffer program
void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);

}




// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate( float elapsedTime )
{
	Play::ClearDrawingBuffer(Play::cBlack);

	if (Tick()) {
		return true;
	}
	Play::PresentDrawingBuffer();

	return Play::KeyDown(KEY_ESCAPE);

}





// Gets called once when the player quits the game 
int MainGameExit( void )
{
	Play::DestroyManager();
	return 0;
}


//
