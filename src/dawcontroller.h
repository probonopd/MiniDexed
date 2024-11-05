// MiniDexed - Dexed FM synthesizer for bare metal Raspberry Pi
// Copyright (C) 2024 The MiniDexed Team
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
#ifndef _dawcontroller_h
#define _dawcontroller_h

#include <circle/types.h>
#include "uimenu.h"

class CMIDIKeyboard;
class CMiniDexed;
class CDAWConnection;
class CConfig;
class CUserInterface;

class CDAWController
{
public:
	CDAWController (CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard, CConfig *pConfig, CUserInterface *pUI);
	~CDAWController (void);

	void OnConnect (void);
	void MIDISysexHandler (u8 *pPacket, unsigned nLength, unsigned nCable);

	void DisplayWrite (const char *pMenu, const char *pParam, const char *pValue,
			   bool bArrowDown, bool bArrowUp);
	
	void UpdateState (void);
	void UpdateMenu (CUIMenu::TCPageType Type, s8 ucPage, u8 ucOP, u8 ucTG);

	void MIDIListener (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2);

private:
	CMiniDexed *m_pSynthesizer;
	CMIDIKeyboard *m_pKeyboard;
	CConfig *m_pConfig;
	CUserInterface *m_pUI;
	CDAWConnection *m_pDAWConnection;
};

#endif
