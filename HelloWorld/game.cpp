#define PLAY_IMPLEMENTATION
#include "game.h"
#include <queue>
#include <climits>
#include <cstdlib>

//////  --- Grid variables
std::vector<std::vector<Node>> grid;
int gridWidth = 0, gridHeight = 0;
int startRow = 0, startCol = 0, goalRow = 0, goalCol = 0;
int tileSize = 10, gridOffsetX = 0, gridOffsetY = 0;

//////  --- Menu variables
enum class Screen { Menu, Visualiser };
Screen currentScreen = Screen::Menu;
int selectedAlgorithm = 0, selectedMap = 0, menuRow = 0;
const int MENU_BOX_WIDTH = 140;
const int MENU_BOX_HEIGHT = 50;
const int MENU_BOX_GAP = 20;
const int MENU_FIRST_BOX_LEFT = 260;

//////  --- A Star variables
std::vector<Node*> openList;
bool searchFinished = false;
bool showParentArrows = true;
int stepCount;



bool Tick() {

	switch (currentScreen) {
		case Screen::Menu:
			UpdateMenu();       
			DrawMenu();       
			break;
		case Screen::Visualiser: // switch on algoritm and map inside those
			UpdateVisualiser(); 
			DrawVisualiser(); 
			break;
	}

	


	return false;
}


/// <summary>
/// Return the selected algorim
/// </summary>
/// <returns></returns>
Algorithm CurrentAlgorithm() { return static_cast<Algorithm>(selectedAlgorithm); }


#pragma region Menu


/// <summary>
/// Navigate on the menu screen
/// </summary>
void UpdateMenu() {

	if (Play::KeyPressed(Play::KEY_UP) || Play::KeyPressed(Play::KEY_DOWN)) menuRow = 1 - menuRow; // top row is algorim, bottom row is the map

	int direction = 0;
	if (Play::KeyPressed(Play::KEY_RIGHT)) direction = 1;
	if (Play::KeyPressed(Play::KEY_LEFT))  direction = -1;
	if (direction != 0) {
		if (menuRow == 0) selectedAlgorithm = (selectedAlgorithm + direction + ALGORITHM_COUNT) % ALGORITHM_COUNT;
		else              selectedMap = (selectedMap + direction + MAP_COUNT) % MAP_COUNT;
	}

	if (Play::KeyPressed(Play::KEY_ENTER)) StartVisualiser();
}


/// One selectable box with centred text. Chosen = filled, and yellow if its row is the active one.
void DrawMenuBox(int left, int bottom, const char* text, bool isChosen, bool isActiveRow) {

	Play::Point2D bottomLeft = { left, bottom };
	Play::Point2D topRight = { left + MENU_BOX_WIDTH, bottom + MENU_BOX_HEIGHT };

	if (isChosen) {
		Play::DrawRect(bottomLeft, topRight, isActiveRow ? Play::cYellow : Play::cGrey, true); // the last bool is the fill
	}
	Play::DrawRect(bottomLeft, topRight, Play::cWhite, false);   // outline on every box

	Play::Colour textColour = isChosen ? Play::cBlack : Play::cWhite;
	Play::DrawDebugText({ left + MENU_BOX_WIDTH / 2, bottom + MENU_BOX_HEIGHT / 2 }, text, textColour);
}

/// <summary>
/// Draws info and tiles
/// </summary>
void DrawMenu() {
	// title
	Play::DrawDebugText({ DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 100 }, "PATHFINDING", Play::cWhite);

	// Row 0: one column per algorithm
	const int algorithmRowBottom = 440;
	Play::DrawDebugText({ 60, algorithmRowBottom + MENU_BOX_HEIGHT / 2 }, "ALGORITHM", menuRow == 0 ? Play::cYellow : Play::cWhite, false);
	for (int column = 0; column < ALGORITHM_COUNT; column++) {
		int left = MENU_FIRST_BOX_LEFT + column * (MENU_BOX_WIDTH + MENU_BOX_GAP);
		DrawMenuBox(left, algorithmRowBottom, ALGORITHM_NAMES[column], column == selectedAlgorithm, menuRow == 0);
	}

	// Row 1: one column per map
	const int mapRowBottom = 320;
	Play::DrawDebugText({ 60, mapRowBottom + MENU_BOX_HEIGHT / 2 }, "MAP", menuRow == 1 ? Play::cYellow : Play::cWhite, false);
	for (int column = 0; column < MAP_COUNT; column++) {
		int left = MENU_FIRST_BOX_LEFT + column * (MENU_BOX_WIDTH + MENU_BOX_GAP);
		DrawMenuBox(left, mapRowBottom, MAPS[column].name, column == selectedMap, menuRow == 1);
	}

	// instructions
	Play::DrawDebugText({ DISPLAY_WIDTH / 2, 150 }, "UP DOWN: CHANGE ROW   LEFT RIGHT: CHANGE CHOICE   ENTER: START", Play::cGrey);
}

#pragma endregion


/// <summary>
/// Starts the selected algoritm init --  called by start visualizer
/// </summary>
void InitSearch() {
	switch (CurrentAlgorithm()) {
	case Algorithm::AStar:
		AStarInit();
		break;
	case Algorithm::DepthFirst:
		DFSInit();
		break;
	case Algorithm::MyAlg:
		MAInit();
		break;
	}
}

/// <summary>
/// Called on pressing space in visulizer stage, switches on the selected alg
/// </summary>
void StepSearch() {
	switch (CurrentAlgorithm()) {
	case Algorithm::AStar:
		AStarStep();
		break;
	case Algorithm::DepthFirst:
		DFSStep();
		break;
	case Algorithm::MyAlg:
		MAStep();
		break;
	}
}


/// <summary>
/// initialises the grid visualizer with the selected algoritm, called by pressing endter in menu
/// </summary>
void StartVisualiser() {
	LoadMap(MAPS[selectedMap].data);   // rebuilds the grid, so every old Node* is now invalid
	FitGridToWindow();
	stepCount = 0;				// reset
	searchFinished = false;     // reset the shared state, so an unfinished algorithm
	openList.clear();           // doesn't inherit leftovers from the previous run
	InitSearch();				// initialises the specific selected alg
	currentScreen = Screen::Visualiser;
}


/// <summary>
/// listens to input in visualizer stage 
/// </summary>
void UpdateVisualiser() {
	if (Play::KeyPressed(Play::KEY_SPACE)) StepSearch();
	if (Play::KeyPressed(Play::KEY_P)) showParentArrows = !showParentArrows;
	if (Play::KeyPressed(Play::KEY_BACKSPACE)) currentScreen = Screen::Menu;
}


/// <summary>
/// Start visualizer needs to run one becfore this draws the map, debug, arrows and instruction text
/// </summary>
void DrawVisualiser() {
	DrawMap();
	DrawDebugGrid();
	if (showParentArrows) DrawParentArrows();

	// Top line: the results to compare
	std::string results = std::string(ALGORITHM_NAMES[selectedAlgorithm]) + " ON " + MAPS[selectedMap].name; // alg and map
	results += "   STEPS: " + std::to_string(stepCount);  // counted steps
	if (searchFinished) {
		const Node& goal = grid[goalRow][goalCol];
		if (goal.state == NodeState::Closed) {
			results += "   PATH FOUND   ";
		}
		else {
			results += "   NO PATH";
		}
	}
	Play::DrawDebugText({ DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 12 }, results.c_str(), Play::cWhite);

	// Bottom line: the controls
	Play::DrawDebugText({ DISPLAY_WIDTH / 2, 10 }, "SPACE: STEP   P: ARROWS   BACKSPACE: MENU", Play::cGrey);
}


/// <summary>
/// scales the tile size to make the map fit on screen
/// </summary>
void FitGridToWindow() {
	const int margin = 20;
	int availableWidth = DISPLAY_WIDTH - 2 * margin;
	int availableHeight = DISPLAY_HEIGHT - 2 * margin;
	tileSize = std::min(availableWidth / gridWidth, availableHeight / gridHeight);   // whole pixels keep tiles crisp
	gridOffsetX = (DISPLAY_WIDTH - gridWidth * tileSize) / 2;                        // centre the grid
	gridOffsetY = (DISPLAY_HEIGHT - gridHeight * tileSize) / 2;
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

#pragma region Draw



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
			int x = gridOffsetX + col * tileSize;
			int y = gridOffsetY + (gridHeight - 1 - row) * tileSize;
			Play::DrawRect({ x, y }, { x + tileSize, y + tileSize }, TileColour(grid[row][col]), true);
		}
	}
	
}

// The middle of a tile on screen (same flipped y as DrawMap)
Play::Point2D TileCentre(const Node& node) {
	float x = gridOffsetX + node.col * tileSize + tileSize / 2.0f;
	float y = gridOffsetY + (gridHeight - 1 - node.row) * tileSize + tileSize / 2.0f;
	return { x, y };
}

void DrawArrow(Play::Point2D from, Play::Point2D to, Play::Colour colour) {
	Play::DrawLine(from, to, colour);

	Play::Vector2D direction = to - from;
	direction.Normalize();                              // length 1, same direction
	Play::Vector2D sideways = direction.Perpendicular();

	float headLength = tileSize * 0.3f;
	Play::Point2D headBase = to - direction * headLength;
	Play::DrawLine(to, headBase + sideways * (headLength * 0.5f), colour);
	Play::DrawLine(to, headBase - sideways * (headLength * 0.5f), colour);
}

void DrawParentArrows() {
	for (int row = 0; row < gridHeight; row++) {
		for (int col = 0; col < gridWidth; col++) {
			const Node& node = grid[row][col];
			if (node.parent == nullptr) continue;
			if (node.isOnPath) {
				DrawArrow(TileCentre(*node.parent), TileCentre(node), Play::cRed);
			}
			else {
				DrawArrow(TileCentre(*node.parent), TileCentre(node), Play::cYellow);
			}
		}
	}
}

void DrawDebugGrid() {
	const Play::Colour lineColour{ 20, 20, 20 };
	int width = gridWidth * tileSize;
	int height = gridHeight * tileSize;

	for (int column = 0; column <= gridWidth; column++) {
		int x = gridOffsetX + column * tileSize;
		Play::DrawLine({ x, gridOffsetY }, { x, gridOffsetY + height }, lineColour);
	}
	for (int row = 0; row <= gridHeight; row++) {
		int y = gridOffsetY + row * tileSize;
		Play::DrawLine({ gridOffsetX, y }, { gridOffsetX + width, y }, lineColour);
	}
}

#pragma endregion


float Heuristic(int row, int col) {   
	float rowDistance = static_cast<float>(std::abs(row - goalRow));
	float columnDistance = static_cast<float>(std::abs(col - goalCol));
	float shorterDistance = std::min(rowDistance, columnDistance);
	float longerDistance = std::max(rowDistance, columnDistance);
	return 1.414f * shorterDistance + (longerDistance - shorterDistance);

	//return static_cast<float>(sqrt((row - goalRow) * (row - goalRow) + (col - goalCol) * (col - goalCol))); // pithagora
}


#pragma region AStar




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
	//for (int i = 1; i < static_cast<int>(openList.size()); i++) {
	//	if (openList[i]->f < openList[bestIndex]->f) bestIndex = i; // compares the full cost of the open list and saves the best one
	//}
	Node* current = openList[bestIndex];
	openList.erase(openList.begin() + bestIndex);   // remove it from the open list
	current->state = NodeState::Closed;  

	stepCount++;                         // one step = one tile taken off the open list
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

#pragma endregion



#pragma region DepthFirst

void DFSInit() {
	openList.clear();
	searchFinished = false;

	Node* start = &grid[startRow][startCol];
	start->state = NodeState::Open;      // Open = discovered, waiting on the stack
	openList.push_back(start);
}


void DFSStep() {

	/// --- END CONDITIONS ---
	if (searchFinished) return;
	if (openList.empty()) { searchFinished = true; return; }   // nothing left: no path

	// Take the NEWEST tile (top of the stack). This is the only real difference from A*
	Node* current = openList.back();
	openList.pop_back();
	current->state = NodeState::Closed;

	stepCount++;

	// goal condition
	if (current->row == goalRow && current->col == goalCol) {
		searchFinished = true;
		for (Node* n = current; n != nullptr; n = n->parent) {
			n->isOnPath = true;
		}
		return;
	}

	/// --- Main loop --- 
	for (int rowOffset = -1; rowOffset <= 1; rowOffset++) {
		for (int columnOffset = -1; columnOffset <= 1; columnOffset++) {
			if (rowOffset == 0 && columnOffset == 0) continue; // ingnore itself

			int neighbourRow = current->row + rowOffset;
			int neighbourCol = current->col + columnOffset;

			// all the edge cases skip the assigning of a parent and the next step will pick next from the stack
			// outside the map
			if (neighbourRow < 0 || neighbourRow >= gridHeight || neighbourCol < 0 || neighbourCol >= gridWidth) continue;

			Node* neighbour = &grid[neighbourRow][neighbourCol];

			if (!neighbour->walkable) continue;                        // wall
			if (neighbour->state != NodeState::Unvisited) continue;    // already Open or Closed

			// don't cut corners on diagonal moves
			if (rowOffset != 0 && columnOffset != 0) {
				if (!grid[current->row + rowOffset][current->col].walkable ||
					!grid[current->row][current->col + columnOffset].walkable) {
					continue;
				}
			}


			// no edge case, advance
			neighbour->parent = current;
			neighbour->state = NodeState::Open;    // marked on discovery, so it can never be pushed twice
			openList.push_back(neighbour);
		}
	}
}

#pragma endregion


#pragma region MyAlgoritm

Node* maCurrent = nullptr;               // where the "navigating AI" is standing
Node* maTarget = nullptr;                // the visible tile it is currently walking towards
std::vector<Node*> maHistory;            // every tile it stepped away from, so it can retrace its route

// 8 directions matched by index
const int DIR_ROW[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
const int DIR_COL[8] = { 0, 1, 1,  1,  0, -1, -1, -1 };


// a bool is also a number in c++
int Sign(int value) { return (value > 0) - (value < 0); }


// if it has a parent it mean it was visited (or the start) maHistory can forget tiles if the agent turns back
bool HasAgentVisited(const Node& n) {
	return n.visited;
}



/// <summary>
/// sets initial values, sets the starting point and the current position, does the initial scan in line of sight
/// </summary>
void MAInit() {

	openList.clear();
	searchFinished = false;
	maHistory.clear();
	maTarget = nullptr; // the target tile in line of sight, not the goal

	Node* start = &grid[startRow][startCol];
	start->g = 0.0f;
	start->h = Heuristic(start->row, start->col);
	start->f = start->h;
	start->state = NodeState::Open;

	maCurrent = start;        // the agent stands on the start, which is not a candidate (see HasAgentVisited)
	MAScan(start);            // first look around
	
}


void MAStep() {
	/// --- END CONDITIONS ---
	if (searchFinished) return;


	/// --- Pick the next step ---
	Node* next = nullptr;

	if (maTarget != nullptr) {  // if a target is in sight, pick the next target in the direction of target by one tile
		int rowStep = Sign(maTarget->row - maCurrent->row); // which directio on the  rows
		int columnStep = Sign(maTarget->col - maCurrent->col); // which direction on the col
		next = &grid[maCurrent->row + rowStep][maCurrent->col + columnStep];
		maHistory.push_back(maCurrent);  // current goes to history
	}
	else { // if there is not a target close the dead end and go back one tile ( like a stack)
		maCurrent->state = NodeState::Closed;
		if (maHistory.empty()) { searchFinished = true; return; }   // back at the start with nothing left: no path

		next = maHistory.back();
		maHistory.pop_back();
	}

	// with next picked, time to take a step
	stepCount++;

	
	if (!HasAgentVisited(*next)) { // if the next tile is new, add the current to the parrent
		next->parent = maCurrent;
		openList.erase(std::remove(openList.begin(), openList.end(), next), openList.end()); // takes next out of the open list
	}
	if (next == maTarget) maTarget = nullptr;   // arrived
	maCurrent = next; // take the step
	maCurrent->visited = true;


	/// --- Goal condition --- 
	if (maCurrent->row == goalRow && maCurrent->col == goalCol) {
		searchFinished = true;
		// iterates on the parented list and turns the path flag true
		for (Node* n = maCurrent; n != nullptr; n = n->parent) {
			n->isOnPath = true;
		}
		return;
	}

	MAScan(maCurrent);     // look around from the new position
}



void MAScan(Node* from) {
	// from is the tile to look from, best will become the target if possible
	Node* best = nullptr;

	// look in all 8 direction ( do the index trick on the premade arrays
	for (int dir = 0; dir < 8; dir++) {
		int rowOffset = DIR_ROW[dir];
		int columnOffset = DIR_COL[dir];
		int fromRow = from->row;
		int fromCol = from->col;

		// one direction at the time
		while (true) {
			// add one to the direction every iteration
			int nextRow = fromRow + rowOffset;
			int nextCol = fromCol + columnOffset;

			// outside the map
			if (nextRow < 0 || nextRow >= gridHeight || nextCol < 0 || nextCol >= gridWidth) break;

			Node* tile = &grid[nextRow][nextCol];

			if (!tile->walkable) break;                          // walls break the line of sight
			if (tile->state == NodeState::Closed) break;         // dead ends are treated like walls

			// diagonal view can't squeeze between two walls. No corner cuttin
			if (rowOffset != 0 && columnOffset != 0) {
				if (!grid[nextRow][fromCol].walkable || !grid[fromRow][nextCol].walkable) break;
			}

			// first time this tile is seen: open it and calculate h (h never changes, so once is enough)
			if (tile->state == NodeState::Unvisited) {
				tile->h = Heuristic(nextRow, nextCol); // tile is grid[nextRow,nextCol]
				tile->state = NodeState::Open;
				openList.push_back(tile);
			}

			// candidates are tiles in sight that the agent hasn't stood on yet
			if (!HasAgentVisited(*tile)) { 
				if (best == nullptr || tile->h < best->h - 0.001f) best = tile;
			}

			// move the LOOK from location to the next to follow the iteration
			fromRow = nextRow;
			fromCol = nextCol;
		}
	}

	// Make best the target, but first check if best is better than the current Target
	if (best != nullptr && (maTarget == nullptr || best->h < maTarget->h - 0.001f)) {
		maTarget = best;
	}
}

#pragma endregion



