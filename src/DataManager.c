#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "DataManager.h"

const char* USER_FILE = "users.txt";

UserData* users = NULL;
int userCount = 0;

// 加载用户数据
void LoadUserData()
{
    FILE* file = fopen(USER_FILE, "r");
    if (!file) return;

    fscanf(file, "%d\n", &userCount);
    users = (UserData*)malloc(sizeof(UserData) * userCount);

    for (int i = 0; i < userCount; i++)
    {
        fscanf(file, "%s %s %d\n", users[i].username, users[i].password, &users[i].score);
    }

    fclose(file);
}

// 保存用户数据
void SaveUserData()
{
    FILE* file = fopen(USER_FILE, "w");
    if (!file) return;

    fprintf(file, "%d\n", userCount);
    for (int i = 0; i < userCount; i++)
    {
        fprintf(file, "%s %s %d\n", users[i].username, users[i].password, users[i].score);
    }

    fclose(file);
}

// 用户登录
bool LoginUser(const char* username, const char* password)
{
    for (int i = 0; i < userCount; i++)
    {
        if (strcmp(users[i].username, username) == 0 &&
            strcmp(users[i].password, password) == 0)
        {
            return true;
        }
    }
    return false;
}

// 用户注册
bool RegisterUser(const char* username, const char* password)
{
    for (int i = 0; i < userCount; i++)
    {
        if (strcmp(users[i].username, username) == 0)
        {
            return false; // 用户名已存在
        }
    }

    users = (UserData*)realloc(users, sizeof(UserData) * (userCount + 1));
    strcpy(users[userCount].username, username);
    strcpy(users[userCount].password, password);
    users[userCount].score = 0;
    userCount++;

    SaveUserData();
    return true;
}