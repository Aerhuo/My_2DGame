#include <math.h>
#include <string.h>
#include "GameData.h"
#include "GameLogic.h"
#include "Renderer.h"

const ItemConfig itemConfigs[] = {
    // 特殊物品(增加下波敌人出现的时间)
    {"闹钟", '@', 0, 0, 0, 0, 25},

    // 攻击类物品（永久增益）
    {"普通枕头", '^', 0, 10, 0, 0, 100},
    {"舒适枕头", 'V', 0, 20, 0, 0, 100},
    {"黄金枕头", '*', 0, 50, 0, 0, 25},

    // 恢复类物品（永久增益）
    {"食堂包子", '+', 10, 0, 0, 0, 100},
    {"食堂套餐烤肉", '=', 20, 0, 0, 0, 100},
    {"烤肉", '#', 50, 0, 0, 0, 25},

    // 速度类物品（永久增益）
    {"手表", '>', 0, 0, 0.25f, 0, 100},
    {"电脑", 'C', 0, 0, 1.0f, 0, 25},
    {"手机", 'P', 0, 0, 1.5f, 0, 25},

    // 时间限制类物品
    {"宿舍钥匙", '&', 0, 0, 0, SEC(10), 25}, // 10秒穿墙
    {"吹风机", '%', 0, 0, 0, SEC(30), 25},   // 30秒远程攻击
};

const int itemConfigCount = sizeof(itemConfigs) / sizeof(itemConfigs[0]);

// 名字 显示符号 基础生命值 基础攻击力 基础速度 (每秒移动的格子数) 生成权重
const CharacterConfig characterConfigs[] = {
    {"三角洲高手", '@', 100.0f, 20.0f, 6.0f, 0}, // 玩家
    {"早八", 'M', 80, 5, 3, 6000},               // 60% 概率
    {"晚自习", 'E', 100, 5, 4, 4000},            // 40% 概率
    {"周六早八", 'S', 120, 10, 5, 2000},         // 20% 概率
    {"周日晚自习", 'N', 140, 15, 6, 1000},       // 10% 概率
    {"体测", 'T', 200, 20, 7, 500},              // 5% 概率
    {"考试", 'X', 220, 25, 8, 200},              // 2% 概率
    {"挂科", 'F', 240, 30, 9, 100},              // 1% 概率
    {"假期", 'H', 300, 35, 10, 50},              // 0.5% 概率
    {"重修", 'B', 1000, 60, 12, 30},             // 0.3% 概率
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

    // 如果要删除的不是最后一个，就把最后一个移过来覆盖它
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
        player.speed += item.addSpeed;
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