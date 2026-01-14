#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <stdbool.h>
#include <windows.h>
#include "Game.h"
#include "Renderer.h"
#include "DataManager.h"
#include "GameLogic.h"
int currentUserIndex = -1;

// 登录系统
void Login()
{
    char username[MAX_NAME_LEN];
    char password[MAX_PWD_LEN];
    int choice;

    while (1)
    {
        // 清屏
        system("cls");

        printf("========================================\n");
        printf("      李伟峰梦境实现者 - 登录系统    \n");
        printf("========================================\n");
        printf("1. 登录\n");
        printf("2. 注册\n");
        printf("3. 退出\n");
        printf("请选择: ");

        if (scanf("%d", &choice) != 1)
        {
            // 处理非法输入，防止死循环
            while (getchar() != '\n')
                ;
            continue;
        }

        if (choice == 3)
            exit(0);

        if (choice == 1 || choice == 2)
        {
            printf("\n请输入用户名: \n");
            scanf("%s", username);
            printf("\n请输入密码: \n");
            scanf("%s", password);

            if (choice == 1)
            {
                if (LoginUser(username, password))
                {
                    printf("\n登录成功! 欢迎, %s. \n", username);

                    // 查找当前用户索引
                    for (int i = 0; i < userCount; i++)
                    {
                        if (strcmp(users[i].username, username) == 0)
                        {
                            currentUserIndex = i;
                            break;
                        }
                    }

                    system("pause"); // 暂停让用户看一眼
                    break;           // 跳出循环，进入游戏
                }
                else
                {
                    printf("\n错误: 用户名或密码错误. \n");
                }
            }
            else
            {
                if (RegisterUser(username, password))
                {
                    printf("\n注册成功! 请登录. \n");
                }
                else
                {
                    printf("\n错误: 用户名已存在. \n");
                }
            }

            // 等待用户按键继续
            system("pause");
        }
    }
}

char *menuOptions[] = {
    "开始游戏",
    "设置",
    "分数榜",
    "游戏介绍",
    "退出游戏"};

char *selectedMenuOptions[] = {
    "> 开始游戏 <",
    "> 设置 <",
    "> 分数榜 <",
    "> 游戏介绍 <",
    "> 退出游戏 <"};

const int menuOptionCount = 5;
const char title[] = "== 李伟峰梦境大冒险 ==";

Level gameLevel = MEDIUM;

// 主菜单帧逻辑
void RunMenuFrame(GameState *state)
{
    static int selectedOption = 0;

    // 处理输入
    if (_kbhit())
    {
        char key = _getch();
        if (key == 'w' || key == 'W') // 上箭头
        {
            selectedOption = (selectedOption - 1 + menuOptionCount) % menuOptionCount;
        }
        else if (key == 's' || key == 'S') // 下箭头
        {
            selectedOption = (selectedOption + 1) % menuOptionCount;
        }
        else if (key == '\r') // 确认
        {
            switch (selectedOption)
            {
            case 0:
                *state = STATE_PLAYING;
                break;
            case 1:
                *state = STATE_SETTINGS;
                break;
            case 2:
                *state = STATE_Score;
                break;
            case 3:
                *state = STATE_INTRO;
                break;
            case 4:
                *state = STATE_EXIT;
                break;
            }
        }
    }

    // 绘制菜单
    DrawStrCenter(8, title);
    DrawStrCenter(10, "使用 W/S 键移动光标, 回车键确认");

    char s[120] = "当前用户: ";
    strcat(s, users[currentUserIndex].username);

    strcat(s, " | 当前难度: ");

    switch (gameLevel)
    {
    case EASY:
        strcat(s, "简单");
        break;
    case MEDIUM:
        strcat(s, "中等");
        break;
    case HARD:
        strcat(s, "困难");
        break;
    }

    DrawStrCenter(12, s);

    int startY = 16; // 定义菜单起始Y坐标
    for (int i = 0; i < menuOptionCount; ++i)
    {
        if (i == selectedOption)
        {
            DrawStrCenter(startY + i * 2, selectedMenuOptions[i]);
        }
        else
        {
            DrawStrCenter(startY + i * 2, menuOptions[i]);
        }
    }
}

bool isGameInitialized = false;

// 初始化游戏
void InitializeGame()
{
    InitGameLogic();
}

// 游戏中帧逻辑
void RunGameFrame(GameState *state)
{
    // 初始化 (仅进行一次)
    if (!isGameInitialized)
    {
        InitializeGame();
        isGameInitialized = true;
    }

    // 更新游戏逻辑 (输入、移动、战斗、AI)
    bool isAlive = UpdateGameLogic(); // 返回玩家是否存活

    // 渲染游戏画面 (地图、实体、UI)
    RenderGameLogic();

    // 死亡结算处理
    if (!isAlive)
    {
        // 覆盖绘制“游戏结束”界面
        
        // 绘制弹窗背景
        for(int y=7; y<=15; y++) {
            DrawStr(MAP_WIDTH / 2 - 12, y, "                              "); // 清空中间区域
        }

        DrawStr(MAP_WIDTH / 2 - 12, 8, "==========================");
        DrawStr(MAP_WIDTH / 2 - 10, 10, "      GAME OVER         ");
        DrawStr(MAP_WIDTH / 2 - 12, 12, "==========================");
        
        char scoreMsg[64];
        sprintf(scoreMsg, "本次生存: %d 秒", survivalSeconds);
        DrawStr(MAP_WIDTH / 2 - 10, 14, scoreMsg);
        
        DrawStr(MAP_WIDTH / 2 - 10, 20, "按任意键查看榜单...");

        // 强制立即刷新屏幕，让玩家看到 Game Over
        FlushScreen();

        // 防止玩家在死前疯狂按键导致误触跳过结算
        Sleep(1000); 
        while (_kbhit()) _getch(); // 清空键盘缓冲区

        // 等待用户按键
        _getch();

        // === 分数结算 ===
        // 如果本次分数比历史最高分高，则更新
        if (survivalSeconds > users[currentUserIndex].score) {
            users[currentUserIndex].score = survivalSeconds;
        }
        
        // 保存数据到文件
        SaveUserData();

        *state = STATE_Score; // 跳转到分数榜
        
        // 重置初始化标记
        isGameInitialized = false; 
    }
}

char* settingsOptions[] = {
    "简单",
    "中等",
    "困难",
    "返回"};
    
char* settingsSelectedOptions[] = {
    "> 简单 <",
    "> 中等 <",
    "> 困难 <",
    "> 返回 <"};

int settingsOptionCount = 4;

// 设置界面帧逻辑
void RunSettingsFrame(GameState *state)
{
    static int selectedOption = 0;

    // 处理输入
    if (_kbhit())
    {
        char key = _getch();
        if (key == 'w' || key == 'W') // 上箭头
        {
            selectedOption = (selectedOption - 1 + 4) % 4;
        }
        else if (key == 's' || key == 'S') // 下箭头
        {
            selectedOption = (selectedOption + 1) % 4;
        }
        else if (key == '\r') // 确认
        {
            if (selectedOption != 3)
            {
                gameLevel = (Level)selectedOption;
            }

            *state = STATE_MENU;
        }
    }

    // 绘制菜单
    DrawStrCenter(8, "== 设置游戏难度 ==");
    DrawStrCenter(10, "使用 W/S 键移动光标, 回车键确认");

    int startY = 16; // 定义菜单起始Y坐标
    for (int i = 0; i < settingsOptionCount; ++i)
    {
        if (i == selectedOption)
        {
            DrawStrCenter(startY + i * 2, settingsSelectedOptions[i]);
        }
        else
        {
            DrawStrCenter(startY + i * 2, settingsOptions[i]);
        }
    }
}

bool isSorted = false;

// 分数榜帧逻辑
void RunScoreFrame(GameState *state)
{
    DrawStrCenter(5, "== 分数榜 ==");

    if (!isSorted)
    {
        // 显示前10名
        // 冒泡排序
        for (int i = 0; i < userCount - 1; i++)
        {
            for (int j = 0; j < userCount - i - 1; j++)
            {
                if (users[j].score < users[j + 1].score)
                {
                    if (j + 1 == currentUserIndex)
                        currentUserIndex = j;
                    else if (j == currentUserIndex)
                        currentUserIndex = j + 1;
                    
                    UserData temp = users[j];
                    users[j] = users[j + 1];
                    users[j + 1] = temp;
                }
            }
        }

        isSorted = true;
    }

    int displayCount = userCount < MAX_SCORES ? userCount : MAX_SCORES;
    for (int i = 0; i < displayCount; i++)
    {
        char scoreEntry[100];
        sprintf(scoreEntry, "%d. %s - %d 分", i + 1, users[i].username, users[i].score);
        DrawStrCenter(8 + i * 2, scoreEntry);
    }

    DrawStrCenter(28, "按任意键返回主菜单");

    // 等待用户按键返回
    if (_kbhit())
    {
        _getch(); // 清除按键
        *state = STATE_MENU;
    }
}

// Game.c - 在文件末尾，RunScoreFrame函数之后添加


// 游戏介绍帧逻辑
void RunIntroFrame(GameState *state)
{
    static IntroPage currentPage = INTRO_PAGE_BACKGROUND;
    

// 绘制标题
DrawStrCenter(2, "========================================");
DrawStrCenter(3, "             《李伟峰梦游记》游戏介绍");

char titleLine[100];
if (currentPage == INTRO_PAGE_BACKGROUND)
    sprintf(titleLine, "【游戏背景与目标】");
else
    sprintf(titleLine, "【游戏内容介绍】");
    
DrawStrCenter(4, titleLine);
DrawStrCenter(5, "========================================");

int lineY = 7;

if (currentPage == INTRO_PAGE_BACKGROUND)
{
    // 背景介绍内容
    DrawStrCenter(lineY++, "================= 游戏背景 =================");
    lineY++;
    DrawStrCenter(lineY++, "在游戏中你将扮演主角：李伟峰");
    DrawStrCenter(lineY++, "李伟峰做了一个奇怪的梦，梦中许多害怕的事物变成了怪物");
    DrawStrCenter(lineY++, "同时也有许多喜欢的事物出现，帮助他抵御攻击");
    DrawStrCenter(lineY++, "你的任务：收集喜欢的道具，同时对怪物进行反击");
    lineY++;
    DrawStrCenter(lineY++, "================= 游戏规则 =================");
    lineY++;
    DrawStrCenter(lineY++, "• 每过7秒：场上刷新随机道具");
    DrawStrCenter(lineY++, "• 每过6秒：刷新一个怪物");
    DrawStrCenter(lineY++, "• 头号敌人：重修（BOSS级怪物）");
    DrawStrCenter(lineY++, "• 重要规则：一定要远离床！！！");
    DrawStrCenter(lineY++, "  （对李伟峰来说，床十分危险！）");
    lineY++;
    DrawStrCenter(lineY++, "================= 温馨提示 =================");
    lineY++;
    DrawStrCenter(lineY++, "当你遇到困难时，想想李伟峰会怎么做");
    DrawStrCenter(lineY++, "开始你的梦境之旅吧！");
    DrawStrCenter(lineY++, "本作创作目的：缅怀我们的李伟峰同学");
    DrawStrCenter(lineY++, "本作为3人小组作业，如有雷同，纯属他人抄袭");
    DrawStrCenter(lineY++, "更多游戏内容，敬请期待后续更新...");
}
else
{
    // 内容介绍
    DrawStrCenter(lineY++, "================= 游戏主角 =================");
    lineY++;
    DrawStrCenter(lineY++, "• 李伟峰");
    DrawStrCenter(lineY++, "  攻击力：10 | 生命值：100 | 移动速度：2.5");
    lineY++;
    DrawStrCenter(lineY++, "================= 道具介绍 =================");
    lineY++;
    DrawStrCenter(lineY++, "1. 闹钟：    下一个怪物刷新时间增加5秒 (低概率)");
    DrawStrCenter(lineY++, "2. 普通枕头：  增加5点攻击力");
    DrawStrCenter(lineY++, "3. 舒适枕头：  增加15点攻击力");
    DrawStrCenter(lineY++, "4. 黄金枕头：  增加50点攻击力 (低概率)");
    DrawStrCenter(lineY++, "5. 食堂包子：  增加5点生命值");
    DrawStrCenter(lineY++, "6. 食堂套餐烤肉：增加10点生命值");
    DrawStrCenter(lineY++, "7. 烤肉：    增加50点生命值 (低概率)");
    DrawStrCenter(lineY++, "8. 手表：    增加0.25点移动速度");
    DrawStrCenter(lineY++, "9. 电脑：    增加0.5点移动速度 (低概率)");
    DrawStrCenter(lineY++, "10. 手机：   增加1点移动速度 (低概率)");
    DrawStrCenter(lineY++, "11. 宿舍钥匙： 10秒内无视场景障碍 (低概率)");
    DrawStrCenter(lineY++, "12. 吹风机：  30秒内获得远程攻击 (攻击力减半)");
    lineY++;
    
    // 确保不会超出屏幕，分页显示怪物
    if (lineY + 15 > SCREEN_HEIGHT) {
        lineY = 7; // 重置位置，或者你可以添加分页逻辑
    }
    
    DrawStrCenter(lineY++, "================= 怪物介绍 =================");
    lineY++;
    DrawStrCenter(lineY++, "【普通怪物】");
    DrawStrCenter(lineY++, "1. 早八：     攻击5  | 生命80  | 速度1.0");
    DrawStrCenter(lineY++, "2. 晚自习：   攻击7  | 生命100 | 速度1.0");
    DrawStrCenter(lineY++, "3. 周六早八：  攻击5  | 生命120 | 速度1.0");
    DrawStrCenter(lineY++, "4. 周日晚自习：攻击10 | 生命120 | 速度1.25");
    lineY++;
    DrawStrCenter(lineY++, "【精英怪物】");
    DrawStrCenter(lineY++, "5. 体测：     攻击20 | 生命220 | 速度1.5");
    DrawStrCenter(lineY++, "6. 考试：     攻击25 | 生命200 | 速度2.25");
    DrawStrCenter(lineY++, "7. 挂科：     攻击22 | 生命240 | 速度3.25");
    lineY++;
    DrawStrCenter(lineY++, "【特殊怪物】");
    DrawStrCenter(lineY++, "8. 假期：     攻击5  | 生命300 | 速度0.5");
    DrawStrCenter(lineY++, "  (击败高概率掉落低爆率道具)");
    DrawStrCenter(lineY++, "【BOSS】");
    DrawStrCenter(lineY++, "9. 重修：     攻击40 | 生命1000 | 速度10.5");
}
    
    // 绘制操作说明
    lineY = lineY + 2;
    DrawStrCenter(lineY++, "========================================");
    DrawStrCenter(lineY++, "操作说明：");
    DrawStrCenter(lineY++, "A/D键：切换页面 | 回车/ESC键：返回主菜单");
    DrawStrCenter(lineY++, "========================================");
    
    // 处理输入（不直接改变状态，让函数持续运行）
    if (_kbhit())
    {
        char key = _getch();
        if (key == 'a' || key == 'A' || key == 'd' || key == 'D')
        {
            // A/D键切换页面
            currentPage = (currentPage == INTRO_PAGE_BACKGROUND) ? 
                          INTRO_PAGE_CONTENT : INTRO_PAGE_BACKGROUND;
        }
        else if (key == '\r' || key == 27) // 回车或ESC键返回主菜单
        {
            *state = STATE_MENU;
        }
    }
}