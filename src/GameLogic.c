#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <conio.h>
#include <string.h>
#include <windows.h>
#include "GameLogic.h"
#include "GameData.h"
#include "Renderer.h"
#include "DataManager.h"
#include "Game.h"

TileType worldMap[MAP_HEIGHT][MAP_WIDTH];
int distMap[MAP_HEIGHT][MAP_WIDTH];
int cameraX = 0, cameraY = 0;
int frameCount = 0;
int survivalSeconds = 0;

int minDistFormPlayerToSpawnEnemy = 20;
int minDistFormPlayerToSpawnItem = 2;
int spawnEnemyTimer = 0;
int spawnItemTimer = 0;

typedef struct
{
    int x, y;
} Point;
Point bfsQueue[MAP_WIDTH * MAP_HEIGHT];

void GenerateMap();
void UpdateCamera();
void RunBFS();
bool SpawnItem(int configIndex, int x, int y);
bool SpawnEnemy(int configIndex, int x, int y);
void SpawnDirector();
void MovePlayer(int dx, int dy);
void UpdateEnemies();
void DrawUI();

void InitGameLogic()
{
    frameCount = 0;
    survivalSeconds = 0;
    enemyCount = 0; // 清空敌人
    itemCount = 0;  // 清空物品

    // 初始化玩家 (初始位置为地图中心)
    player.config = characterConfigs[0];
    player.hp = player.config.baseHp;
    player.atk = player.config.baseAtk;
    player.speed = player.config.baseSpeed;
    player.x = MAP_WIDTH / 2;
    player.y = MAP_HEIGHT / 2;

    // 生成地图
    GenerateMap();

    // 设置摄像机位置
    UpdateCamera();
}

bool UpdateGameLogic()
{
    frameCount++;
    if (frameCount % FPS == 0)
        survivalSeconds++;

    // 累加计时器
    player.moveTimer++;

    // 计算玩家行动阈值
    float safeSpeed = (player.speed < .1f) ? .1f : player.speed;
    int playerMoveThreshold = (int)(FPS / safeSpeed);
    if (playerMoveThreshold < 1)
        playerMoveThreshold = 1;

    // 处理输入 (用 GetAsyncKeyState 实现丝滑移动)
    // 只有当计时器准备好时，才去检查键盘状态
    if (player.moveTimer >= playerMoveThreshold)
    {
        int dx = 0;
        int dy = 0;
        bool tryMove = false;

        // (0x8000 表示按键当前处于按下状态)
        if (GetAsyncKeyState('W') & 0x8000)
        {
            dx = 0;
            dy = -1;
            tryMove = true;
        }
        else if (GetAsyncKeyState('S') & 0x8000)
        {
            dx = 0;
            dy = 1;
            tryMove = true;
        }
        else if (GetAsyncKeyState('A') & 0x8000)
        {
            dx = -1;
            dy = 0;
            tryMove = true;
        }
        else if (GetAsyncKeyState('D') & 0x8000)
        {
            dx = 1;
            dy = 0;
            tryMove = true;
        }

        if (tryMove)
        {
            MovePlayer(dx, dy);

            // 只有真正触发了移动逻辑，才重置计时器
            player.moveTimer = 0;
        }
    }

    // 清空残留的缓冲区
    while (_kbhit())
        _getch();

    // 更新 AI 寻路图 (每10帧计算一次)
    if (frameCount % 10 == 0)
        RunBFS();

    // 怪物行为
    UpdateEnemies();

    // 刷新系统
    SpawnDirector();

    // 移动摄像机
    UpdateCamera();

    // 更新BUFF状态
    UpdateBuffs(&player);

    return player.hp > 0;
}

void RenderGameLogic()
{
    // 绘制地图
    for (int vy = 0; vy < VIEWPORT_HEIGHT; vy++)
    {
        for (int vx = 0; vx < VIEWPORT_WIDTH; vx++)
        {
            int wx = cameraX + vx;
            int wy = cameraY + vy;

            char c = ' ';
            if (wx >= 0 && wx < MAP_WIDTH && wy >= 0 && wy < MAP_HEIGHT)
            {
                if (worldMap[wy][wx] == TILE_WALL)
                    c = '#';
                else
                    c = ' ';
            }
            DrawChar(vx, vy, c);
        }
    }

    // 绘制物品
    for (int i = 0; i < itemCount; i++)
    {
        int sx = items[i].x - cameraX;
        int sy = items[i].y - cameraY;
        if (sx >= 0 && sx < VIEWPORT_WIDTH && sy >= 0 && sy < VIEWPORT_HEIGHT)
        {
            DrawChar(sx, sy, items[i].config.symbol);
        }
    }

    // 绘制怪物
    for (int i = 0; i < enemyCount; i++)
    {
        int sx = enemies[i].x - cameraX;
        int sy = enemies[i].y - cameraY;
        if (sx >= 0 && sx < VIEWPORT_WIDTH && sy >= 0 && sy < VIEWPORT_HEIGHT)
        {
            DrawChar(sx, sy, enemies[i].config.symbol);
        }
    }

    // 绘制玩家
    int px = player.x - cameraX;
    int py = player.y - cameraY;
    if (px >= 0 && px < VIEWPORT_WIDTH && py >= 0 && py < VIEWPORT_HEIGHT)
    {
        DrawChar(px, py, player.config.symbol);
    }

    // 绘制 UI
    DrawUI();
}

void GenerateMap()
{
    // 清空地图
    for (int y = 0; y < MAP_HEIGHT; y++)
        for (int x = 0; x < MAP_WIDTH; x++)
            worldMap[y][x] = TILE_FLOOR;

    // 随机生成 500 个障碍物
    int count = 0;
    while (count < 500)
    {
        int x = rand() % MAP_WIDTH;
        int y = rand() % MAP_HEIGHT;

        // 避开玩家出生点
        if (abs(x - MAP_WIDTH / 2) < 5 && abs(y - MAP_HEIGHT / 2) < 5)
            continue;

        if (worldMap[y][x] == TILE_FLOOR)
        {
            worldMap[y][x] = TILE_WALL;
            count++;
        }
    }
}

void UpdateCamera()
{
    // 让玩家处于中心
    int targetX = player.x - VIEWPORT_WIDTH / 2;
    int targetY = player.y - VIEWPORT_HEIGHT / 2;

    // 边界处理
    if (targetX < 0)
        targetX = 0;
    if (targetY < 0)
        targetY = 0;
    if (targetX > MAP_WIDTH - VIEWPORT_WIDTH)
        targetX = MAP_WIDTH - VIEWPORT_WIDTH;
    if (targetY > MAP_HEIGHT - VIEWPORT_HEIGHT)
        targetY = MAP_HEIGHT - VIEWPORT_HEIGHT;

    cameraX = targetX;
    cameraY = targetY;
}

// BFS 生成全图到玩家的距离
void RunBFS()
{
    // 初始化距离无限大
    for (int y = 0; y < MAP_HEIGHT; y++)
        for (int x = 0; x < MAP_WIDTH; x++)
            distMap[y][x] = 1e9;

    int head = 0, tail = 0;

    // 起点是玩家
    bfsQueue[tail++] = (Point){player.x, player.y};
    distMap[player.y][player.x] = 0;

    int dx[] = {0, 0, -1, 1};
    int dy[] = {-1, 1, 0, 0};

    while (head < tail)
    {
        Point curr = bfsQueue[head++];
        int d = distMap[curr.y][curr.x];

        for (int i = 0; i < 4; i++)
        {
            int nx = curr.x + dx[i];
            int ny = curr.y + dy[i];

            if (nx < 0 || nx >= MAP_WIDTH || ny < 0 || ny >= MAP_HEIGHT)
                continue;
            if (worldMap[ny][nx] == TILE_WALL)
                continue; // 墙不能走

            if (distMap[ny][nx] > d + 1)
            {
                distMap[ny][nx] = d + 1;
                bfsQueue[tail++] = (Point){nx, ny};
            }
        }
    }
}

void MovePlayer(int dx, int dy)
{
    int nx = player.x + dx;
    int ny = player.y + dy;

    // 撞墙/边界检查
    if (nx < 0 || nx >= MAP_WIDTH || ny < 0 || ny >= MAP_HEIGHT)
        return;

    if (worldMap[ny][nx] == TILE_WALL && !player.canPenetrateObstacles)
        return;

    // 攻击判定：如果目标格有怪
    for (int i = 0; i < enemyCount; i++)
    {
        if (enemies[i].x == nx && enemies[i].y == ny)
        {
            // 造成伤害
            enemies[i].hp -= player.atk;

            // 怪物死亡逻辑
            if (enemies[i].hp <= 0)
            {
                RemoveEnemy(i);
            }
            return; // 攻击了就不移动
        }
    }

    // 物品拾取判定
    // 检查目标位置是否有物品
    for (int i = 0; i < itemCount; i++)
    {
        if (items[i].x == nx && items[i].y == ny)
        {
            // 激活物品效果
            ItemApply(items[i].config);

            // 从地图上移除物品
            RemoveItem(i);

            i--;
        }
    }

    // 4. 执行移动
    player.x = nx;
    player.y = ny;
}

void UpdateEnemies()
{
    for (int i = 0; i < enemyCount; i++)
    {
        enemies[i].moveTimer++;

        float safeSpeed = (enemies[i].speed < .1f) ? .1f : enemies[i].speed;
        int enemyMoveThreshold = (int)(FPS / safeSpeed);
        // 每 (FPS/Speed) 帧移动一次
        if (enemies[i].moveTimer < enemyMoveThreshold)
            continue;
        enemies[i].moveTimer = 0;

        // 寻找 distMap 值最小的邻居
        int bestDist = distMap[enemies[i].y][enemies[i].x];
        int targetX = enemies[i].x;
        int targetY = enemies[i].y;

        int dx[] = {0, 0, -1, 1};
        int dy[] = {-1, 1, 0, 0};

        // 简单的随机方向打乱，防止怪物排队
        int startDir = rand() % 4;

        for (int k = 0; k < 4; k++)
        {
            int dir = (startDir + k) % 4;
            int nx = enemies[i].x + dx[dir];
            int ny = enemies[i].y + dy[dir];

            // 1. 边界检查
            if (nx >= 0 && nx < MAP_WIDTH && ny >= 0 && ny < MAP_HEIGHT)
            {
                // 墙壁碰撞检查
                if (worldMap[ny][nx] == TILE_WALL)
                    continue;

                // 寻路判断
                if (distMap[ny][nx] < bestDist)
                {
                    // 怪物堆叠检查
                    bool occupied = false;
                    for (int j = 0; j < enemyCount; j++)
                    {
                        if (i != j && enemies[j].x == nx && enemies[j].y == ny)
                        {
                            occupied = true;
                            break;
                        }
                    }

                    if (!occupied)
                    {
                        bestDist = distMap[ny][nx];
                        targetX = nx;
                        targetY = ny;
                    }
                }
            }
        }

        // 攻击玩家判定
        if (targetX == player.x && targetY == player.y)
        {
            player.hp -= enemies[i].atk;
        }
        else
        {
            enemies[i].x = targetX;
            enemies[i].y = targetY;
        }
    }
}

// 添加物品实例
bool SpawnItem(int configIndex, int x, int y)
{
    // 边界检查
    if (itemCount >= MAX_ITEMS_COUNT)
        return false;
    if (configIndex < 0 || configIndex >= itemConfigCount)
        return false;

    // 获取指针
    Item *it = &items[itemCount];

    // 加载配置数据
    it->config = itemConfigs[configIndex];
    it->x = x;
    it->y = y;

    itemCount++;
    return true;
}

// 添加怪物实例
bool SpawnEnemy(int configIndex, int x, int y)
{
    // 边界检查
    if (enemyCount >= MAX_ENEMIES_COUNT)
        return false;
    if (configIndex < 0 || configIndex >= characterConfigCount)
        return false;

    // 获取指针
    Character *e = &enemies[enemyCount];

    // 加载配置数据
    e->config = characterConfigs[configIndex];
    e->hp = e->config.baseHp;
    e->atk = e->config.baseAtk;
    e->speed = e->config.baseSpeed;

    // 初始化位置和状态
    e->x = x;
    e->y = y;
    e->moveTimer = 0; // 计时器归零

    // 清理 Buff 和特殊状态
    // 清除之前怪物的残留 Buff
    for (int i = 0; i < MAX_ACTIVE_BUFFS; i++)
    {
        e->buffs[i].active = false;
        e->buffs[i].type = NONE;
        e->buffs[i].timer = 0;
        e->buffs[i].value = 0;
    }

    // 初始化其他可能存在的状态
    e->canPenetrateObstacles = false;

    // 计数器 +1
    enemyCount++;
    return true;
}

void SpawnDirector()
{
    if (enemyCount < MAX_ENEMIES_COUNT)
    {
        spawnEnemyTimer++;
    }

    if (itemCount < MAX_ITEMS_COUNT)
    {
        spawnItemTimer++;
    }

    // 难度控制：Level 枚举影响
    // EASY: 基础8s刷一只怪 10s刷一件物品
    // MEDIUM: 基础5s刷一只怪 7s刷一件物品
    // HARD: 基础3s刷一只怪 5s刷一件物品
    int baseEnemyRate = SEC(5);
    int baseItemRate = SEC(7);
    if (gameLevel == EASY)
    {
        baseEnemyRate = SEC(8);
        baseItemRate = SEC(10);
    }
    else if (gameLevel == HARD)
    {
        baseEnemyRate = SEC(3);
        baseItemRate = SEC(5);
    }

    int minEnemyRate = SEC(3);
    int minItemRate = SEC(5);
    if (gameLevel == EASY)
    {
        minEnemyRate = SEC(4);
        minItemRate = SEC(6);
    }
    else if (gameLevel == HARD)
    {
        minEnemyRate = SEC(1);
        minItemRate = SEC(2);
    }

    // 随着生存时间减少刷新间隔 (每存活10秒，间隔减少5帧)
    int currentEnemyRate = baseEnemyRate - (survivalSeconds / 10) * 5;
    int currentItemRate = baseItemRate - (survivalSeconds / 10) * 5;
    if (currentEnemyRate < minEnemyRate)
        currentEnemyRate = minEnemyRate;
    if (currentItemRate < minItemRate)
        currentItemRate = minItemRate;

    if (spawnEnemyTimer >= currentEnemyRate) // 生成怪物逻辑
    {
        // 在视口外生成
        int x, y;
        int tries = 0;
        spawnEnemyTimer = 0;

        while (tries < 10) // 尝试十次生成
        {
            x = rand() % MAP_WIDTH;
            y = rand() % MAP_HEIGHT;

            // 必须是地板
            if (worldMap[y][x] == TILE_WALL)
            {
                tries++;
                continue;
            }

            // 必须离玩家一定距离
            if (abs(x - player.x) + abs(y - player.y) < minDistFormPlayerToSpawnEnemy)
            {
                tries++;
                continue;
            }

            // 怪物生成逻辑
            if (characterConfigCount > 1)
            {
                // 计算总权重 (跳过 index 0 的玩家)
                int totalWeight = 0;
                for (int i = 1; i < characterConfigCount; i++)
                {
                    totalWeight += characterConfigs[i].spwanRate;
                }

                // 取一个随机数
                int r = rand() % totalWeight;

                // 遍历寻找落在哪各区间
                int selectedIndex = 1;
                for (int i = 1; i < characterConfigCount; i++)
                {
                    r -= characterConfigs[i].spwanRate;
                    if (r < 0)
                    {
                        selectedIndex = i;
                        break;
                    }
                }

                SpawnEnemy(selectedIndex, x, y);
            }

            break;
        }
    }

    if (spawnItemTimer >= currentItemRate) // 生成物品逻辑
    {
        // 在视口外生成
        int x, y;
        int tries = 0;
        spawnItemTimer = 0;

        while (tries < 10) // 尝试十次生成
        {
            x = rand() % MAP_WIDTH;
            y = rand() % MAP_HEIGHT;

            // 必须是地板
            if (worldMap[y][x] == TILE_WALL)
            {
                tries++;
                continue;
            }

            // 必须离玩家一定距离
            if (abs(x - player.x) + abs(y - player.y) < minDistFormPlayerToSpawnItem)
            {
                tries++;
                continue;
            }

            // 物品生成逻辑
            if (itemConfigCount > 0)
            {
                // 计算总权重
                int totalWeight = 0;
                for (int i = 0; i < itemConfigCount; i++)
                {
                    totalWeight += itemConfigs[i].spawnRate;
                }

                // 随机
                int r = rand() % totalWeight;

                // 选择
                int selectedIndex = 0;
                for (int i = 0; i < itemConfigCount; i++)
                {
                    r -= itemConfigs[i].spawnRate;
                    if (r < 0)
                    {
                        selectedIndex = i;
                        break;
                    }
                }

                SpawnItem(selectedIndex, x, y);
            }

            break;
        }
    }
}

void DrawUI()
{
    // 绘制 UI 背景框
    for (int y = 0; y < 30; y++)
        DrawChar(89, y, '|');

    DrawStr(UI_START_X + 2, 2, "== 状态 ==");

    char buf[32];
    sprintf(buf, "玩家: %s", users[currentUserIndex].username);
    DrawStr(UI_START_X + 2, 4, buf);

    sprintf(buf, "生命: %.0f", player.hp);
    DrawStr(UI_START_X + 2, 6, buf);
    
    sprintf(buf, "攻击: %.0f", player.atk);
    DrawStr(UI_START_X + 2, 8, buf);

    sprintf(buf, "速度: %.2f", player.speed);
    DrawStr(UI_START_X + 2, 10, buf);

    sprintf(buf, "存活: %d 秒", survivalSeconds);
    DrawStr(UI_START_X + 2, 12, buf);
    sprintf(buf, "怪物: %d", enemyCount);
    DrawStr(UI_START_X + 2, 14, buf);

    sprintf(buf, "物品: %d", itemCount);
    DrawStr(UI_START_X + 2, 16, buf);

    char *levelStr = "中等";
    if (gameLevel == EASY)
        levelStr = "简单";
    if (gameLevel == HARD)
        levelStr = "困难";
    sprintf(buf, "难度: %s", levelStr);
    DrawStr(UI_START_X + 2, 18, buf);
}