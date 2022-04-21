//
// pckeyboard.cpp
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
#include "pckeyboard.h"
#include <circle/devicenameservice.h>
#include <circle/util.h>
#include <assert.h>

struct TKeyInfo
{
	char	KeyCode;	// upper case letter or digit
	u8	KeyNumber;	// MIDI number
};

// KeyCode is valid for standard QWERTY keyboard
static TKeyInfo KeyTable[] =
{
	{',', 72}, // C4
	{'M', 71}, // B4
	{'J', 70}, // A#4
	{'N', 69}, // A4
	{'H', 68}, // G#3
	{'B', 67}, // G3
	{'G', 66}, // F#3
	{'V', 65}, // F3
	{'C', 64}, // E3
	{'D', 63}, // D#3
	{'X', 62}, // D3
	{'S', 61}, // C#3
	{'Z', 60}, // C3
	{'U', 59}, // B3
	{'7', 58}, // A#3
	{'Y', 57}, // A3
	{'6', 56}, // G#2
	{'T', 55}, // G2
	{'5', 54}, // F#2
	{'R', 53}, // F2
	{'E', 52}, // E2
	{'3', 51}, // D#2
	{'W', 50}, // D2
	{'2', 49}, // C#2
	{'Q', 48}  // C2
};

CPCKeyboard *CPCKeyboard::s_pThis = 0;

CPCKeyboard::CPCKeyboard (CMiniDexed *pSynthesizer, CConfig *pConfig)
:	CMIDIDevice (pSynthesizer, pConfig),
	m_pKeyboard (0)
{
	s_pThis = this;

	memset (m_LastKeys, 0, sizeof m_LastKeys);

	AddDevice ("ukbd1");
}

CPCKeyboard::~CPCKeyboard (void)
{
	s_pThis = 0;
}

void CPCKeyboard::Process (boolean bPlugAndPlayUpdated)
{
	if (!bPlugAndPlayUpdated)
	{
		return;
	}

	if (m_pKeyboard == 0)
	{
		m_pKeyboard =
			(CUSBKeyboardDevice *) CDeviceNameService::Get ()->GetDevice ("ukbd1", FALSE);
		if (m_pKeyboard != 0)
		{
			m_pKeyboard->RegisterKeyStatusHandlerRaw (KeyStatusHandlerRaw);

			m_pKeyboard->RegisterRemovedHandler (DeviceRemovedHandler);
		}
	}
}

void CPCKeyboard::KeyStatusHandlerRaw (unsigned char ucModifiers, const unsigned char RawKeys[6])
{
	assert (s_pThis != 0);

	// report released keys
	for (unsigned i = 0; i < 6; i++)
	{
		u8 ucKeyCode = s_pThis->m_LastKeys[i];
		if (   ucKeyCode != 0
		    && !FindByte (RawKeys, ucKeyCode, 6))
		{
			u8 ucKeyNumber = GetKeyNumber (ucKeyCode);
			if (ucKeyNumber != 0)
			{
				u8 NoteOff[] = {0x80, ucKeyNumber, 0};
				s_pThis->MIDIMessageHandler (NoteOff, sizeof NoteOff);
			}
		}
	}

	// report pressed keys
	for (unsigned i = 0; i < 6; i++)
	{
		u8 ucKeyCode = RawKeys[i];
		if (   ucKeyCode != 0
		    && !FindByte (s_pThis->m_LastKeys, ucKeyCode, 6))
		{
			u8 ucKeyNumber = GetKeyNumber (ucKeyCode);
			if (ucKeyNumber != 0)
			{
				u8 NoteOn[] = {0x90, ucKeyNumber, 100};
				s_pThis->MIDIMessageHandler (NoteOn, sizeof NoteOn);
			}
		}
	}

	memcpy (s_pThis->m_LastKeys, RawKeys, sizeof s_pThis->m_LastKeys);
}

u8 CPCKeyboard::GetKeyNumber (u8 ucKeyCode)
{
	char chKey;
	if (0x04 <= ucKeyCode && ucKeyCode <= 0x1D)
	{
		chKey = ucKeyCode-'\x04'+'A';	// key code of 'A' is 0x04
	}
	else if (0x1E <= ucKeyCode && ucKeyCode <= 0x26)
	{
		chKey = ucKeyCode-'\x1E'+'1';	// key code of '1' is 0x1E
	}
	else if (ucKeyCode == 0x36)
	{
		chKey = ',';			// key code of ',' is 0x36
	}
	else
	{
		return 0;
	}

	for (unsigned i = 0; i < sizeof KeyTable / sizeof KeyTable[0]; i++)
	{
		if (KeyTable[i].KeyCode == chKey)
		{
			return KeyTable[i].KeyNumber;
		}
	}

	return 0;
}

boolean CPCKeyboard::FindByte (const u8 *pBuffer, u8 ucByte, unsigned nLength)
{
	while (nLength-- > 0)
	{
		if (*pBuffer++ == ucByte)
		{
			return TRUE;
		}
	}

	return FALSE;
}

void CPCKeyboard::DeviceRemovedHandler (CDevice *pDevice, void *pContext)
{
	assert (s_pThis != 0);
	s_pThis->m_pKeyboard = 0;
}
