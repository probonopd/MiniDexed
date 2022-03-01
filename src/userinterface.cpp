//
// userinterface.cpp
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
#include "userinterface.h"
#include "minidexed.h"
#include <circle/logger.h>
#include <circle/string.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

LOGMODULE ("ui");

CUserInterface::CUserInterface (CMiniDexed *pMiniDexed, CConfig *pConfig)
:	m_pMiniDexed (pMiniDexed),
	m_pConfig (pConfig),
	m_pLCD (0),
	m_pLCDBuffered (0)
{
}

CUserInterface::~CUserInterface (void)
{
	delete m_pLCDBuffered;
	delete m_pLCD;
}

bool CUserInterface::Initialize (void)
{
	assert (m_pConfig);

	if (m_pConfig->GetLCDEnabled ())
	{
		m_pLCD = new CHD44780Device (CConfig::LCDColumns, CConfig::LCDRows,
					     m_pConfig->GetLCDPinData4 (),
					     m_pConfig->GetLCDPinData5 (),
					     m_pConfig->GetLCDPinData6 (),
					     m_pConfig->GetLCDPinData7 (),
					     m_pConfig->GetLCDPinEnable (),
					     m_pConfig->GetLCDPinRegisterSelect (),
					     m_pConfig->GetLCDPinReadWrite ());
		assert (m_pLCD);

		if (!m_pLCD->Initialize ())
		{
			return false;
		}

		m_pLCDBuffered = new CWriteBufferDevice (m_pLCD);
		assert (m_pLCDBuffered);

		LCDWrite ("\x1B[?25l");		// cursor off

		LOGDBG ("LCD initialized");
	}

	return true;
}

void CUserInterface::Process (void)
{
	if (m_pLCDBuffered)
	{
		m_pLCDBuffered->Update ();
	}
}

void CUserInterface::ProgramChanged (unsigned nProgram)
{
	nProgram++;	// MIDI numbering starts with 0, user interface with 1

	// fetch program name from Dexed instance
	char ProgramName[11];
	memset (ProgramName, 0, sizeof ProgramName);
	assert (m_pMiniDexed);
	m_pMiniDexed->setName (ProgramName);

	printf ("Loading voice %u: \"%s\"\n", nProgram, ProgramName);

	CString String;
	String.Format ("\n\r%u\n\r%s", nProgram, ProgramName);
	LCDWrite (String);
}

void CUserInterface::LCDWrite (const char *pString)
{
	if (m_pLCDBuffered)
	{
		m_pLCDBuffered->Write (pString, strlen (pString));
	}
}
