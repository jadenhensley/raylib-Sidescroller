#pragma once
#include "raylib.h"
#include "stdio.h"
class Timer {
private:
    float durationSeconds;
    float timeLeft;
    bool autostart;
public:
    Timer(float _durSec = 0.0f, bool _autoStart = false)
        : durationSeconds(_durSec), autostart(_autoStart), timeLeft(_autoStart ? _durSec : 0.0f) {
    }
    void tick();
    bool hasEnded() const;
    void start();
    float getTimeLeft() const;
    void setDuration(float _durSec);
    float getDuration() const;

    float getNormalizedProgress() const;
};
