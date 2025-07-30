//
// perftimer.h
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
	unsigned m_nLastMicros;
	unsigned m_nMaximumMicros;

	unsigned m_nLastDumpTicks;
};

#endif
