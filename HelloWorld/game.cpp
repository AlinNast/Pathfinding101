#define PLAY_IMPLEMENTATION
#include "game.h"
#include <queue>
#include <climits>
#include <cstdlib>


std::vector<std::vector<Node>> grid;
int gridWidth = 0, gridHeight = 0;
int startRow = 0, startCol = 0, goalRow = 0, goalCol = 0;
std::vector<Node*> openList;
bool searchFinished = false;
int framCount = 0;

bool Tick() {
	framCount++;


	DrawMap();
	DrawDebugGrid();
	
	/// -----  Input 
	if (Play::KeyPressed(Play::KEY_SPACE)) AStarStep();

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
	if (n.state == NodeState::Closed) return Play::Colour{ 0, 0, 25 };
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

// The middle of a tile on screen (same flipped y as DrawMap)
Play::Point2D TileCentre(const Node& node) {
	float x = node.col * TILE_SIZE + TILE_SIZE / 2.0f;
	float y = (gridHeight - 1 - node.row) * TILE_SIZE + 1 + TILE_SIZE / 2.0f;
	return { x, y };
}

void DrawArrow(Play::Point2D from, Play::Point2D to, Play::Colour colour) {
	Play::DrawLine(from, to, colour);

	Play::Vector2D direction = to - from;
	direction.Normalize();                              // length 1, same direction
	Play::Vector2D sideways = direction.Perpendicular();

	float headLength = TILE_SIZE * 0.3f;
	Play::Point2D headBase = to - direction * headLength;
	Play::DrawLine(to, headBase + sideways * (headLength * 0.5f), colour);
	Play::DrawLine(to, headBase - sideways * (headLength * 0.5f), colour);
}

void DrawParentArrows() {
	for (int row = 0; row < gridHeight; row++) {
		for (int col = 0; col < gridWidth; col++) {
			const Node& node = grid[row][col];
			if (node.parent == nullptr) continue;
			// *node.parent turns the pointer into the Node it points to
			DrawArrow(TileCentre(*node.parent), TileCentre(node), Play::cYellow);
		}
	}
}

void DrawDebugGrid() {
	const Play::Colour lineColour{ 20, 20, 20 };   // dark
	int w = gridWidth * TILE_SIZE, h = gridHeight * TILE_SIZE;
	for (int i = 0; i <= w; i += TILE_SIZE) Play::DrawLine({ i, 0 }, { i, h + 1 }, lineColour);
	for (int i = 1; i <= h + 1; i += TILE_SIZE) Play::DrawLine({ 0, i }, { w, i }, lineColour);
	if (searchFinished) {
		DrawParentArrows();
	}
}


float Heuristic(int row, int col) {   
	float rowDistance = static_cast<float>(std::abs(row - goalRow));
	float columnDistance = static_cast<float>(std::abs(col - goalCol));
	float shorterDistance = std::min(rowDistance, columnDistance);
	float longerDistance = std::max(rowDistance, columnDistance);
	return 1.414f * shorterDistance + (longerDistance - shorterDistance);

	//return static_cast<float>(sqrt((row - goalRow) * (row - goalRow) + (col - goalCol) * (col - goalCol))); // pithagora
}




void AStarInit() {
	// setup general vars
	openList.clear();
	searchFinished = false;

	Node* start = &grid[startRow][startCol];   // & gives the address of the node, saves a pointer
	start->g = 0.0f;                           // -> is "." for pointers
	start->h = Heuristic(start->row, start->col); // heuristic for the start
	start->f = start->g + start->h;
	start->state = NodeState::Open;
	openList.push_back(start);
}


void AStarStep() {

	/// --- END CONDITIONS --- 
	if (searchFinished) return;
	if (openList.empty()) { searchFinished = true; return; };    // nowhere left to look: no path


	// Pick the item with the best F from the open list , the first time the best is the start, set by the init
	const float tolerance = 0.001f;
	int bestIndex = 0;
	for (int i = 1; i < static_cast<int>(openList.size()); i++) {
		Node* candidate = openList[i];
		Node* best = openList[bestIndex];

		bool hasLowerF = candidate->f < best->f - tolerance;
		bool hasSameF = std::abs(candidate->f - best->f) <= tolerance;

		if (hasLowerF || (hasSameF && candidate->h < best->h)) bestIndex = i;
	}
	for (int i = 1; i < static_cast<int>(openList.size()); i++) {
		if (openList[i]->f < openList[bestIndex]->f) bestIndex = i; // compares the full cost of the open list and saves the best one
	}
	Node* current = openList[bestIndex];
	openList.erase(openList.begin() + bestIndex);   // remove it from the open list
	current->state = NodeState::Closed;  

	// Here i can save the state to make replays

	if (current->row == goalRow && current->col == goalCol) {
		searchFinished = true; 
		for (Node* n = &grid[goalRow][goalCol]; n != nullptr; n = n->parent) {
			n->isOnPath = true;
		}
		PLAY_TRACE("",grid[goalRow][goalCol].g);
		return;
	} // Check for current == goal


	/// --- MAIN LOOP ---
	for (int rowOffset = -1; rowOffset <= 1; rowOffset++) { // check the neighbouring tiles in the row
		for (int columnOffset = -1; columnOffset <= 1; columnOffset++) { // check the neighbouring tiles in the col
			if (rowOffset == 0 && columnOffset == 0) continue;           // skip itself

			// coordinates of the neighboring row that to be checked
			int neighbourRow = current->row + rowOffset;
			int neighbourCol = current->col + columnOffset;

			// Edge cases
			// outside map
			if (neighbourRow < 0 || neighbourRow >= gridHeight || neighbourCol < 0 || neighbourCol >= gridWidth) continue;   // outside the map

			Node* neighbour = &grid[neighbourRow][neighbourCol];


			// not walkable
			if (!neighbour->walkable) {
				neighbour->state = NodeState::Closed;
				
			}


			// closed skips the rest of the calculations
			if (neighbour->state == NodeState::Closed) {
				continue;
			}


			// dont cut corners
			if (rowOffset != 0 && columnOffset != 0) { // if the move is diagonal over a unwalkable dont do it
				if (!grid[current->row + rowOffset][current->col].walkable || !grid[current->row][current->col + columnOffset].walkable) {
					continue;
				}
			}

			// after edge cases , calculate g h f
			float stepCost = (rowOffset != 0 && columnOffset != 0) ? 1.414f : 1.0f; 
			float newG = current->g + stepCost;  // add to the cost the curent tile already has

			if (neighbour->state == NodeState::Unvisited) {
				neighbour->g = newG;                           // -> is "." for pointers
				neighbour->h = Heuristic(neighbour->row, neighbour->col); // heuristic for the start
				neighbour->f = neighbour->g + neighbour->h;
				neighbour->state = NodeState::Open;
				neighbour->parent = current;
				openList.push_back(neighbour);
			}
			else if (neighbour->state == NodeState::Open && newG < neighbour->g) {
				neighbour->g = newG;                           // -> is "." for pointers
				neighbour->f = neighbour->g + neighbour->h;
				neighbour->parent = current;
				PLAY_TRACE("Expanding (%d,%d) g=%.2f h=%.2f f=%.2f\n", current->row, current->col, current->g, current->h, current->f);
			}
		}
	}


}


