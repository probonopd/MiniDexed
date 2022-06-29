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
#include <circle/timer.h>
#include <string.h>

LOGMODULE ("uibuttons");

CUIButton::CUIButton (void)
:	m_pinNumber (0),
	m_pin (0),
	m_lastValue (1),
	m_timer (0),
	m_debounceTimer (0),
	m_numClicks (0),
	m_clickEvent(BtnEventNone),
	m_doubleClickEvent(BtnEventNone),
	m_longPressEvent(BtnEventNone),
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
		m_pin = new CGPIOPin (m_pinNumber, GPIOModeInputPullUp);
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

unsigned CUIButton::getPinNumber(void)
{
	return m_pinNumber;
}
	
CUIButton::BtnTrigger CUIButton::ReadTrigger (void)
{
	if (!m_pin)
	{
		// Always return "not pressed" if not configured
		return BtnTriggerNone;
	}

	unsigned value = m_pin->Read();

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

	LOGERR("Invalid action: %s", triggerString);

	return BtnTriggerNone;
}


CUIButtons::CUIButtons (
			unsigned prevPin, const char *prevAction,
			unsigned nextPin, const char *nextAction,
			unsigned backPin, const char *backAction,
			unsigned selectPin, const char *selectAction,
			unsigned homePin, const char *homeAction,
			unsigned doubleClickTimeout, unsigned longPressTimeout
)
:	m_doubleClickTimeout(doubleClickTimeout),
	m_longPressTimeout(longPressTimeout),
	m_prevPin(prevPin),
	m_prevAction(CUIButton::triggerTypeFromString(prevAction)),
	m_nextPin(nextPin),
	m_nextAction(CUIButton::triggerTypeFromString(nextAction)),
	m_backPin(backPin),
	m_backAction(CUIButton::triggerTypeFromString(backAction)),
	m_selectPin(selectPin),
	m_selectAction(CUIButton::triggerTypeFromString(selectAction)),
	m_homePin(homePin),
	m_homeAction(CUIButton::triggerTypeFromString(homeAction)),
	m_eventHandler (0),
	m_lastTick (0)
{
}

CUIButtons::~CUIButtons (void)
{
}

boolean CUIButtons::Initialize (void)
{
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

	// Each button can be assigned up to 3 actions: click, doubleclick and
	// longpress. We may not initialise all of the buttons
	unsigned pins[MAX_BUTTONS] = {
		m_prevPin, m_nextPin, m_backPin, m_selectPin, m_homePin
	};
	CUIButton::BtnTrigger triggers[MAX_BUTTONS] = {
		m_prevAction, m_nextAction, m_backAction, m_selectAction, m_homeAction
	};
	CUIButton::BtnEvent events[MAX_BUTTONS] = {
		CUIButton::BtnEventPrev,
		CUIButton::BtnEventNext,
		CUIButton::BtnEventBack,
		CUIButton::BtnEventSelect,
		CUIButton::BtnEventHome
	};

	for (unsigned i=0; i<MAX_BUTTONS; i++) {
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
				break;
			}
			else if (m_buttons[j].getPinNumber() == 0) {
				// This is un-initialised so can be assigned
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
			LOGDBG("Event: %u", event);
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
