//
// perftimer.h
//
#ifndef _perftimer_h
#define _perftimer_h

#include <string>
#include <circle/timer.h>

class CPerformanceTimer
{
public:
	CPerformanceTimer (const char *pName, unsigned nDeadlineMicros = 0);

	void Start (void);
	void Stop (void);

	void Dump (unsigned nIntervalTicks = CLOCKHZ);

private:
	std::string m_Name;
	unsigned m_nDeadlineMicros;

	unsigned m_nStartTicks;
	unsigned m_nMaximumMicros;

	unsigned m_nLastDumpTicks;
};

#endif
