#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#define MAX_NAME_LEN 20
#define MAX_PWD_LEN 20
#define MAX_SCORES 10

#include <stdbool.h>

typedef struct
{
    char username[MAX_NAME_LEN];
    char password[MAX_PWD_LEN];
    int score;
} UserData;

extern UserData* users;
extern int userCount;

// 加载用户数据
void LoadUserData();

// 保存用户数据
void SaveUserData();

// 用户登录
bool LoginUser(const char* username, const char* password);

// 用户注册
bool RegisterUser(const char* username, const char* password);

#endif