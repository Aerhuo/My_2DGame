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
    "退出游戏"};

char *selectedMenuOptions[] = {
    "> 开始游戏 <",
    "> 设置 <",
    "> 分数榜 <",
    "> 退出游戏 <"};

const int menuOptionCount = 4;
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
        for(int y=12; y<=20; y++) {
            DrawStrCenter(y, "                              "); // 清空中间区域
        }

        DrawStrCenter(13, "==========================");
        DrawStrCenter(15, "      GAME OVER         ");
        DrawStrCenter(17, "==========================");
        
        char scoreMsg[64];
        sprintf(scoreMsg, "本次生存: %d 秒", survivalSeconds);
        DrawStrCenter(19, scoreMsg);
        
        DrawStrCenter(25, "按任意键查看榜单...");

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