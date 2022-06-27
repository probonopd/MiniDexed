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

#define BUTTONS_UPDATE_NUM_TICKS 100
#define DEBOUNCE_TIME 100
#define MAX_BUTTONS 5

class CUIButtons;

class CUIButton
{
public:
	enum BtnTrigger
	{
		BtnTriggerNone = 0,
		BtnTriggerClick = 1,
		BtnTriggerDoubleClick = 2,
		BtnTriggerLongPress = 3
	};

	enum BtnEvent
	{
		BtnEventNone = 0,
		BtnEventPrev = 1,
		BtnEventNext = 2,
		BtnEventBack = 3,
		BtnEventSelect = 4,
		BtnEventHome = 5,
		BtnEventUnknown = 6
	};
	
	CUIButton (void);
	~CUIButton (void);
	
	void reset (void);
	boolean Initialize (unsigned pinNumber, unsigned doubleClickTimeout, unsigned longPressTimeout);

	void setClickEvent(BtnEvent clickEvent);
	void setDoubleClickEvent(BtnEvent doubleClickEvent);
	void setLongPressEvent(BtnEvent longPressEvent);

	unsigned getPinNumber(void);
	
	BtnTrigger ReadTrigger (void);
	BtnEvent Read (void);

	static BtnTrigger triggerTypeFromString(const char* triggerString);

private:
	// Pin number
	unsigned m_pinNumber;
	// GPIO pin
	CGPIOPin *m_pin;
	// The value of the pin at the end of the last loop
	unsigned m_lastValue;
	// Set to 0 on press, increment each read, use to trigger events
	uint16_t m_timer;
	// Debounce timer
	uint16_t m_debounceTimer;
	// Number of clicks recorded since last timer reset
	uint8_t m_numClicks;
	// Event to fire on click
	BtnEvent m_clickEvent;
	// Event to fire on double click
	BtnEvent m_doubleClickEvent;
	// Event to fire on long press
	BtnEvent m_longPressEvent;
	
	// Timeout for double click in tenths of a millisecond
	unsigned m_doubleClickTimeout;
	// Timeout for long press in tenths of a millisecond
	unsigned m_longPressTimeout;
};

class CUIButtons
{
public:
	typedef void BtnEventHandler (CUIButton::BtnEvent Event, void *param);

public:
	CUIButtons (
			unsigned prevPin, const char *prevAction,
			unsigned nextPin, const char *nextAction,
			unsigned backPin, const char *backAction,
			unsigned selectPin, const char *selectAction,
			unsigned homePin, const char *homeAction,
			unsigned doubleClickTimeout, unsigned longPressTimeout
	);
	~CUIButtons (void);
	
	boolean Initialize (void);
	
	void RegisterEventHandler (BtnEventHandler *handler, void *param = 0);
	
	void Update (void);

	void ResetButton (unsigned pinNumber);
	
private:
	// Array of 5 buttons
	CUIButton m_buttons[MAX_BUTTONS];
	
	// Timeout for double click in tenths of a millisecond
	unsigned m_doubleClickTimeout;
	// Timeout for long press in tenths of a millisecond
	unsigned m_longPressTimeout;
	
	// Configuration for buttons
	unsigned m_prevPin;
	CUIButton::BtnTrigger m_prevAction;
	unsigned m_nextPin;
	CUIButton::BtnTrigger m_nextAction;
	unsigned m_backPin;
	CUIButton::BtnTrigger m_backAction;
	unsigned m_selectPin;
	CUIButton::BtnTrigger m_selectAction;
	unsigned m_homePin;
	CUIButton::BtnTrigger m_homeAction;

	BtnEventHandler *m_eventHandler;
	void *m_eventParam;

	unsigned m_lastTick;

	void bindButton(unsigned pinNumber, CUIButton::BtnTrigger trigger, CUIButton::BtnEvent event);
};

#endif
