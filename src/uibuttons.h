//
// uibuttons.h
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
#ifndef _uibuttons_h
#define _uibuttons_h

#include <circle/gpiopin.h>
#include <circle/types.h>
#include "config.h"

class CUIButton
{
public:
	CUIButton (unsigned nPin);
	~CUIButton (void);
	
	boolean Initialize (void);
	
	boolean Read (void);

private:
	CGPIOPin m_Pin;
	unsigned m_nLastValue;
};


class CUIButtons
{
public:
	enum TBtnEvent
	{
		BtnEventLeft,
		BtnEventRight,
		BtnEventUp,
		BtnEventDown,
		BtnEventSelect,
		BtnEventUnknown
	};
	
	typedef void TBtnEventHandler (TBtnEvent Event, void *pParam);

public:
	CUIButtons (unsigned nLeftPin, unsigned nRightPin, unsigned nUpPin, unsigned nDownPin, unsigned nSelectPin);
	~CUIButtons (void);
	
	boolean Initialize (void);
	
	void RegisterEventHandler (TBtnEventHandler *pHandler, void *pParam = 0);
	
	void Update (void);
	
private:
	CUIButton m_LeftButton;
	CUIButton m_RightButton;
	CUIButton m_UpButton;
	CUIButton m_DownButton;
	CUIButton m_SelectButton;

	TBtnEventHandler *m_pEventHandler;
	void *m_pEventParam;
};

#endif
