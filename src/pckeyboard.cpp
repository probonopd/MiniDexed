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
#include <circle/startup.h>

struct TKeyInfo
{
	char	KeyCode;	// upper case letter or digit
	u8	KeyNumber;	// MIDI number
};

// KeyCode is valid for standard QWERTY keyboard
// selected octave gets added to these
// This is the default.
// Any PCKeyNote<x> entries in minidexed.ini invalidate the entire table
static TKeyInfo KeyTable[] =
{
	{KEY_Q, 36}, // C1
	{KEY_2, 37},
	{KEY_W, 38},
	{KEY_3, 39},
	{KEY_E, 40},
	{KEY_R, 41}, // F1
	{KEY_5, 42},
	{KEY_T, 43},
	{KEY_6, 44},
	{KEY_Y, 45},
	{KEY_7, 46},
	{KEY_U, 47},
	{KEY_I, 48}, // C2
	{KEY_9, 49},
	{KEY_O, 50},
	{KEY_0, 51},
	{KEY_P, 52},
	{KEY_LEFTBRACE, 53},
	{KEY_EQUAL, 54},
	{KEY_RIGHTBRACE, 55}, // G2
	{KEY_BACKSLASH, 57}, // A2

	{KEY_Z, 24},  // C0
	{KEY_S, 25},
	{KEY_X, 26},
	{KEY_D, 27},
	{KEY_C, 28},
	{KEY_V, 29},  // F0
	{KEY_G, 30},
	{KEY_B, 31},
	{KEY_H, 32},
	{KEY_N, 33},
	{KEY_J, 34},
	{KEY_M, 35},
	{KEY_COMMA, 36},  // C1
	{KEY_L, 37},
	{KEY_DOT, 38},
	{KEY_SEMICOLON, 39},
	{KEY_SLASH, 40}
};

CPCKeyboard *CPCKeyboard::s_pThis = 0;

CPCKeyboard::CPCKeyboard (CMiniDexed *pSynthesizer, CConfig *pConfig, CUserInterface *pUI)
:	CMIDIDevice (pSynthesizer, pConfig, pUI),
	m_pConfig (pConfig),
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

#define CC_OCTAVE_UP	128
#define CC_OCTAVE_DOWN	129
#define CC_NOTES_OFF	130
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
			u8 ucKeyNumber = s_pThis->KeyCodeToNote (ucKeyCode);
			u8 ucKeyCC = s_pThis->KeyCodeToCC (ucKeyCode);
			// first see if this key should send a note on
			if ( (ucKeyNumber > 0) && (ucKeyNumber <= 127) )
			{
				u8 NoteOff[] = {0x80, ucKeyNumber, 0};
				s_pThis->MIDIMessageHandler (NoteOff, sizeof NoteOff);
			}
			// then check if it should send a CC
			else if ( (ucKeyCC > 0) && (ucKeyCC <= 127) )
			{
				u8 ButtonOn[] = {0xb0, ucKeyCC, 0};
				s_pThis->MIDIMessageHandler (ButtonOn, sizeof ButtonOn);
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
			u8 ucKeyNumber = s_pThis->KeyCodeToNote (ucKeyCode);
			u8 ucKeyCC = s_pThis->KeyCodeToCC (ucKeyCode);
			// first see if this key should send a note on
			if ( (ucKeyNumber > 0) && (ucKeyNumber <= 127) )
			{
				u8 NoteOn[] = {0x90, ucKeyNumber, 100};
				s_pThis->MIDIMessageHandler (NoteOn, sizeof NoteOn);
			}
			// then check if it should send a CC
			else if ( (ucKeyCC > 0) && (ucKeyCC <= 127) )
			{
				u8 ButtonOn[] = {0xb0, ucKeyCC, 100};
				s_pThis->MIDIMessageHandler (ButtonOn, sizeof ButtonOn);
			}
			else if (ucKeyCC == CC_OCTAVE_UP)
			{
				if (++s_pThis->octave > 5) s_pThis->octave = 5;
			}
			else if (ucKeyCC == CC_OCTAVE_DOWN)
			{
				if (s_pThis->octave > 0) s_pThis->octave--;
			}
			else if (ucKeyCC == CC_NOTES_OFF)
			{
				u8 NoteOff[] = {0x80, 60, 0};
				for (u8 i=0; i<128; i++)
				{
					NoteOff[1] = i;
					s_pThis->MIDIMessageHandler (NoteOff, sizeof NoteOff);
				}
			}
			else if (ucKeyCode == KEY_DELETE)
			{
				if ((ucModifiers & KEY_MOD_LCTRL) && (ucModifiers & KEY_MOD_LALT))
				{
					CLogger::Get ()->Write("keyboard", LogNotice, "Ctrl+Alt+Del detected! Rebooting system...");
					// Safely request Circle to trigger a hardware reset
					reboot();
				}
			}
		}
	}

	memcpy (s_pThis->m_LastKeys, RawKeys, sizeof s_pThis->m_LastKeys);
}

u8 CPCKeyboard::KeyCodeToCC (u8 ucKeyCode)
{
	return(m_pConfig->GetPCKeyCC(ucKeyCode));
}

u8 CPCKeyboard::KeyCodeToNote (u8 ucKeyCode)
{
	u8 baseNote = 0;
	if (m_pConfig->GetPCKeyUseDefaultNotes() )
	{
		for (unsigned i = 0; i < sizeof KeyTable / sizeof KeyTable[0]; i++)
		{
			if (KeyTable[i].KeyCode == ucKeyCode)
			{
				baseNote = KeyTable[i].KeyNumber;
			}
		}
	} else {
		baseNote = m_pConfig->GetPCKeyNote(ucKeyCode);
	}

	if (baseNote) baseNote += (s_pThis->octave*12);
	return baseNote;
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
