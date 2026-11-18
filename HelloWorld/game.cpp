#define PLAY_IMPLEMENTATION
#include "game.h"


int framCount = 0;


bool Tick() {
	framCount++;
	DrawDebugGrid();

	

	return false;
}

void DrawDebugGrid() {
	// Draws Vertical lines to see the grid
	for (int i = 0; i < DISPLAY_WIDTH; i += TILE_SIZE) {
		Play::DrawLine({ i, 0 }, { i, DISPLAY_HEIGHT }, Play::cRed);
	}
	// Draws horizontal lines
	for (int i = 1; i < DISPLAY_HEIGHT+1; i += TILE_SIZE) {
		Play::DrawLine({ 0, i }, { DISPLAY_WIDTH, i }, Play::cRed);
	}
}

void GameOver() {
	
}

