#ifndef TIMER_H
#define TIMER_H

//void timer_reset();
//double timer_get(); // in seconds

#ifdef WIN32

static LARGE_INTEGER _timer_start = { 0 };
static double _timer_fequency = 0;
static inline void timer_reset() {
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	_timer_fequency = (double)frequency.QuadPart;
	QueryPerformanceCounter(&_timer_start);
}
static inline double timer_get() {
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	return (now.QuadPart - _timer_start.QuadPart) / _timer_fequency;
}

#else

#include <time.h>
static clock_t _timer_start = 0;
static inline void timer_reset() { _timer_start = clock(); }
static inline double timer_get() { return (clock() - _timer_start) / (double)CLOCKS_PER_SEC; }

#endif

#endif