#pragma once
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Constants.h"
#include "Play.h"


extern std::vector<std::string> grid;   // grid[row][col]
extern int gridWidth, gridHeight;       // in tiles

void LoadMap(const std::string& text);

void DrawMap();

bool Tick();

void DrawDebugGrid();

