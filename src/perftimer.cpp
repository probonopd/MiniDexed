//
// perftimer.cpp
//
// MiniDexed - Dexed FM synthesizer for bare metal Raspberry Pi
// Copyright (C) 2022  The MiniDexed Team
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
	m_nLastMicros = (nEndTicks - m_nStartTicks) / (CLOCKHZ / 1000000);

	if (m_nLastMicros > m_nMaximumMicros)
	{
		m_nMaximumMicros = m_nLastMicros;
	}
}

void CPerformanceTimer::Dump (unsigned nIntervalTicks)
{
	unsigned nTicks = CTimer::GetClockTicks ();

	if (nTicks - m_nLastDumpTicks >= nIntervalTicks)
	{
		m_nLastDumpTicks = nTicks;

		unsigned nMaximumMicros = m_nMaximumMicros;	// may be overwritten from interrupt
		unsigned nLastMicros = m_nLastMicros;

		std::cout << m_Name << ": Last duration was " << nLastMicros << "us";

		if (m_nDeadlineMicros != 0)
		{
			std::cout << " (" << nLastMicros*100 / m_nDeadlineMicros << "%)";
		}

		std::cout << " Maximum was " << nMaximumMicros << "us";

		if (m_nDeadlineMicros != 0)
		{
			std::cout << " (" << nMaximumMicros*100 / m_nDeadlineMicros << "%)";
		}

		std::cout << std::endl;
	}
}
