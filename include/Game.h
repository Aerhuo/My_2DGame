#ifndef GAME_H
#define GAME_H

typedef enum
{
    STATE_MENU,     // 主菜单
    STATE_PLAYING,  // 游戏中
    STATE_SETTINGS, // 设置界面
    STATE_Score,    // 分数榜
    STATE_EXIT      // 退出标记
} GameState;

typedef enum
{
    EASY,
    MEDIUM,
    HARD,
} Level;

// 登录系统
void Login();

// 主菜单帧逻辑
void RunMenuFrame(GameState *state);

// 游戏中帧逻辑
void RunGameFrame(GameState *state);

// 设置界面帧逻辑
void RunSettingsFrame(GameState *state);

// 分数榜帧逻辑
void RunScoreFrame(GameState *state);

#endif