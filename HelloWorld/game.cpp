#define PLAY_IMPLEMENTATION
#include "game.h"


int framCount = 0;
std::vector<std::string> grid;
int gridWidth = 0, gridHeight = 0;

bool Tick() {
	framCount++;
	DrawMap();
	DrawDebugGrid();
	

	return false;
}

// Splits the text into lines, ignoring empty lines and Windows '\r'
void LoadMap(const std::string& text) {

	grid.clear();
	std::istringstream stream(text);
	std::string line;
	while (std::getline(stream, line)) {
		if (!line.empty() && line.back() == '\r') line.pop_back();
		if (!line.empty()) grid.push_back(line);
	}

	// make a dynamic grid size to fit many maps

	gridHeight = static_cast<int>(grid.size());
	gridWidth = static_cast<int>(grid[0].size());
}

Play::Colour PickTileColour(char c) {
	switch (c) {
	case 'X': return Play::cGrey;    // wall
	case 'S': return Play::cGreen;   // start
	case 'G': return Play::cRed;     // goal
	default:  return Play::cBlack;   // walkable
	}
}


void DrawMap() {
	for (int row = 0; row < gridHeight; row++) {
		for (int col = 0; col < gridWidth; col++) {
			int x = col * TILE_SIZE;
			// PlayBuffer's y axis points UP, but the first text row is the TOP
			int y = (gridHeight - 1 - row) * TILE_SIZE + 1;   // +1 matches your grid lines
			Play::DrawRect({ x, y }, { x + TILE_SIZE, y + TILE_SIZE }, PickTileColour(grid[row][col]), true);
		}
	}
}

void DrawDebugGrid() {
	const Play::Colour lineColour{ 20, 20, 20 };   // dark, so it doesn't fight the tiles
	int w = gridWidth * TILE_SIZE, h = gridHeight * TILE_SIZE;
	for (int i = 0; i <= w; i += TILE_SIZE) Play::DrawLine({ i, 0 }, { i, h + 1 }, lineColour);
	for (int i = 1; i <= h + 1; i += TILE_SIZE) Play::DrawLine({ 0, i }, { w, i }, lineColour);
}



