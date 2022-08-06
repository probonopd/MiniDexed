//
// userinterface.h
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
#ifndef _userinterface_h
#define _userinterface_h

#include "config.h"
#include "uimenu.h"
#include "uibuttons.h"
#include <sensor/ky040.h>
#include <display/hd44780device.h>
#include <display/ssd1306device.h>
#include <circle/gpiomanager.h>
#include <circle/writebuffer.h>
#include <circle/i2cmaster.h>

class CMiniDexed;

class CUserInterface
{
public:
	CUserInterface (CMiniDexed *pMiniDexed, CGPIOManager *pGPIOManager, CI2CMaster *pI2CMaster, CConfig *pConfig);
	~CUserInterface (void);

	bool Initialize (void);

	void Process (void);

	void ParameterChanged (void);

	// Write to display in this format:
	// +----------------+
	// |PARAM       MENU|
	// |[<]VALUE     [>]|
	// +----------------+
	void DisplayWrite (const char *pMenu, const char *pParam, const char *pValue,
			   bool bArrowDown, bool bArrowUp);

private:
	void LCDWrite (const char *pString);		// Print to optional HD44780 display

	void EncoderEventHandler (CKY040::TEvent Event);
	static void EncoderEventStub (CKY040::TEvent Event, void *pParam);
	void UIButtonsEventHandler (CUIButton::BtnEvent Event);
	static void UIButtonsEventStub (CUIButton::BtnEvent Event, void *pParam);

private:
	CMiniDexed *m_pMiniDexed;
	CGPIOManager *m_pGPIOManager;
	CI2CMaster *m_pI2CMaster;
	CConfig *m_pConfig;

	CCharDevice    *m_pLCD;
	CHD44780Device *m_pHD44780;
	CSSD1306Device *m_pSSD1306;
	CWriteBufferDevice *m_pLCDBuffered;
	
	CUIButtons *m_pUIButtons;

	CKY040 *m_pRotaryEncoder;
	bool m_bSwitchPressed;

	CUIMenu m_Menu;
};

#endif
