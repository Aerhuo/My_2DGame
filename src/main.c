#include <stdio.h>
#include <windows.h>
#include "Renderer.h"
#include "Game.h"
#include "DataManager.h"

int main()
{
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
        Sleep(33); // 30 FPS
    }

    SaveUserData();
}