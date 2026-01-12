#ifndef GAMEDATA_H
#define GAMEDATA_H

#define MAX_ENEMIES_COUNT 100
#define FPS 30
#define SEC(s) ((int)((s) * FPS))

// 速度单位: 每秒移动的格子数
// 时间的单位是帧

#include <stdbool.h>

// 道具属性配置模板
typedef struct {
    char name[20];
    char symbol;    // 显示符号

    float addHp;       // 增加生命值
    float addAtk;      // 增加攻击力
    float addSpeed;  // 增加速度
    int duration;    // 持续时间 (帧)
    int spawnRate;   // 生成权重
} ItemConfig;

typedef struct
{
    char name[20];
    char symbol; // 显示符号

    float baseHp;
    float baseAtk;
    
    float baseSpeed; // 移动速度 (每秒移动的格子数)

    int spwanRate; // 生成权重
} CharacterConfig;

extern const ItemConfig itemConfigs[];
extern const int itemConfigCount;

extern const CharacterConfig characterConfigs[];
extern const int characterConfigCount;

typedef struct
{
    CharacterConfig config;

    int x, y; // 位置

    float hp;
    float atk;

    float speed;
    int moveTimer; // 移动计时器

    bool canPenetrateObstacles;
} Character;

extern Character player;

extern Character enemies[MAX_ENEMIES_COUNT];
extern int enemyCount;

void ItemApply(ItemConfig item);

#endif