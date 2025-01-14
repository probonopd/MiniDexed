//
// uibuttons.h
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
#ifndef _uibuttons_h
#define _uibuttons_h

#include <circle/gpiopin.h>
#include <circle/types.h>
#include "midipin.h"
#include "config.h"

#define BUTTONS_UPDATE_NUM_TICKS 100
#define DEBOUNCE_TIME 20
#define MAX_GPIO_BUTTONS 11  // 5 UI buttons, 6 Program/Bank/TG Select buttons
#define MAX_MIDI_BUTTONS 11
#define MAX_BUTTONS (MAX_GPIO_BUTTONS+MAX_MIDI_BUTTONS)

class CUIButtons;

class CUIButton
{
public:
	enum BtnTrigger
	{
		BtnTriggerNone = 0,
		BtnTriggerClick = 1,
		BtnTriggerDoubleClick = 2,
		BtnTriggerLongPress = 3,
		BtnTriggerDec = 4,
		BtnTriggerInc = 5,
	};

	enum BtnEvent
	{
		BtnEventNone = 0,
		BtnEventPrev = 1,
		BtnEventNext = 2,
		BtnEventBack = 3,
		BtnEventSelect = 4,
		BtnEventHome = 5,
		BtnEventPgmUp = 6,
		BtnEventPgmDown = 7,
		BtnEventBankUp = 8,
		BtnEventBankDown = 9,
		BtnEventTGUp = 10,
		BtnEventTGDown = 11,
		BtnEventUnknown = 12
	};
	
	CUIButton (void);
	~CUIButton (void);
	
	void reset (void);
	boolean Initialize (unsigned pinNumber, unsigned doubleClickTimeout, unsigned longPressTimeout);

	void setClickEvent(BtnEvent clickEvent);
	void setDoubleClickEvent(BtnEvent doubleClickEvent);
	void setLongPressEvent(BtnEvent longPressEvent);
	void setDecEvent(BtnEvent decEvent);
	void setIncEvent(BtnEvent incEvent);

	unsigned getPinNumber(void);
	
	BtnTrigger ReadTrigger (void);
	BtnEvent Read (void);
	void Write (unsigned nValue); // MIDI buttons only!

	static BtnTrigger triggerTypeFromString(const char* triggerString);
	
private:
	// Pin number
	unsigned m_pinNumber;
	// GPIO pin
	CGPIOPin *m_pin;
	// MIDI pin
	CMIDIPin *m_midipin;
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
	// Event to fire on dec
	BtnEvent m_decEvent;
	// Event to fire on inc
	BtnEvent m_incEvent;

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
	CUIButtons (CConfig *pConfig);
	~CUIButtons (void);
	
	boolean Initialize (void);
	
	void RegisterEventHandler (BtnEventHandler *handler, void *param = 0);
	
	void Update (void);

	void ResetButton (unsigned pinNumber);

	void BtnMIDICmdHandler (unsigned nMidiCmd, unsigned nMidiData1, unsigned nMidiData2);
	
private:
	CConfig *m_pConfig;

	// Array of normal GPIO buttons and "MIDI buttons"
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
	
	// Program and TG Selection buttons
	unsigned m_pgmUpPin;
	CUIButton::BtnTrigger m_pgmUpAction;
	unsigned m_pgmDownPin;
	CUIButton::BtnTrigger m_pgmDownAction;
	unsigned m_BankUpPin;
	CUIButton::BtnTrigger m_BankUpAction;
	unsigned m_BankDownPin;
	CUIButton::BtnTrigger m_BankDownAction;
	unsigned m_TGUpPin;
	CUIButton::BtnTrigger m_TGUpAction;
	unsigned m_TGDownPin;
	CUIButton::BtnTrigger m_TGDownAction;
	
	// MIDI button configuration
	unsigned m_notesMidi;

	unsigned m_prevMidi;
	CUIButton::BtnTrigger m_prevMidiAction;
	unsigned m_nextMidi;
	CUIButton::BtnTrigger m_nextMidiAction;
	unsigned m_backMidi;
	CUIButton::BtnTrigger m_backMidiAction;
	unsigned m_selectMidi;
	CUIButton::BtnTrigger m_selectMidiAction;
	unsigned m_homeMidi;
	CUIButton::BtnTrigger m_homeMidiAction;
	
	unsigned m_pgmUpMidi;
	CUIButton::BtnTrigger m_pgmUpMidiAction;
	unsigned m_pgmDownMidi;
	CUIButton::BtnTrigger m_pgmDownMidiAction;
	unsigned m_BankUpMidi;
	CUIButton::BtnTrigger m_BankUpMidiAction;
	unsigned m_BankDownMidi;
	CUIButton::BtnTrigger m_BankDownMidiAction;
	unsigned m_TGUpMidi;
	CUIButton::BtnTrigger m_TGUpMidiAction;
	unsigned m_TGDownMidi;
	CUIButton::BtnTrigger m_TGDownMidiAction;

	BtnEventHandler *m_eventHandler;
	void *m_eventParam;

	unsigned m_lastTick;

	void bindButton(unsigned pinNumber, CUIButton::BtnTrigger trigger, CUIButton::BtnEvent event);
};

#endif
