//
// config.h
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
#ifndef _config_h
#define _config_h

#include <fatfs/ff.h>
#include <Properties/propertiesfatfsfile.h>
#include <circle/sysconfig.h>
#include <string>

class CConfig		// Configuration for MiniDexed
{
public:
#ifndef ARM_ALLOW_MULTI_CORE
	static const unsigned ToneGenerators = 1;
#else
	static const unsigned TGsCore1 = 2;		// process 2 TGs on core 1
	static const unsigned TGsCore23 = 3;		// process 3 TGs on core 2 and 3 each
	static const unsigned ToneGenerators = TGsCore1 + 2*TGsCore23;
#endif

#if RASPPI == 1
	static const unsigned MaxNotes = 8;		// polyphony
#else
	static const unsigned MaxNotes = 16;
#endif

	static const unsigned MaxChunkSize = 4096;

#if RASPPI <= 3
	static const unsigned MaxUSBMIDIDevices = 2;
#else
	static const unsigned MaxUSBMIDIDevices = 4;
#endif

	// TODO - Leave this for uimenu.cpp for now, but it will need to be dynamic at some point...
	static const unsigned LCDColumns = 16;		// HD44780 LCD
	static const unsigned LCDRows = 2;

public:
	CConfig (FATFS *pFileSystem);
	~CConfig (void);

	void Load (void);

	// Sound device
	const char *GetSoundDevice (void) const;
	unsigned GetSampleRate (void) const;
	unsigned GetChunkSize (void) const;
	unsigned GetDACI2CAddress (void) const;		// 0 for auto probing
	bool GetChannelsSwapped (void) const;

	// MIDI
	unsigned GetMIDIBaudRate (void) const;
	const char *GetMIDIThruIn (void) const;	// "" if not specified
	const char *GetMIDIThruOut (void) const;	// "" if not specified
	bool GetMIDIRXProgramChange (void) const;	// true if not specified
	

	// HD44780 LCD
	// GPIO pin numbers are chip numbers, not header positions
	bool GetLCDEnabled (void) const;
	unsigned GetLCDPinEnable (void) const;
	unsigned GetLCDPinRegisterSelect (void) const;
	unsigned GetLCDPinReadWrite (void) const;	// set to 0 if not connected
	unsigned GetLCDPinData4 (void) const;
	unsigned GetLCDPinData5 (void) const;
	unsigned GetLCDPinData6 (void) const;
	unsigned GetLCDPinData7 (void) const;
	unsigned GetLCDI2CAddress (void) const;
	
	// SSD1306 LCD
	unsigned GetSSD1306LCDI2CAddress (void) const;
	unsigned GetSSD1306LCDWidth (void) const;
	unsigned GetSSD1306LCDHeight (void) const;

	unsigned GetLCDColumns (void) const;
	unsigned GetLCDRows (void) const;

	// GPIO Button Navigation
	// GPIO pin numbers are chip numbers, not header positions
	unsigned GetButtonPinPrev (void) const;
	unsigned GetButtonPinNext (void) const;
	unsigned GetButtonPinBack (void) const;
	unsigned GetButtonPinSelect (void) const;
	unsigned GetButtonPinHome (void) const;
	unsigned GetButtonPinShortcut (void) const;

	// Action type for buttons: "click", "doubleclick", "longpress", ""
	const char *GetButtonActionPrev (void) const;
	const char *GetButtonActionNext (void) const;
	const char *GetButtonActionBack (void) const;
	const char *GetButtonActionSelect (void) const;
	const char *GetButtonActionHome (void) const;
	
	// Timeouts for button events in milliseconds
	unsigned GetDoubleClickTimeout (void) const;
	unsigned GetLongPressTimeout (void) const;

	// KY-040 Rotary Encoder
	// GPIO pin numbers are chip numbers, not header positions
	bool GetEncoderEnabled (void) const;
	unsigned GetEncoderPinClock (void) const;
	unsigned GetEncoderPinData (void) const;

	// Debug
	bool GetMIDIDumpEnabled (void) const;
	bool GetProfileEnabled (void) const;
	
	// Load performance mode. 0 for load just rotating encoder, 1 load just when Select is pushed
	bool GetPerformanceSelectToLoad (void) const;

private:
	CPropertiesFatFsFile m_Properties;

	std::string m_SoundDevice;
	unsigned m_nSampleRate;
	unsigned m_nChunkSize;
	unsigned m_nDACI2CAddress;
	bool m_bChannelsSwapped;

	unsigned m_nMIDIBaudRate;
	std::string m_MIDIThruIn;
	std::string m_MIDIThruOut;
	bool m_bMIDIRXProgramChange;

	bool m_bLCDEnabled;
	unsigned m_nLCDPinEnable;
	unsigned m_nLCDPinRegisterSelect;
	unsigned m_nLCDPinReadWrite;
	unsigned m_nLCDPinData4;
	unsigned m_nLCDPinData5;
	unsigned m_nLCDPinData6;
	unsigned m_nLCDPinData7;
	unsigned m_nLCDI2CAddress;
	
	unsigned m_nSSD1306LCDI2CAddress;
	unsigned m_nSSD1306LCDWidth;
	unsigned m_nSSD1306LCDHeight;
	
	unsigned m_nLCDColumns;
	unsigned m_nLCDRows;
	
	unsigned m_nButtonPinPrev;
	unsigned m_nButtonPinNext;
	unsigned m_nButtonPinBack;
	unsigned m_nButtonPinSelect;
	unsigned m_nButtonPinHome;
	unsigned m_nButtonPinShortcut;

	std::string m_ButtonActionPrev;
	std::string m_ButtonActionNext;
	std::string m_ButtonActionBack;
	std::string m_ButtonActionSelect;
	std::string m_ButtonActionHome;
	
	unsigned m_nDoubleClickTimeout;
	unsigned m_nLongPressTimeout;

	bool m_bEncoderEnabled;
	unsigned m_nEncoderPinClock;
	unsigned m_nEncoderPinData;

	bool m_bMIDIDumpEnabled;
	bool m_bProfileEnabled;
	bool m_bPerformanceSelectToLoad;
};

#endif
