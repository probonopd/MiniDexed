//
// uibuttons.cpp
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
#include "uibuttons.h"
#include <circle/logger.h>
#include <assert.h>
#include <circle/timer.h>
#include <string.h>

LOGMODULE ("uibuttons");

CUIButton::CUIButton (void)
:	m_pinNumber (0),
	m_pin (0),
	m_midipin (0),
	m_lastValue (1),
	m_timer (0),
	m_debounceTimer (0),
	m_numClicks (0),
	m_clickEvent(BtnEventNone),
	m_doubleClickEvent(BtnEventNone),
	m_longPressEvent(BtnEventNone),
	m_decEvent(BtnEventNone),
	m_incEvent(BtnEventNone),
	m_doubleClickTimeout(0),
	m_longPressTimeout(0)
{
}

CUIButton::~CUIButton (void)
{
	if (m_pin)
	{
		delete m_pin;
	}
	if (m_midipin)
	{
		delete m_midipin;
	}
}

void CUIButton::reset (void)
{
	m_timer = m_longPressTimeout;
	m_numClicks = 0;
}

boolean CUIButton::Initialize (unsigned pinNumber, unsigned doubleClickTimeout, unsigned longPressTimeout)
{
	assert (!m_pin);
	assert(longPressTimeout >= doubleClickTimeout);

	m_pinNumber = pinNumber;
	m_doubleClickTimeout = doubleClickTimeout;
	m_longPressTimeout = longPressTimeout;
	
	// Initialise timing values
	m_timer = m_longPressTimeout;
	m_debounceTimer = DEBOUNCE_TIME;

	if (m_pinNumber != 0)
	{
		if (isMidiPin(m_pinNumber))
		{
			LOGDBG("MIDI Button on msg: %d (%x)", MidiPinToCC(m_pinNumber), MidiPinToCC(m_pinNumber));
			m_midipin = new CMIDIPin (m_pinNumber);
		} else {
			LOGDBG("GPIO Button on pin: %d (%x)", m_pinNumber, m_pinNumber);
			m_pin = new CGPIOPin (m_pinNumber, GPIOModeInputPullUp);
		}
	}
	return TRUE;
}

void CUIButton::setClickEvent(BtnEvent clickEvent)
{
	m_clickEvent = clickEvent;
}

void CUIButton::setDoubleClickEvent(BtnEvent doubleClickEvent)
{
	m_doubleClickEvent = doubleClickEvent;
}

void CUIButton::setLongPressEvent(BtnEvent longPressEvent)
{
	m_longPressEvent = longPressEvent;
}

void CUIButton::setDecEvent(BtnEvent decEvent)
{
	m_decEvent = decEvent;
}

void CUIButton::setIncEvent(BtnEvent incEvent)
{
	m_incEvent = incEvent;
}

unsigned CUIButton::getPinNumber(void)
{
	return m_pinNumber;
}
	
CUIButton::BtnTrigger CUIButton::ReadTrigger (void)
{
	unsigned value;
	unsigned raw = 0;
	if (isMidiPin(m_pinNumber))
	{
		if (!m_midipin)
		{
			// Always return "not pressed" if not configured
			return BtnTriggerNone;
		}
		value = m_midipin->Read();
		raw = m_midipin->ReadRaw();
	}
	else
	{
		if (!m_pin)
		{
			// Always return "not pressed" if not configured
			return BtnTriggerNone;
		}
		value = m_pin->Read();
	}

	if (m_decEvent && raw < MIDIPIN_CENTER)
	{
		reset();
		// reset value to trigger only once
		m_midipin->Write(MIDIPIN_CENTER);
		return BtnTriggerDec;
	}
	else if (m_incEvent && MIDIPIN_CENTER < raw)
	{
		reset();
		// reset value to trigger only once
		m_midipin->Write(MIDIPIN_CENTER);
		return BtnTriggerInc;
	}

	if (m_timer < m_longPressTimeout) {
		m_timer++;

		if (m_timer == m_doubleClickTimeout && m_lastValue == 1 && m_numClicks == 1) {
			// The user has clicked and released the button once within the
			// timeout - this must be a single click
			reset();
			return BtnTriggerClick;
		}
		if (m_timer == m_longPressTimeout) {
			if (m_lastValue == 0 && m_numClicks == 1) {
				// Single long press
				reset();
				return BtnTriggerLongPress;
			}
			else {
				// Just reset it - we've run out of possible interactions
				reset();
			}
		}
	}

	// Debounce here - we don't need to do anything if the debounce timer is active
	if (m_debounceTimer < DEBOUNCE_TIME) {
		m_debounceTimer++;
		return BtnTriggerNone;
	}
	
	// Buttons in PULL UP mode are "active low"
	if (value == 0)
	{
		if (m_lastValue == 0) {
			// 0 -> 0 : Button is pressed, was already pressed
		}
		else {
			// 1 -> 0 : Button was not pressed but is now pressed
			m_lastValue = 0;
			m_debounceTimer = 0;

			if (m_numClicks == 0) {
				// No clicks recorded - start a new timer
				m_timer = 0;
			}
			if (m_numClicks < 2) {
				m_numClicks++;
			}
		}
	}
	else
	{
		if (m_lastValue == 1) {
			// 1 -> 1 : Button is not pressed, was already not pressed
		}
		else {
			// 0 -> 1 : Button was pressed but is now not pressed (it was released)
			m_lastValue = 1;
			m_debounceTimer = 0;

			if (m_numClicks == 1 &&
					(m_doubleClickEvent == BtnEventNone ||
					 m_timer >= m_doubleClickTimeout && m_timer < m_longPressTimeout)
			) {
				// Either the user released the button when there is no double
				// click mapped
				// OR:
				// The user released the button after the double click
				// timeout, but before the long press timeout
				reset();
				return BtnTriggerClick;
			}
			else if (m_numClicks == 2) {
				// This is the second release in a short period of time
				reset();
				return BtnTriggerDoubleClick;
			}
		}
	}

	return BtnTriggerNone;
}

void CUIButton::Write (unsigned nValue) {
	// This only makes sense for MIDI buttons.
	if (m_midipin && isMidiPin(m_pinNumber))
	{
		// Update the "MIDI Pin"
		m_midipin->Write(nValue);
	}
}

CUIButton::BtnEvent CUIButton::Read (void) {
	BtnTrigger trigger = ReadTrigger();

	if (trigger == BtnTriggerClick) {
		return m_clickEvent;
	}
	else if (trigger == BtnTriggerDoubleClick) {
		return m_doubleClickEvent;
	}
	else if (trigger == BtnTriggerLongPress) {
		return m_longPressEvent;
	}
	else if (trigger == BtnTriggerDec) {
		return m_decEvent;
	}
	else if (trigger == BtnTriggerInc) {
		return m_incEvent;
	}

	assert (trigger == BtnTriggerNone);

	return BtnEventNone;
}

CUIButton::BtnTrigger CUIButton::triggerTypeFromString(const char* triggerString)
{
	if (strcmp(triggerString, "") == 0 || strcmp(triggerString, "none") == 0) {
		return BtnTriggerNone;
	}
	else if (strcmp(triggerString, "click") == 0) {
		return BtnTriggerClick;
	}	
	else if (strcmp(triggerString, "doubleclick") == 0) {
		return BtnTriggerDoubleClick;
	}	
	else if (strcmp(triggerString, "longpress") == 0) {
		return BtnTriggerLongPress;
	}
	else if (strcmp(triggerString, "dec") == 0) {
		return BtnTriggerDec;
	}
	else if (strcmp(triggerString, "inc") == 0) {
		return BtnTriggerInc;
	}

	LOGERR("Invalid action: %s", triggerString);

	return BtnTriggerNone;
}


CUIButtons::CUIButtons (CConfig *pConfig)
:	m_pConfig(pConfig),
	m_eventHandler (0),
	m_lastTick (0)
{
}

CUIButtons::~CUIButtons (void)
{
}

boolean CUIButtons::Initialize (void)
{
	assert (m_pConfig);

	// Read the button configuration
	m_doubleClickTimeout = m_pConfig->GetDoubleClickTimeout ();
	m_longPressTimeout = m_pConfig->GetLongPressTimeout ();
	m_prevPin = m_pConfig->GetButtonPinPrev ();
	m_prevAction = CUIButton::triggerTypeFromString( m_pConfig->GetButtonActionPrev ());
	m_nextPin = m_pConfig->GetButtonPinNext ();
	m_nextAction = CUIButton::triggerTypeFromString( m_pConfig->GetButtonActionNext ());
	m_backPin = m_pConfig->GetButtonPinBack ();
	m_backAction = CUIButton::triggerTypeFromString( m_pConfig->GetButtonActionBack ());
	m_selectPin = m_pConfig->GetButtonPinSelect ();
	m_selectAction = CUIButton::triggerTypeFromString( m_pConfig->GetButtonActionSelect ());
	m_homePin = m_pConfig->GetButtonPinHome ();
	m_homeAction = CUIButton::triggerTypeFromString( m_pConfig->GetButtonActionHome ());
	m_pgmUpPin = m_pConfig->GetButtonPinPgmUp ();
	m_pgmUpAction = CUIButton::triggerTypeFromString( m_pConfig->GetButtonActionPgmUp ());
	m_pgmDownPin = m_pConfig->GetButtonPinPgmDown ();
	m_pgmDownAction = CUIButton::triggerTypeFromString( m_pConfig->GetButtonActionPgmDown ());
	m_BankUpPin = m_pConfig->GetButtonPinBankUp ();
	m_BankUpAction = CUIButton::triggerTypeFromString( m_pConfig->GetButtonActionBankUp ());
	m_BankDownPin = m_pConfig->GetButtonPinBankDown ();
	m_BankDownAction = CUIButton::triggerTypeFromString( m_pConfig->GetButtonActionBankDown ());
	m_TGUpPin = m_pConfig->GetButtonPinTGUp ();
	m_TGUpAction = CUIButton::triggerTypeFromString( m_pConfig->GetButtonActionTGUp ());
	m_TGDownPin = m_pConfig->GetButtonPinTGDown ();
	m_TGDownAction = CUIButton::triggerTypeFromString( m_pConfig->GetButtonActionTGDown ());
	m_notesMidi = ccToMidiPin( m_pConfig->GetMIDIButtonNotes ());
	m_prevMidi = ccToMidiPin( m_pConfig->GetMIDIButtonPrev ());
	m_prevMidiAction = CUIButton::triggerTypeFromString( m_pConfig->GetMIDIButtonActionPrev ());
	m_nextMidi = ccToMidiPin( m_pConfig->GetMIDIButtonNext ());
	m_nextMidiAction = CUIButton::triggerTypeFromString( m_pConfig->GetMIDIButtonActionNext ());
	m_backMidi = ccToMidiPin( m_pConfig->GetMIDIButtonBack ());
	m_backMidiAction = CUIButton::triggerTypeFromString( m_pConfig->GetMIDIButtonActionBack ());
	m_selectMidi = ccToMidiPin( m_pConfig->GetMIDIButtonSelect ());
	m_selectMidiAction = CUIButton::triggerTypeFromString( m_pConfig->GetMIDIButtonActionSelect ());
	m_homeMidi = ccToMidiPin( m_pConfig->GetMIDIButtonHome ());
	m_homeMidiAction = CUIButton::triggerTypeFromString( m_pConfig->GetMIDIButtonActionHome ());
	m_pgmUpMidi = ccToMidiPin( m_pConfig->GetMIDIButtonPgmUp ());
	m_pgmUpMidiAction = CUIButton::triggerTypeFromString( m_pConfig->GetMIDIButtonActionPgmUp ());
	m_pgmDownMidi = ccToMidiPin( m_pConfig->GetMIDIButtonPgmDown ());
	m_pgmDownMidiAction = CUIButton::triggerTypeFromString( m_pConfig->GetMIDIButtonActionPgmDown ());
	m_BankUpMidi = ccToMidiPin( m_pConfig->GetMIDIButtonBankUp ());
	m_BankUpMidiAction = CUIButton::triggerTypeFromString( m_pConfig->GetMIDIButtonActionBankUp ());
	m_BankDownMidi = ccToMidiPin( m_pConfig->GetMIDIButtonBankDown ());
	m_BankDownMidiAction = CUIButton::triggerTypeFromString( m_pConfig->GetMIDIButtonActionBankDown ());
	m_TGUpMidi = ccToMidiPin( m_pConfig->GetMIDIButtonTGUp ());
	m_TGUpMidiAction = CUIButton::triggerTypeFromString( m_pConfig->GetMIDIButtonActionTGUp ());
	m_TGDownMidi = ccToMidiPin( m_pConfig->GetMIDIButtonTGDown ());
	m_TGDownMidiAction = CUIButton::triggerTypeFromString( m_pConfig->GetMIDIButtonActionTGDown ());

	// First sanity check and convert the timeouts:
	// Internally values are in tenths of a millisecond, but config values
	// are in milliseconds
	unsigned doubleClickTimeout = m_doubleClickTimeout * 10;
	unsigned longPressTimeout = m_longPressTimeout * 10;

	if (longPressTimeout < doubleClickTimeout) {
		// This is invalid - long press must be longest timeout
		LOGERR("LongPressTimeout (%u) should not be shorter than DoubleClickTimeout (%u)",
				m_longPressTimeout,
				m_doubleClickTimeout);

		// Just make long press as long as double click
		longPressTimeout = doubleClickTimeout;
	}

	// Each normal button can be assigned up to 3 actions: click, doubleclick and
	// longpress. We may not initialise all of the buttons.
	// MIDI buttons can be assigned to click, doubleclick, longpress, dec ,inc
	unsigned pins[MAX_BUTTONS] = {
		m_prevPin, m_nextPin, m_backPin, m_selectPin, m_homePin, m_pgmUpPin,  m_pgmDownPin,  m_BankUpPin,  m_BankDownPin, m_TGUpPin,  m_TGDownPin, 
		m_prevMidi, m_nextMidi, m_backMidi, m_selectMidi, m_homeMidi, m_pgmUpMidi, m_pgmDownMidi, m_BankUpMidi, m_BankDownMidi, m_TGUpMidi, m_TGDownMidi
	};
	CUIButton::BtnTrigger triggers[MAX_BUTTONS] = {
		// Normal buttons
		m_prevAction, m_nextAction, m_backAction, m_selectAction, m_homeAction,
		m_pgmUpAction, m_pgmDownAction, m_BankUpAction, m_BankDownAction, m_TGUpAction, m_TGDownAction, 
		// MIDI buttons
		m_prevMidiAction, m_nextMidiAction, m_backMidiAction, m_selectMidiAction, m_homeMidiAction,
		m_pgmUpMidiAction, m_pgmDownMidiAction, m_BankUpMidiAction, m_BankDownMidiAction, m_TGUpMidiAction, m_TGDownMidiAction,
	};
	CUIButton::BtnEvent events[MAX_BUTTONS] = {
		// Normal buttons
		CUIButton::BtnEventPrev,
		CUIButton::BtnEventNext,
		CUIButton::BtnEventBack,
		CUIButton::BtnEventSelect,
		CUIButton::BtnEventHome,
		CUIButton::BtnEventPgmUp,
		CUIButton::BtnEventPgmDown,
		CUIButton::BtnEventBankUp,
		CUIButton::BtnEventBankDown,
		CUIButton::BtnEventTGUp,
		CUIButton::BtnEventTGDown,
		// MIDI buttons
		CUIButton::BtnEventPrev,
		CUIButton::BtnEventNext,
		CUIButton::BtnEventBack,
		CUIButton::BtnEventSelect,
		CUIButton::BtnEventHome,
		CUIButton::BtnEventPgmUp,
		CUIButton::BtnEventPgmDown,
		CUIButton::BtnEventBankUp,
		CUIButton::BtnEventBankDown,
		CUIButton::BtnEventTGUp,
		CUIButton::BtnEventTGDown
	};

	// Setup normal GPIO buttons first
	for (unsigned i=0; i<MAX_GPIO_BUTTONS; i++) {
		// if this pin is 0 it means it's disabled - so continue
		if (pins[i] == 0) {
			continue;
		}

		// Search through buttons and see if there is already a button with this
		// pin number.  If we find a 0 then the button is not initialised and is
		// ready for this pin
		for (unsigned j=0; j<MAX_BUTTONS; j++) {
			if (m_buttons[j].getPinNumber() == pins[i]) {
				// This pin is already assigned
				LOGNOTE("Note: GPIO pin %d is already assigned", pins[i]);
				break;
			}
			else if (m_buttons[j].getPinNumber() == 0) {
				// This is un-initialised so can be assigned
				m_buttons[j].Initialize(pins[i], doubleClickTimeout, longPressTimeout);
				break;
			}
		}
	}
	
	// Now setup the MIDI buttons.
	// Note: the configuration is simpler as the only trigger supported is a single, short press
	for (unsigned i=MAX_GPIO_BUTTONS; i<MAX_BUTTONS; i++) {
		// if this pin is 0 it means it's disabled - so continue
		if (pins[i] == 0) {
			continue;
		}

		// Carry on in the list from where GPIO buttons left off
		for (unsigned j=0; j<MAX_BUTTONS; j++) {
			if (m_buttons[j].getPinNumber() == 0) {
				// This is un-initialised so can be assigned
				// doubleClickTimeout and longPressTimeout are ignored for MIDI buttons at present
				m_buttons[j].Initialize(pins[i], doubleClickTimeout, longPressTimeout);
				break;
			}
		}
	}

	// All of the buttons are now initialised, they just need to have their
	// events assigned to them
	
	for (unsigned i=0; i<MAX_BUTTONS; i++) {
		bindButton(pins[i], triggers[i], events[i]);
	}

	return TRUE;
}

void CUIButtons::bindButton(unsigned pinNumber, CUIButton::BtnTrigger trigger, CUIButton::BtnEvent event)
{
	// First find the button
	bool found = false;
	for (unsigned i=0; i<MAX_BUTTONS; i++) {
		if (m_buttons[i].getPinNumber() == pinNumber) {
			// This is the one!
			found = true;
			
			if (trigger == CUIButton::BtnTriggerClick) {
				m_buttons[i].setClickEvent(event);
			}
			else if (trigger == CUIButton::BtnTriggerDoubleClick) {
				m_buttons[i].setDoubleClickEvent(event);
			}
			else if (trigger == CUIButton::BtnTriggerLongPress) {
				m_buttons[i].setLongPressEvent(event);
			}
			else if (trigger == CUIButton::BtnTriggerDec) {
				m_buttons[i].setDecEvent(event);
			}
			else if (trigger == CUIButton::BtnTriggerInc) {
				m_buttons[i].setIncEvent(event);
			}
			else {
				assert (trigger == CUIButton::BtnTriggerNone);
			}

			break;
		}
	}

	assert(found);
}

void CUIButtons::RegisterEventHandler (BtnEventHandler *handler, void *param)
{
	assert (!m_eventHandler);
	m_eventHandler = handler;
	assert (m_eventHandler);
	m_eventParam = param;
}

void CUIButtons::Update (void)
{
	assert (m_eventHandler);

	// Don't update unless enough time has elapsed.
	// Get the current time in microseconds
	unsigned currentTick = CTimer::GetClockTicks();
	if (currentTick - m_lastTick < BUTTONS_UPDATE_NUM_TICKS) {
		// Not enough time has passed, just return
		return;
	}

	m_lastTick = currentTick;

	for (unsigned i=0; i<MAX_BUTTONS; i++) {
		CUIButton::BtnEvent event = m_buttons[i].Read();
		if (event != CUIButton::BtnEventNone) {
//			LOGDBG("Event: %u", event);
			(*m_eventHandler) (event, m_eventParam);
		}
	}
}

void CUIButtons::ResetButton (unsigned pinNumber)
{
	for (unsigned i=0; i<MAX_BUTTONS; i++) {
		if (m_buttons[i].getPinNumber() == pinNumber) {
			m_buttons[i].reset();
		}
	}
}

void CUIButtons::BtnMIDICmdHandler (unsigned nMidiCmd, unsigned nMidiData1, unsigned nMidiData2)
{
	if (m_notesMidi > 0) {
//		LOGDBG("BtnMIDICmdHandler (notes): %x %x %x)", nMidiCmd, nMidiData1, nMidiData2);
		// Using MIDI Note messages for MIDI buttons
		unsigned midiPin = ccToMidiPin(nMidiData1);
		for (unsigned i=0; i<MAX_BUTTONS; i++) {
			if (m_buttons[i].getPinNumber() == midiPin) {
				if (nMidiCmd == 0x80) {
					// NoteOff = Button OFF
					m_buttons[i].Write (0);
				} else if ((nMidiCmd == 0x90) && (nMidiData2 == 0)) {
					// NoteOn with Vel == 0 = Button OFF
					m_buttons[i].Write (0);
				} else if (nMidiCmd == 0x90) {
					// NoteOn = Button ON
					m_buttons[i].Write (127);
				} else {
					// Ignore other MIDI commands
				}
			}
		}
	} else {
//		LOGDBG("BtnMIDICmdHandler (CC): %x %x %x)", nMidiCmd, nMidiData1, nMidiData2);
		// Using MIDI CC messages for MIDI buttons
		if (nMidiCmd == 0xB0) {  // Control Message
			unsigned midiPin = ccToMidiPin(nMidiData1);
			for (unsigned i=0; i<MAX_BUTTONS; i++) {
				if (m_buttons[i].getPinNumber() == midiPin) {
					m_buttons[i].Write (nMidiData2);
				}
			}
		}
	}	
}
