//
// pckeyboard.h
//
// MiniSynth Pi - A virtual analogue synthesizer for Raspberry Pi
// Copyright (C) 2017-2020  R. Stange <rsta2@o2online.de>
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
#ifndef _pckeyboard_h
#define _pckeyboard_h

#include "mididevice.h"
#include "config.h"
#include <circle/usb/usbkeyboard.h>
#include <circle/device.h>
#include <circle/types.h>

class CMiniDexed;

class CPCKeyboard : public CMIDIDevice
{
public:
	CPCKeyboard (CMiniDexed *pSynthesizer, CConfig *pConfig, CUserInterface *pUI);
	~CPCKeyboard (void);

	void Process (boolean bPlugAndPlayUpdated);

private:
	static void KeyStatusHandlerRaw (unsigned char ucModifiers, const unsigned char RawKeys[6]);

	static u8 GetKeyNumber (u8 ucKeyCode);

	static boolean FindByte (const u8 *pBuffer, u8 ucByte, unsigned nLength);

	static void DeviceRemovedHandler (CDevice *pDevice, void *pContext);

private:
	CUSBKeyboardDevice * volatile m_pKeyboard;

	u8 m_LastKeys[6];

	static CPCKeyboard *s_pThis;
};

#endif
