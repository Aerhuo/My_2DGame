#include "GameData.h"
#include "Renderer.h"

// 名字 显示符号 增加生命值 增加攻击力 增加速度 持续时间 (帧) 生成权重
const ItemConfig itemConfigs[] = {
    {"闹钟", '@', 0, 0, 0, 0, 25},

    {"普通枕头", '^', 0, 10, 0, 0, 100},
    {"舒适枕头", '^', 0, 20, 0, 0, 100},
    {"黄金枕头", '^', 0, 50, 0, 0, 25},

    {"食堂包子", '+', 10, 0, 0, 0, 100},
    {"食堂套餐烤肉", '+', 20, 0, 0, 0, 100},
    {"烤肉", '+', 50, 0, 0, 0, 25},

    {"手表", '>', 0, 0, .25f, 0, 100},
    {"电脑", '>', 0, 0, 1.0f, 0, 25},
    {"手机", '>', 0, 0, 1.5f, 0, 25},

    {"宿舍钥匙", '&', 0, 0, 0, SEC(10), 25},

    {"吹风机", '%', 0, 0, 0, SEC(30), 25}
};

const int itemConfigCount = sizeof(itemConfigs) / sizeof(itemConfigs[0]);

// 名字 显示符号 基础生命值 基础攻击力 基础速度 (每秒移动的格子数) 生成权重
const CharacterConfig characterConfigs[] = {
    {"李伟峰", '@', 100.0f, 20.0f, 6.0f, 0}, // 玩家

    {"小怪1", 'g', 30.0f, 5.0f, 1.0f, 50},
    {"小怪2", 'h', 50.0f, 10.0f, 1.5f, 30},
    {"大怪", 'G', 100.0f, 20.0f, 0.5f, 20}
};

const int characterConfigCount = sizeof(characterConfigs) / sizeof(characterConfigs[0]);

// 玩家实例
Character player;

// 敌人实例数组
Character enemies[MAX_ENEMIES_COUNT];
int enemyCount = 0;