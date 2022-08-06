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

CUserInterface::CUserInterface (CMiniDexed *pMiniDexed, CGPIOManager *pGPIOManager, CI2CMaster *pI2CMaster, CConfig *pConfig)
:	m_pMiniDexed (pMiniDexed),
	m_pGPIOManager (pGPIOManager),
	m_pI2CMaster (pI2CMaster),
	m_pConfig (pConfig),
	m_pLCD (0),
	m_pLCDBuffered (0),
	m_pUIButtons (0),
	m_pRotaryEncoder (0),
	m_bSwitchPressed (false),
	m_Menu (this, pMiniDexed)
{
}

CUserInterface::~CUserInterface (void)
{
	delete m_pRotaryEncoder;
	delete m_pUIButtons;
	delete m_pLCDBuffered;
	delete m_pLCD;
}

bool CUserInterface::Initialize (void)
{
	assert (m_pConfig);

	if (m_pConfig->GetLCDEnabled ())
	{
		unsigned i2caddr = m_pConfig->GetLCDI2CAddress ();
		unsigned ssd1306addr = m_pConfig->GetSSD1306LCDI2CAddress ();
		if (ssd1306addr != 0) {
			m_pSSD1306 = new CSSD1306Device (m_pConfig->GetSSD1306LCDWidth (), m_pConfig->GetSSD1306LCDHeight (), m_pI2CMaster, ssd1306addr);
			LOGDBG ("LCD: SSD1306");
			if (!m_pSSD1306->Initialize ())
			{
				return false;
			}
			m_pLCD = m_pSSD1306;
		} else if (i2caddr == 0)
		{
			m_pHD44780 = new CHD44780Device (m_pConfig->GetLCDColumns (), m_pConfig->GetLCDRows (),
							 m_pConfig->GetLCDPinData4 (),
							 m_pConfig->GetLCDPinData5 (),
							 m_pConfig->GetLCDPinData6 (),
							 m_pConfig->GetLCDPinData7 (),
							 m_pConfig->GetLCDPinEnable (),
							 m_pConfig->GetLCDPinRegisterSelect (),
							 m_pConfig->GetLCDPinReadWrite ());
			LOGDBG ("LCD: HD44780");
			if (!m_pHD44780->Initialize ())
			{
				return false;
			}
			m_pLCD = m_pHD44780;
		}
		else
		{
			m_pHD44780 = new CHD44780Device (m_pI2CMaster, i2caddr,
							m_pConfig->GetLCDColumns (), m_pConfig->GetLCDRows ());
			LOGDBG ("LCD: HD44780 I2C");
			if (!m_pHD44780->Initialize ())
			{
				return false;
			}
			m_pLCD = m_pHD44780;
		}
		assert (m_pLCD);

		m_pLCDBuffered = new CWriteBufferDevice (m_pLCD);
		assert (m_pLCDBuffered);

		LCDWrite ("\x1B[?25l\x1B""d+");		// cursor off, autopage mode

		LOGDBG ("LCD initialized");
	}

	m_pUIButtons = new CUIButtons (	m_pConfig->GetButtonPinPrev (),
									m_pConfig->GetButtonActionPrev (),
									m_pConfig->GetButtonPinNext (),
									m_pConfig->GetButtonActionNext (),
									m_pConfig->GetButtonPinBack (),
									m_pConfig->GetButtonActionBack (),
									m_pConfig->GetButtonPinSelect (),
									m_pConfig->GetButtonActionSelect (),
									m_pConfig->GetButtonPinHome (),
									m_pConfig->GetButtonActionHome (),
									m_pConfig->GetDoubleClickTimeout (),
									m_pConfig->GetLongPressTimeout () );
	assert (m_pUIButtons);

	if (!m_pUIButtons->Initialize ())
	{
		return false;
	}

	m_pUIButtons->RegisterEventHandler (UIButtonsEventStub, this);

	LOGDBG ("Button User Interface initialized");

	if (m_pConfig->GetEncoderEnabled ())
	{
		m_pRotaryEncoder = new CKY040 (m_pConfig->GetEncoderPinClock (),
					       m_pConfig->GetEncoderPinData (),
					       m_pConfig->GetButtonPinShortcut (),
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
	if (m_pUIButtons)
	{
		m_pUIButtons->Update();
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

	CString Msg ("\x1B[H\E[?25l");		// cursor home and off

	// first line
	Msg.Append (pParam);

	size_t nLen = strlen (pParam) + strlen (pMenu);
	if (nLen < m_pConfig->GetLCDColumns ())
	{
		for (unsigned i = m_pConfig->GetLCDColumns ()-nLen; i > 0; i--)
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
		if (Value.GetLength () < m_pConfig->GetLCDColumns ()-1)
		{
			for (unsigned i = m_pConfig->GetLCDColumns ()-Value.GetLength ()-1; i > 0; i--)
			{
				Value.Append (" ");
			}
		}

		Value.Append ("\x7E");		// arrow right character
	}

	Msg.Append (Value);

	if (Value.GetLength () < m_pConfig->GetLCDColumns ())
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
		if (m_bSwitchPressed) {
			// We must reset the encoder switch button to prevent events from being
			// triggered after the encoder is rotated
			m_pUIButtons->ResetButton(m_pConfig->GetButtonPinShortcut());
			m_Menu.EventHandler(CUIMenu::MenuEventPressAndStepUp);

		}
		else {
			m_Menu.EventHandler(CUIMenu::MenuEventStepUp);
		}
		break;

	case CKY040::EventCounterclockwise:
		if (m_bSwitchPressed) {
			m_pUIButtons->ResetButton(m_pConfig->GetButtonPinShortcut());
			m_Menu.EventHandler(CUIMenu::MenuEventPressAndStepDown);
		}
		else {
			m_Menu.EventHandler(CUIMenu::MenuEventStepDown);
		}
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

void CUserInterface::UIButtonsEventHandler (CUIButton::BtnEvent Event)
{
	switch (Event)
	{
	case CUIButton::BtnEventPrev:
		m_Menu.EventHandler (CUIMenu::MenuEventStepDown);
		break;

	case CUIButton::BtnEventNext:
		m_Menu.EventHandler (CUIMenu::MenuEventStepUp);
		break;

	case CUIButton::BtnEventBack:
		m_Menu.EventHandler (CUIMenu::MenuEventBack);
		break;

	case CUIButton::BtnEventSelect:
		m_Menu.EventHandler (CUIMenu::MenuEventSelect);
		break;

	case CUIButton::BtnEventHome:
		m_Menu.EventHandler (CUIMenu::MenuEventHome);
		break;

	default:
		break;
	}
}

void CUserInterface::UIButtonsEventStub (CUIButton::BtnEvent Event, void *pParam)
{
	CUserInterface *pThis = static_cast<CUserInterface *> (pParam);
	assert (pThis != 0);

	pThis->UIButtonsEventHandler (Event);
}
