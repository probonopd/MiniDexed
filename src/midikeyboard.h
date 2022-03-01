//
// midikeyboard.h
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
#ifndef _midikeyboard_h
#define _midikeyboard_h

#include "mididevice.h"
#include "config.h"
#include <circle/usb/usbmidi.h>
#include <circle/device.h>
#include <circle/types.h>

class CMiniDexed;

class CMIDIKeyboard : public CMIDIDevice
{
public:
	CMIDIKeyboard (CMiniDexed *pSynthesizer, CConfig *pConfig);
	~CMIDIKeyboard (void);

	void Process (boolean bPlugAndPlayUpdated);

private:
	static void MIDIPacketHandler (unsigned nCable, u8 *pPacket, unsigned nLength);

	static void DeviceRemovedHandler (CDevice *pDevice, void *pContext);

private:
	CUSBMIDIDevice * volatile m_pMIDIDevice;

	static CMIDIKeyboard *s_pThis;
};

#endif
