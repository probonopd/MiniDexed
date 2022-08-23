//
// midipin.cpp
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
#include "midipin.h"
#include <circle/logger.h>
#include <assert.h>

LOGMODULE ("midipin");

CMIDIPin::CMIDIPin (unsigned nPinNumber)
:	m_nPinNumber (nPinNumber),
	m_nValue (HIGH)
{
}

CMIDIPin::~CMIDIPin (void)
{
}
		
unsigned CMIDIPin::Read (void)
{
	return m_nValue;
}

void CMIDIPin::Write (unsigned nValue)
{
	// Takes values in the MIDI controller range 0 to 127
	// and OFF < 64 < ON.
	// Simulates a PULLUP IO pin, so "true" is LOW (0)
	if (nValue >= 64) {
		// "on"
		m_nValue = LOW;
	} else {
		// "off"
		m_nValue = HIGH;
	}
	return;
}

