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
#include <circle/startup.h>
#include <string.h>
#include <assert.h>

LOGMODULE ("ui");

CUserInterface::CUserInterface (CMiniDexed *pMiniDexed, CGPIOManager *pGPIOManager, CConfig *pConfig)
:	m_pMiniDexed (pMiniDexed),
	m_pGPIOManager (pGPIOManager),
	m_pConfig (pConfig),
	m_pLCD (0),
	m_pLCDBuffered (0),
	m_pRotaryEncoder (0),
	m_bSwitchPressed (false),
	m_Menu (this, pMiniDexed)
{
}

CUserInterface::~CUserInterface (void)
{
	delete m_pRotaryEncoder;
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

		LCDWrite ("\x1B[?25l\x1B""d+");		// cursor off, autopage mode

		LOGDBG ("LCD initialized");
	}

	if (m_pConfig->GetEncoderEnabled ())
	{
		m_pRotaryEncoder = new CKY040 (m_pConfig->GetEncoderPinClock (),
					       m_pConfig->GetEncoderPinData (),
					       m_pConfig->GetEncoderPinSwitch (),
					       m_pGPIOManager);
		assert (m_pRotaryEncoder);

		if (!m_pRotaryEncoder->Initialize ())
		{
			return false;
		}

		m_pRotaryEncoder->RegisterEventHandler (EncoderEventStub, this);

		LOGDBG ("Rotary encoder initialized");
	}

	m_Menu.EventHandler (CUIMenu::MenuEventUpdate);

	return true;
}

void CUserInterface::Process (void)
{
	if (m_pLCDBuffered)
	{
		m_pLCDBuffered->Update ();
	}
}

void CUserInterface::ParameterChanged (void)
{
	m_Menu.EventHandler (CUIMenu::MenuEventUpdate);
}

void CUserInterface::DisplayWrite (const char *pMenu, const char *pParam, const char *pValue,
				   bool bArrowDown, bool bArrowUp)
{
	assert (pMenu);
	assert (pParam);
	assert (pValue);

	CString Msg ("\x1B[H");		// cursor home

	// first line
	Msg.Append (pParam);

	size_t nLen = strlen (pParam) + strlen (pMenu);
	if (nLen < CConfig::LCDColumns)
	{
		for (unsigned i = CConfig::LCDColumns-nLen; i > 0; i--)
		{
			Msg.Append (" ");
		}
	}

	Msg.Append (pMenu);

	// second line
	CString Value (" ");
	if (bArrowDown)
	{
		Value = "\x7F";			// arrow left character
	}

	Value.Append (pValue);

	if (bArrowUp)
	{
		if (Value.GetLength () < CConfig::LCDColumns-1)
		{
			for (unsigned i = CConfig::LCDColumns-Value.GetLength ()-1; i > 0; i--)
			{
				Value.Append (" ");
			}
		}

		Value.Append ("\x7E");		// arrow right character
	}

	Msg.Append (Value);

	if (Value.GetLength () < CConfig::LCDColumns)
	{
		Msg.Append ("\x1B[K");		// clear end of line
	}

	LCDWrite (Msg);
}

void CUserInterface::LCDWrite (const char *pString)
{
	if (m_pLCDBuffered)
	{
		m_pLCDBuffered->Write (pString, strlen (pString));
	}
}

void CUserInterface::EncoderEventHandler (CKY040::TEvent Event)
{
	switch (Event)
	{
	case CKY040::EventSwitchDown:
		m_bSwitchPressed = true;
		break;

	case CKY040::EventSwitchUp:
		m_bSwitchPressed = false;
		break;

	case CKY040::EventClockwise:
		m_Menu.EventHandler (m_bSwitchPressed ? CUIMenu::MenuEventPressAndStepUp
						      : CUIMenu::MenuEventStepUp);
		break;

	case CKY040::EventCounterclockwise:
		m_Menu.EventHandler (m_bSwitchPressed ? CUIMenu::MenuEventPressAndStepDown
						      : CUIMenu::MenuEventStepDown);
		break;

	case CKY040::EventSwitchClick:
		m_Menu.EventHandler (CUIMenu::MenuEventBack);
		break;

	case CKY040::EventSwitchDoubleClick:
		m_Menu.EventHandler (CUIMenu::MenuEventSelect);
		break;

	case CKY040::EventSwitchTripleClick:
		m_Menu.EventHandler (CUIMenu::MenuEventHome);
		break;

	case CKY040::EventSwitchHold:
		if (m_pRotaryEncoder->GetHoldSeconds () >= 120)
		{
			delete m_pLCD;		// reset LCD

			reboot ();
		}
		break;

	default:
		break;
	}
}

void CUserInterface::EncoderEventStub (CKY040::TEvent Event, void *pParam)
{
	CUserInterface *pThis = static_cast<CUserInterface *> (pParam);
	assert (pThis != 0);

	pThis->EncoderEventHandler (Event);
}
