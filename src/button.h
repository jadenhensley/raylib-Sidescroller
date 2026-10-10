#pragma once
#include <iostream>
#include <string>
#include "raylib.h"

struct Button {
    std::string text;
    Rectangle window;
    Color colorBg;
    Color colorTxt;
    int txtSize;
    bool initialized = false;
    Button(std::string argText, float posX, float posY, float width, float height, \
        Color argColorBg, Color argColorTxt, int argTxtSize)\
        :text(argText), window{ posX, posY, width, height }, colorBg(argColorBg), colorTxt(argColorTxt), txtSize(argTxtSize)
    {
        initialized = true;
    }
    void Draw();
    bool GetClickInWindow(bool click); // note: click should be handled by parent dialogue's state. then passed in to the button.
};

