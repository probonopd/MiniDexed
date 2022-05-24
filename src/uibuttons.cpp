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
:	m_Pin (nPin, GPIOModeInputPullUp),
	m_nLastValue (0)
{
}

CUIButton::~CUIButton (void)
{
}

boolean CUIButton::Initialize (void)
{
	return TRUE;
}

#define DEBOUNCER 50

boolean CUIButton::Read (void)
{
	unsigned nValue = m_Pin.Read();
	
	if (nValue != 0)
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


CUIButtons::CUIButtons (unsigned nLeftPin, unsigned nRightPin, unsigned nUpPin, unsigned nDownPin, unsigned nSelectPin)
:	m_LeftButton (nLeftPin),
	m_RightButton (nRightPin),
	m_UpButton (nUpPin),
	m_DownButton (nDownPin),
	m_SelectButton (nSelectPin)
{
}

CUIButtons::~CUIButtons (void)
{
}

boolean CUIButtons::Initialize (void)
{
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

	if (m_LeftButton.Read ())
	{
		LOGNOTE ("Left");
		(*m_pEventHandler) (BtnEventLeft, m_pEventParam);
	}
	if (m_RightButton.Read ())
	{
		LOGNOTE ("Right");
		(*m_pEventHandler) (BtnEventRight, m_pEventParam);
	}
	if (m_UpButton.Read ())
	{
		LOGNOTE ("Up");
		(*m_pEventHandler) (BtnEventUp, m_pEventParam);
	}
	if (m_DownButton.Read ())
	{
		LOGNOTE ("Down");
		(*m_pEventHandler) (BtnEventDown, m_pEventParam);
	}
	if (m_SelectButton.Read ())
	{
		LOGNOTE ("Select");
		(*m_pEventHandler) (BtnEventSelect, m_pEventParam);
	}
}

