#include "stdio.h"
#include "raylib.h"
#include "../src/button.h"
#pragma once

class util_printui
{
public:
    int uiSpawnAnchorX = 0;
    int uiSpawnAnchorY = 0;
    int uiLowerYBounds = GetScreenHeight();
    int uiOffsetX = 0;
    int uiOffsetY = 0;
    int uiLongestWidth = 0;
    int uiTallestHeight= 0;
    int storedFontSize = 24;
    Color currentColor = BLACK;

    util_printui(const int &argSpawnX = 0, const int &argSpawnY = 0)
        : uiSpawnAnchorX(argSpawnX), uiSpawnAnchorY(argSpawnY) {}

    void setColor(Color argColor)
    {
        currentColor = argColor;
    }

    void setFontSize(const int& argSize)
    {
        storedFontSize = argSize;
    }

    void resetui()
    {
        uiOffsetX = 0;
        uiOffsetY = 0;
        uiLongestWidth = 0;
        uiTallestHeight= 0;
        currentColor = BLACK;
    }
    /*can pass in pixels, which is basically font size or height.*/
    void bumpui(const int& pixels=24)
    {
        printui("", pixels);
    }
    void uinextH()
    {
        // note: use with caution. can also manually set these since they are public members.
        uiOffsetX += uiLongestWidth;
        uiOffsetY = 0;
    }
    void uinextV()
    {
        // note: use with caution. can also manually set these since they are public members.
        uiOffsetX = 0;
        uiOffsetY = uiTallestHeight;
    }
    /* if needed, returns the screen position of the end of the string.*/
    Vector2 printui(const char* txt, int fontSize = 24)
    {
        int usedSize = fontSize;
        if (storedFontSize != fontSize)
        {
            usedSize = storedFontSize;
        }

        int lineWidth = MeasureText(txt, usedSize);
        if (lineWidth > uiLongestWidth)
        {
            uiLongestWidth = lineWidth;
        };
        if (uiSpawnAnchorY + uiOffsetY + usedSize > uiLowerYBounds)
        {
            uinextH();
        }

        DrawText(txt, uiSpawnAnchorX + uiOffsetX, uiSpawnAnchorY + uiOffsetY, usedSize, currentColor);

        uiOffsetY += usedSize;
        if (uiOffsetY > uiTallestHeight)
        {
            uiTallestHeight = uiOffsetY;
        }

        return { static_cast<float>(uiSpawnAnchorX) + static_cast<float>(uiOffsetX) + lineWidth,
                 static_cast<float>(uiSpawnAnchorY) + static_cast<float>(uiOffsetY) - usedSize
               };
    }
    // let us use overloaded functions.
    void printui(const size_t& argInteger)
    {
        printui(TextFormat("%d", argInteger), 24);
    }
    void printui(const int& argInteger)
    {
        printui(TextFormat("%d", argInteger), 24);
    }
    void printui(const float& argFloat)
    {
        printui(TextFormat("%f", argFloat), 24);
    }

    /* allows you to advance the UI. returns the previous position. useful for Button or Texture logic.*/
    Vector2 advanceByRect(const Rectangle& rect)
    {
        int heightOffset = static_cast<int>(rect.y + rect.height);
        int widthOffset = static_cast<int>(rect.x + rect.width);

        if (widthOffset > uiLongestWidth)
        {
            uiLongestWidth = widthOffset;
        };
        if (uiSpawnAnchorY + uiOffsetY + heightOffset > uiLowerYBounds)
        {
            uinextH();
        }

        Vector2 retValue = { rect.x + uiSpawnAnchorX + uiOffsetX, rect.y + uiSpawnAnchorY + uiOffsetY };
        // set it before we increment.

        uiOffsetY += heightOffset;
        if (uiOffsetY > uiTallestHeight)
        {
            uiTallestHeight = uiOffsetY;
        }
        // return it after we increment so the code functions the same.
        return retValue;
    }

};

