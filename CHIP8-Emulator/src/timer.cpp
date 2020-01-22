#include "timer.h"
#include <SDL.h>

Timer::Timer() {
	stop();
}

void Timer::start() {
	m_status = TIMER_STARTED;
	m_startTicks = SDL_GetTicks();
	m_pausedTicks = 0;
}

void Timer::stop() {
	m_status = TIMER_STOPPED;
	m_startTicks = 0;
	m_pausedTicks = 0;
}

void Timer::pause() {
	if (m_status == TIMER_STARTED) {
		m_status = TIMER_PAUSED;

		// Calculate the moment the timer was paused
		m_pausedTicks = SDL_GetTicks() - m_startTicks;
		m_startTicks = 0;
	}
}

void Timer::unpause() {
	if (m_status == TIMER_PAUSED) {
		m_status = TIMER_STARTED;

		// Reset starting ticks
		m_startTicks = SDL_GetTicks() - m_pausedTicks;

		m_pausedTicks = 0;
	}
}

unsigned long Timer::getTicks() {
	switch (m_status) {
	case TIMER_STARTED:
		return SDL_GetTicks() - m_startTicks;
	case TIMER_PAUSED:
		return m_pausedTicks;

	default:
		return 0;
	}
}