#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>
#include "Renderer.h"

static char screen[SCREEN_HEIGHT][SCREEN_WIDTH];

void SetCursorVisible(bool visible) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = visible;
    SetConsoleCursorInfo(hOut, &cursorInfo);
}

// 初始化控制台（设置画布大小、隐藏光标）
void InitConsole()
{
    // 设置窗口大小
    char cmd[64];
    sprintf(cmd, "mode con cols=%d lines=%d", SCREEN_WIDTH, SCREEN_HEIGHT);
    system(cmd);

    // 隐藏光标
    SetCursorVisible(false);
}

// 清空画布
void ClearScreen()
{
    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            screen[y][x] = ' ';
        }
    }
}

// 在画布的 (x, y) 写入字符
void DrawChar(int x, int y, char c)
{
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)
    {
        screen[y][x] = c;
    }
}

// 获取字符串的视觉宽度
int GetStrWidth(const char *str)
{
    int width = 0;
    int i = 0;
    while (str[i] != '\0')
    {
        // 简单处理中文字符宽度为2，英文字符宽度为1
        if (str[i] < 128)
        {
            width += 1;
            i++;
        }
        else
        {
            width += 2;
            i += 3;
        }
    }
    return width;
}

// 在画布的(x, y) 写入字符串
void DrawStr(int x, int y, const char *str)
{
    int len = strlen(str);
    for (int i = 0; i < len; ++i)
    {
        DrawChar(i + x, y, str[i]);
    }
}

// 在画布绘制居中文本
void DrawStrCenter(int y, const char *str)
{
    int len = GetStrWidth(str);
    int x = (SCREEN_WIDTH / 2) - (len / 2);
    DrawStr(x, y, str);
}

// 绘制画布
void FlushScreen()
{
    // 光标复位
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = {0, 0};
    SetConsoleCursorPosition(hOut, pos);

    // 逐行打印
    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        printf("%.*s", SCREEN_WIDTH, screen[y]);

        // 避免最后一行换行导致滚动
        if (y < SCREEN_HEIGHT - 1)
        {
            putchar('\n');
        }
    }
}
