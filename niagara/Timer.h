#ifndef TIMER_H
#define TIMER_H

#if defined(ARDUINO)
#include <Arduino.h>
#else
#include <chrono>
#endif

/**
 * Class which handles timing functions
 */
class Timer {
public:
    Timer() {
        start();
    }

    void start() {
#if defined(ARDUINO)
        _startTime = millis();
#else
        _startTime = std::chrono::steady_clock::now();
#endif
    }

    // Returns elapsed time in milliseconds
    unsigned long elapsed() const {
#if defined(ARDUINO)
        return millis() - _startTime;
#else
        auto now = std::chrono::steady_clock::now();
        return static_cast<unsigned long>(
            std::chrono::duration_cast<std::chrono::milliseconds>(now - _startTime).count()
        );
#endif
    }

private:
#if defined(ARDUINO)
    unsigned long _startTime;
#else
    std::chrono::steady_clock::time_point _startTime;
#endif
};

#endif // TIMER_H
