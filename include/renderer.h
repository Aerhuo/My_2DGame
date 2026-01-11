#ifndef RENDERER_H
#define RENDERER_H

#define SCREEN_WIDTH 120
#define SCREEN_HEIGHT 30

// 初始化控制台（设置画布大小、隐藏光标）
void InitConsole();

// 清空画布
void ClearScreen();

// 在画布的 (x, y) 写入字符
void DrawChar(int x, int y, char c);

// 在画布的(x, y) 写入字符串
void DrawStr(int x, int y, const char* str);

// 在画布绘制居中文本
void DrawStrCenter(int y, const char* str);

// 绘制画布
void FlushScreen();

#endif