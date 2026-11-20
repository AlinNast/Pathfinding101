#define PLAY_IMPLEMENTATION
#include "game.h"
#include <queue>
#include <climits>
#include <cstdlib>


std::vector<std::vector<Node>> grid;
int gridWidth = 0, gridHeight = 0;
int startRow = 0, startCol = 0, goalRow = 0, goalCol = 0;

int framCount = 0;

bool Tick() {
	framCount++;


	DrawMap();
	DrawDebugGrid();
	

	return false;
}


// Splits the text into lines, ignoring empty lines and Windows '\r'
void LoadMap(const std::string& text) {

	//  Split the text into lines (temporary, only used for parsing)
	std::vector<std::string> lines;
	std::istringstream stream(text);
	std::string line;
	while (std::getline(stream, line)) {
		if (!line.empty() && line.back() == '\r') line.pop_back();
		if (!line.empty()) lines.push_back(line);
	}
	gridHeight = static_cast<int>(lines.size());
	gridWidth = lines.empty() ? 0 : static_cast<int>(lines[0].size());

	//  gridHeight rows, each with gridWidth Nodes, this craetes all the nodes as as defaults
	grid.assign(gridHeight, std::vector<Node>(gridWidth));

	for (int row = 0; row < gridHeight; row++) {
		for (int col = 0; col < gridWidth; col++) {
			
			Node& n = grid[row][col]; // get the node at that location to set it up

			n.row = row;
			n.col = col;

			char c = lines[row][col];
			n.walkable = (c != 'X');
			if (c == 'S') { startRow = row; startCol = col; }
			if (c == 'G') { goalRow = row; goalCol = col; }
		}
	}
}


Play::Colour TileColour(const Node& n) {
	if (!n.walkable) return Play::cGrey;
	if (n.row == startRow && n.col == startCol) return Play::cGreen;
	if (n.row == goalRow && n.col == goalCol) return Play::cRed;
	if (n.state == NodeState::Open) return Play::cCyan;
	if (n.state == NodeState::Closed) return Play::Colour{ 0, 0, 60 };
	return Play::cBlack;
}


void DrawMap() {
	for (int row = 0; row < gridHeight; row++) {
		for (int col = 0; col < gridWidth; col++) {
			int x = col * TILE_SIZE;
			int y = (gridHeight - 1 - row) * TILE_SIZE + 1;
			Play::DrawRect({ x, y }, { x + TILE_SIZE, y + TILE_SIZE }, TileColour(grid[row][col]), true);
		}
	}
}


void DrawDebugGrid() {
	const Play::Colour lineColour{ 20, 20, 20 };   // dark, so it doesn't fight the tiles
	int w = gridWidth * TILE_SIZE, h = gridHeight * TILE_SIZE;
	for (int i = 0; i <= w; i += TILE_SIZE) Play::DrawLine({ i, 0 }, { i, h + 1 }, lineColour);
	for (int i = 1; i <= h + 1; i += TILE_SIZE) Play::DrawLine({ 0, i }, { w, i }, lineColour);
}


float Heuristic(int rowA, int colA) {   // pythagora
	return static_cast<float>(sqrt((rowA - goalRow) * (rowA - goalRow) + (colA - goalCol) * (colA - goalCol)));
}
