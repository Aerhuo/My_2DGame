#include <math.h>
#include <string.h>
#include "GameData.h"
#include "GameLogic.h"
#include "Renderer.h"

const ItemConfig itemConfigs[] = {

    // 特殊物品
    {"闹钟", '!', 0, 0, 0, 0, 25},

    // 攻击类物品（永久增益）
    {"普通枕头", '^', 0, 10, 0, 0, 100},
    {"舒适枕头", '^', 0, 20, 0, 0, 100},
    {"黄金枕头", '^', 0, 50, 0, 0, 25},

    // 恢复类物品（永久增益）
    {"食堂包子", '+', 10, 0, 0, 0, 100},
    {"食堂套餐烤肉", '+', 20, 0, 0, 0, 100},
    {"烤肉", '+', 50, 0, 0, 0, 25},

    // 速度类物品（永久增益）
    {"手表", '>', 0, 0, 0.25f, 0, 100},
    {"电脑", '>', 0, 0, 0.5f, 0, 25},
    {"手机", '>', 0, 0, 1.0f, 0, 25},

    // 时间限制类物品
    {"宿舍钥匙", '&', 0, 0, 0, SEC(10), 25}, // 10秒穿墙
    {"吹风机", '%', 0, 0, 0, SEC(30), 25},   // 30秒远程攻击
};

const int itemConfigCount = sizeof(itemConfigs) / sizeof(itemConfigs[0]);

// 名字 显示符号 基础生命值 基础攻击力 基础速度 (每秒移动的格子数) 生成权重

const CharacterConfig characterConfigs[] = {
    {"三角洲高手", '@', 100.0f, 20.0f, 3.5f, 0},    // 玩家
    {"早八", 'M', 80, 5, 0.5, 6000},               // 普通怪物
    {"晚自习", 'E', 100, 7, 0.5, 4000},            // 普通怪物
    {"周六早八", 'S', 120, 5, 0.5, 2000},          // 普通怪物
    {"周日晚自习", 'N', 120, 10, 1.0, 1000},       // 普通怪物
    {"体测", 'T', 220, 20, 1.0, 500},              // 精英怪物
    {"考试", 'X', 200, 25, 2.0, 200},              // 精英怪物
    {"挂科", 'F', 240, 22, 3.0, 100},              // 精英怪物
    {"假期", 'H', 300, 5, 0.1, 50},                // 特殊怪物（低移速，高生命）
    {"重修", 'B', 1000, 40, 10.0, 30},             // BOSS
};

const int characterConfigCount = sizeof(characterConfigs) / sizeof(characterConfigs[0]);

Item items[MAX_ITEMS_COUNT];
int itemCount = 0;

// 玩家
Character player;

// 敌人数组
Character enemies[MAX_ENEMIES_COUNT];
int enemyCount = 0;

// 移除怪物的标准实现
void RemoveEnemy(int index)
{
    if (index < 0 || index >= enemyCount)
        return;

    // 队列交换
    if (index != enemyCount - 1)
    {
        enemies[index] = enemies[enemyCount - 1];
    }

    // 数量减一
    enemyCount--;
}

// 移除物品的标准实现
void RemoveItem(int index)
{
    if (index < 0 || index >= itemCount)
        return;

    if (index != itemCount - 1)
    {
        items[index] = items[itemCount - 1];
    }

    itemCount--;
}

void AddBuff(Character *target, BuffType type, int duration, int value)
{
    // 先检查身上是否已有该 Buff，如果有则刷新时间
    for (int i = 0; i < MAX_ACTIVE_BUFFS; i++)
    {
        if (target->buffs[i].active && target->buffs[i].type == type)
        {
            target->buffs[i].timer = duration; // 刷新持续时间
            target->buffs[i].value = value;    // 更新强度
            return;
        }
    }

    // 如果没有，找一个空槽位添加
    for (int i = 0; i < MAX_ACTIVE_BUFFS; i++)
    {
        if (!target->buffs[i].active)
        {
            target->buffs[i].active = true;
            target->buffs[i].type = type;
            target->buffs[i].timer = duration;
            target->buffs[i].value = value;

            // 添加特殊效果
            switch (type)
            {
            case ADD_ENEMY_SPAWN_TIME:
                spawnEnemyTimer += SEC(value); // 增加敌人生成时间
                break;
            case NO_OBSTACLE:
                target->canPenetrateObstacles = true;
                break;
            default:
                break;
            }

            return;
        }
    }

    // 如果满了直接忽略
}

bool HasBuff(Character *target, BuffType type)
{
    for (int i = 0; i < MAX_ACTIVE_BUFFS; i++)
    {
        if (target->buffs[i].active && target->buffs[i].type == type)
        {
            return true;
        }
    }
    return false;
}

void RemoveBuff(Character *target, BuffType type)
{
    for (int i = 0; i < MAX_ACTIVE_BUFFS; i++)
    {
        if (target->buffs[i].active && target->buffs[i].type == type)
        {
            target->buffs[i].active = false;
        }

        // 移除特殊效果
            switch (type)
            {
            case ADD_ENEMY_SPAWN_TIME:
                break;
            case NO_OBSTACLE:
                target->canPenetrateObstacles = false;
                break;
            default:
                break;
            }
    }
}

void UpdateBuffs(Character *target)
{
    for (int i = 0; i < MAX_ACTIVE_BUFFS; i++)
    {
        if (target->buffs[i].active)
        {
            if (target->buffs[i].timer > 0)
            {
                target->buffs[i].timer--;
            }
            else
            {
                // 时间到，移除 Buff
                target->buffs[i].active = false;
            }
        }
    }
}

void ItemApply(ItemConfig item)
{
    if (item.addHp > 0)
    {
        player.hp = fmin(player.hp + item.addHp, player.config.baseHp);
    }
    if (item.addAtk > 0)
    {
        player.atk += item.addAtk;
    }
    if (item.addSpeed > 0)
    {
        player.speed = fmin(player.speed + item.addSpeed, MAX_SPEED);
    }

    if (item.duration > 0)
    {
        if (!strcmp(item.name, "闹钟"))
        {
            AddBuff(&player, ADD_ENEMY_SPAWN_TIME, item.duration, 0);
        }
        else if (!strcmp(item.name, "宿舍钥匙"))
        {
            AddBuff(&player, NO_OBSTACLE, item.duration, 0);
        }
    }
}