#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <stdbool.h>
#include "Game.h"

#define MAP_WIDTH 100
#define MAP_HEIGHT 100
#define VIEWPORT_WIDTH 90
#define VIEWPORT_HEIGHT 30
#define UI_START_X 90

typedef enum
{
    TILE_FLOOR = 0,
    TILE_WALL = 1
} TileType;

extern TileType worldMap[MAP_HEIGHT][MAP_WIDTH];
extern int cameraX, cameraY;
extern int survivalSeconds;

// 初始化一局新游戏 (生成地图、重置玩家)
void InitGameLogic();

// 游戏主逻辑更新 (返回 false 表示玩家死亡)
bool UpdateGameLogic();

// 游戏主渲染 (绘制地图、实体、UI)
void RenderGameLogic();

#endif