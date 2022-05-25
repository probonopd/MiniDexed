//
// uibuttons.cpp
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
#include "uibuttons.h"
#include <circle/logger.h>
#include <assert.h>

LOGMODULE ("uibuttons");

CUIButton::CUIButton (unsigned nPin)
:	m_nPin (nPin),
	m_pPin (0),
	m_nLastValue (0)
{
}

CUIButton::~CUIButton (void)
{
	if (m_pPin)
	{
		delete m_pPin;
	}
}

boolean CUIButton::Initialize (void)
{
	assert (!m_pPin);

	if (m_nPin != NOPIN)
	{
		m_pPin = new CGPIOPin (m_nPin, GPIOModeInputPullUp);
	}
	return TRUE;
}

#define DEBOUNCER 50

boolean CUIButton::Read (void)
{
	if (!m_pPin)
	{
		// Always return "not pressed" if not configured
		return FALSE;
	}

	unsigned nValue = m_pPin->Read();
	
	// Buttons in PULL UP mode are "active low"
	if (nValue == 0)
	{
		// Some simple debouncing...
		if (m_nLastValue < DEBOUNCER)
		{
			m_nLastValue++;
		}
		else if (m_nLastValue == DEBOUNCER)
		{
			m_nLastValue++;
			return TRUE;
		}
		else
		{
			// Do nothing until reset/cleared
		}
	}
	else
	{
		m_nLastValue = 0;
	}

	return FALSE;
}


CUIButtons::CUIButtons (unsigned nPrevPin, unsigned nNextPin, unsigned nBackPin, unsigned nSelectPin, unsigned nHomePin)
:	m_PrevButton (nPrevPin),
	m_NextButton (nNextPin),
	m_BackButton (nBackPin),
	m_SelectButton (nSelectPin),
	m_HomeButton (nHomePin)
{
}

CUIButtons::~CUIButtons (void)
{
}

boolean CUIButtons::Initialize (void)
{
	m_PrevButton.Initialize ();
	m_NextButton.Initialize ();
	m_BackButton.Initialize ();
	m_SelectButton.Initialize ();
	m_HomeButton.Initialize ();

	return TRUE;
}

void CUIButtons::RegisterEventHandler (TBtnEventHandler *pHandler, void *pParam)
{
	assert (!m_pEventHandler);
	m_pEventHandler = pHandler;
	assert (m_pEventHandler);
	m_pEventParam = pParam;
}

void CUIButtons::Update (void)
{
	assert (m_pEventHandler);

	if (m_PrevButton.Read ())
	{
		LOGDBG ("Prev");
		(*m_pEventHandler) (BtnEventPrev, m_pEventParam);
	}
	if (m_NextButton.Read ())
	{
		LOGDBG ("Next");
		(*m_pEventHandler) (BtnEventNext, m_pEventParam);
	}
	if (m_BackButton.Read ())
	{
		LOGDBG ("Back");
		(*m_pEventHandler) (BtnEventBack, m_pEventParam);
	}
	if (m_SelectButton.Read ())
	{
		LOGDBG ("Select");
		(*m_pEventHandler) (BtnEventSelect, m_pEventParam);
	}
	if (m_HomeButton.Read ())
	{
		LOGDBG ("Home");
		(*m_pEventHandler) (BtnEventHome, m_pEventParam);
	}
}

