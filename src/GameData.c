#include "GameData.h"
#include "Renderer.h"

const ItemConfig itemConfigs[] = {
    // 特殊物品(增加下波敌人出现的速度)
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
    {"宿舍钥匙", '&', 0, 0, 0, SEC(10), 25},     // 10秒
    {"吹风机", '%', 0, 0, 0, SEC(30), 25},       // 30秒
};

const int itemConfigCount = sizeof(itemConfigs) / sizeof(itemConfigs[0]);

// 名字 显示符号 基础生命值 基础攻击力 基础速度 (每秒移动的格子数) 生成权重
const CharacterConfig characterConfigs[] = {
    {"三角洲高手", '@', 100.0f, 20.0f, 6.0f, 0}, // 玩家
    {"早八", 'M', 80, 5, 3, 6000},       // 60% 概率
    {"晚自习", 'E', 100, 5, 4, 4000},    // 40% 概率
    {"周六早八", 'S', 120, 10, 5, 2000}, // 20% 概率
    {"周日晚自习", 'N', 140, 15, 6, 1000}, // 10% 概率
    {"体测", 'T', 200, 20, 7, 500},      // 5% 概率
    {"考试", 'X', 220, 25, 8, 200},      // 2% 概率
    {"挂科", 'F', 240, 30, 9, 100},      // 1% 概率
    {"假期", 'H', 300, 35, 10, 50},      // 0.5% 概率
    {"重修", 'B', 1000, 60, 12, 30},     // 0.3% 概率
};


const int characterConfigCount = sizeof(characterConfigs) / sizeof(characterConfigs[0]);

// 玩家实例
Character player;

// 敌人实例数组
Character enemies[MAX_ENEMIES_COUNT];
int enemyCount = 0;
void ItemApply(ItemConfig item)
{
    if (item.addHp > 0)
    {
        player.hp += item.addHp;
        if (player.hp > player.config.baseHp)
            player.hp = player.config.baseHp;
    }
    if (item.addAtk > 0)
    {
        player.atk += item.addAtk;
    }
    if (item.addSpeed > 0)
    {
        player.speed += item.addSpeed;
    }
}