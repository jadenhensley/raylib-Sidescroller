#include "timer.h"

void Timer::tick() {
    if (timeLeft > 0.0) {
        timeLeft -= GetFrameTime();
        if (timeLeft < 0.0) timeLeft = 0.0;
    }
    else {
        if (autostart) start();
    }
}

bool Timer::hasEnded() const {
    return (timeLeft <= 0.0);
}

void Timer::start() {
    timeLeft = durationSeconds;
}

float Timer::getTimeLeft() const {
    return timeLeft;
}

void Timer::setDuration(float _durSec) {
    durationSeconds = _durSec;
    return;
}

float Timer::getDuration() const {
    return durationSeconds;
}

float Timer::getNormalizedProgress() const {
    return 1.0 - (getTimeLeft() / getDuration());
}
