#ifndef GAMEDATA_H
#define GAMEDATA_H

#define MAX_ENEMIES_COUNT 100
#define MAX_ITEMS_COUNT 50
#define MAX_ACTIVE_BUFFS 8
#define FPS 30
#define SEC(s) ((int)((s) * FPS))

// 速度单位: 每秒移动的格子数
// 时间的单位是帧

#include <stdbool.h>

// BUFF 类型
typedef enum {
    ADD_ENEMY_SPAWN_TIME, //增加敌人生成时间
    NO_OBSTACLE,//穿墙
    RANGE_ATTACK, //远程攻击
} BuffType;

typedef struct {
    BuffType type;  // 类型 ID
    int timer;      // 剩余时间 (帧)
    int value;
    bool active;    // 是否激活
} Buff;

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
    Buff buffs[MAX_ACTIVE_BUFFS]; 
} Character;

// 给目标添加一个 Buff (自动处理叠加或刷新时间)
void AddBuff(Character* target, BuffType type, int duration, int value);

// 检查目标是否有某个 Buff (返回 true/false)
bool HasBuff(Character* target, BuffType type);

// 移除某个 Buff
void RemoveBuff(Character* target, BuffType type);

// 每帧更新 Buff 状态
void UpdateBuffs(Character* target);

extern Character player;

extern Character enemies[MAX_ENEMIES_COUNT];
extern int enemyCount;

void ItemApply(ItemConfig item);

#endif