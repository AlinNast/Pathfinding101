#pragma once
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Constants.h"
#include "Play.h"

enum class NodeState { Unvisited, Open, Closed };

struct Node {
	int row = 0, col = 0;
	bool walkable = true;
	NodeState state = NodeState::Unvisited;
	Node* parent = nullptr;
};

// Declared here, defined in game.cpp
extern std::vector<std::vector<Node>> grid;   // grid[row][col]
extern int gridWidth, gridHeight;
extern int startRow, startCol, goalRow, goalCol;

void LoadMap(const std::string& text);
void DrawMap();
void DrawDebugGrid();
float Heuristic(int row, int col);
bool Tick();

