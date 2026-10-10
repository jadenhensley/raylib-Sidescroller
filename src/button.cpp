#pragma once
#include "button.h"

void Button::Draw() {
    if (!initialized) return;
    DrawRectangle(window.x, window.y, window.width, window.height, colorBg);
    bool b_doOffsetText = (window.height > 40);
    int offsetX = b_doOffsetText ? (window.width / 8.0f) : 4.0f;
    int offsetY = b_doOffsetText ? (window.height / 2.0f) : 4.0f;
    DrawText(text.c_str(), window.x + offsetX, window.y + offsetY, txtSize, colorTxt);
}
bool Button::GetClickInWindow(bool click) { // note: click should be handled by parent dialogue's state. then passed in to the button.
    if (!initialized) return false;
    Vector2 mousePos = GetMousePosition();
    if (CheckCollisionPointRec(mousePos, window)) {
        if (click) return true;
    };
    return false;
}


