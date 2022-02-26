//
// perftimer.cpp
//
#include "perftimer.h"
#include <iostream>

CPerformanceTimer::CPerformanceTimer (const char *pName, unsigned nDeadlineMicros)
:	m_Name (pName),
	m_nDeadlineMicros (nDeadlineMicros),
	m_nMaximumMicros (0),
	m_nLastDumpTicks (0)
{
}

void CPerformanceTimer::Start (void)
{
	m_nStartTicks = CTimer::GetClockTicks ();
}

void CPerformanceTimer::Stop (void)
{
	unsigned nEndTicks = CTimer::GetClockTicks ();
	unsigned nMicros = (nEndTicks - m_nStartTicks) / (CLOCKHZ / 1000000);

	if (nMicros > m_nMaximumMicros)
	{
		m_nMaximumMicros = nMicros;
	}
}

void CPerformanceTimer::Dump (unsigned nIntervalTicks)
{
	unsigned nTicks = CTimer::GetClockTicks ();

	if (nTicks - m_nLastDumpTicks >= nIntervalTicks)
	{
		m_nLastDumpTicks = nTicks;

		unsigned nMaximumMicros = m_nMaximumMicros;	// may be overwritten from interrupt

		std::cout << m_Name << ": Maximum duration was " << nMaximumMicros <<  "us";

		if (m_nDeadlineMicros != 0)
		{
			std::cout << " (" << nMaximumMicros*100 / m_nDeadlineMicros << "%)";
		}

		std::cout << std::endl;
	}
}
