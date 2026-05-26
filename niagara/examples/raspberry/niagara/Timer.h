#ifndef TIMER_H
#define TIMER_H

#if defined(ARDUINO)
#include <Arduino.h>
#else
#include <chrono>
#endif

/**
 * @file Timer.h
 * @class Timer
 * @author Zanotti Enrico
 * @brief Class which handles timing functions
 * 
 * This class allows for the creation of a `Timer` object
 * which allows for counting elapsed time since the call of
 * a `start()` function.
 * 
 * This class is cross-compatible on both Arduino and gcc platforms.
 */
class Timer {
public:
	/**
	 * Default constructor for the Timer object, which starts the
	 * timer by default on call.
	 */
	Timer() {
		start();
	}

	/**
	 * @brief Starts the timer
	 * 
	 * Sets the timer start to the moment when this function is called.
	 */
	void start() {
#if defined(ARDUINO)
		_startTime = millis();
#else
		_startTime = std::chrono::steady_clock::now();
#endif
	}

	/**
	 * @brief Computes the elapsed time.
	 * @returns Passed time in milliseconds.
	 * 
	 * This method returns the amount of time elapsed between the 
	 * call of the `start()` function (or istantiation of the `Timer` object) and
	 * the call of this function.
	 */
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
