#pragma once
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Constants.h"
#include "Play.h"

enum class NodeState { Unvisited, Open, Closed };

enum class Algorithm { AStar, DepthFirst };


struct Node {
	int row = 0, col = 0;
	bool walkable = true;
	NodeState state = NodeState::Unvisited;
	Node* parent = nullptr;
	float g = 0.0f;   // real cost from the start
	float h = 0.0f;   // estimate to the goal
	float f = 0.0f;   // g + h
	bool isOnPath = false;
};

// Declared here, defined in game.cpp
extern std::vector<std::vector<Node>> grid;   // grid[row][col]
extern int gridWidth, gridHeight;
extern int startRow, startCol, goalRow, goalCol;
extern int tileSize, gridOffsetX, gridOffsetY;

void LoadMap(const std::string& text);
void DrawMap();
void DrawDebugGrid();
float Heuristic(int row, int col);
bool Tick();
void AStarInit();
void AStarStep();
void FitGridToWindow();
void UpdateMenu();
void DrawMenu();
void UpdateVisualiser();
void DrawVisualiser();
void StartVisualiser();
void DrawParentArrows();
Algorithm CurrentAlgorithm();
void DFSInit();
void DFSStep();
