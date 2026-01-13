#include <windows.h>
#include <time.h>
#include "Renderer.h"
#include "Game.h"
#include "DataManager.h"
#include "GameData.h"

int main()
{
    // 设置随机数种子
    srand((unsigned int)time(NULL));

    LoadUserData();
    Login();

    InitConsole();
    GameState currentState = STATE_MENU;

    while (currentState != STATE_EXIT)
    {
        ClearScreen();

        switch (currentState)
        {
            case STATE_MENU:
            RunMenuFrame(&currentState);
            break;

            case STATE_SETTINGS:
            RunSettingsFrame(&currentState);
            break;

            case STATE_PLAYING:
            RunGameFrame(&currentState);
            break;

            case STATE_Score:
            RunScoreFrame(&currentState);
            break;
        }

        FlushScreen();
        Sleep(1000 / FPS);
    }

    SaveUserData();
}
