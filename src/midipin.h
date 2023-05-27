//
// midipin.h
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
#ifndef _midipin_h
#define _midipin_h

#include <circle/gpiopin.h>
#include <circle/types.h>

// MIDI CC numbers go 0 to 127.
// NB: 0 is treated as "unused" so CC=0 won't work
// Normal GPIO pins are below 100.
// So use a "pin number" of 128 + MIDI CC message for a "MIDI Pin"
#define MIDI_PINS 128
#define ccToMidiPin(c) (((c)==0)?0:((c)+MIDI_PINS))
#define MidiPinToCC(p) (((p)>=MIDI_PINS)?((p)-MIDI_PINS):0)
#define isMidiPin(p)   (((p)>=MIDI_PINS)?1:0)

class CMIDIPin
{
public:
	CMIDIPin (unsigned nPinNumber);  // pinNumber = ccToMidiPin (MIDI CC number)
	~CMIDIPin (void);

	// Will return MP_HIGH or MP_LOW.
	// Should be treated as a PULLED UP IO pin
	// i.e. treated as "active low" (LOW) when pressed.
	unsigned Read (void);
	
	// MIDI CC values >=64 will set the MIDI pin to LOW ("on")
	// MIDI CC values <= 63 will set the MIDI pin to HIGH ("off")
	void Write (unsigned nValue);

private:
	unsigned m_nPinNumber;
	unsigned m_nValue;
};

#endif
