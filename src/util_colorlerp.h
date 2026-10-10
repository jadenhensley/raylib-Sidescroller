#pragma once
#include "raylib.h"

namespace util_colorlerp
{
    inline Color ColorLerp(Color color1, Color color2, float factor)
    {
        Color result = { 0 };

        // Clamp factor between 0.0 and 1.0
        if (factor < 0.0f) factor = 0.0f;
        if (factor > 1.0f) factor = 1.0f;

        result.r = (unsigned char)((float)color1.r + (float)(color2.r - color1.r) * factor);
        result.g = (unsigned char)((float)color1.g + (float)(color2.g - color1.g) * factor);
        result.b = (unsigned char)((float)color1.b + (float)(color2.b - color1.b) * factor);
        result.a = (unsigned char)((float)color1.a + (float)(color2.a - color1.a) * factor);

        return result;
    }
};