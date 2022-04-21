//
// serialmididevice.h
//
// MiniDexed - Dexed FM synthesizer for bare metal Raspberry Pi
// Copyright (C) 2022  The MiniDexed Team
//
// Original author of this class:
//	R. Stange <rsta2@o2online.de>
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
#ifndef _serialmididevice_h
#define _serialmididevice_h

#include "mididevice.h"
#include "config.h"
#include <circle/interrupt.h>
#include <circle/serial.h>
#include <circle/writebuffer.h>
#include <circle/types.h>

class CMiniDexed;

class CSerialMIDIDevice : public CMIDIDevice
{
public:
	CSerialMIDIDevice (CMiniDexed *pSynthesizer, CInterruptSystem *pInterrupt, CConfig *pConfig);
	~CSerialMIDIDevice (void);

	boolean Initialize (void);

	void Process (void);

	void Send (const u8 *pMessage, size_t nLength, unsigned nCable = 0) override;

private:
	CConfig *m_pConfig;

	CSerialDevice m_Serial;
	unsigned m_nSerialState;
	u8 m_SerialMessage[3];

	CWriteBufferDevice m_SendBuffer;
};

#endif
