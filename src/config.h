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

#include <circle/net/ipaddress.h>
#include <fatfs/ff.h>
#include <Properties/propertiesfatfsfile.h>
#include <circle/sysconfig.h>
#include <string>

#define SPI_INACTIVE	255
#define SPI_DEF_CLOCK	15000	// kHz
#define SPI_DEF_MODE	0		// Default mode (0,1,2,3)

class CConfig		// Configuration for MiniDexed
{
public:
// Set maximum, minimum and default numbers of tone generators, depending on Pi version.
// Actual number in can be changed via config settings for some Pis.
#ifndef ARM_ALLOW_MULTI_CORE
	// Pi V1 or Zero (single core)
	static const unsigned MinToneGenerators = 1;
	static const unsigned AllToneGenerators = 1;
	static const unsigned DefToneGenerators = AllToneGenerators;
#else
#if (RASPPI==4 || RASPPI==5)
	// Pi 4 and 5 quad core
	// These are max values, default is to support 8 in total with optional 16 TGs
	static const unsigned TGsCore1 = 2;		// process 2 TGs on core 1
	static const unsigned TGsCore23 = 3;		// process 3 TGs on core 2 and 3 each
	static const unsigned TGsCore1Opt = 2;		// process optional additional 2 TGs on core 1
	static const unsigned TGsCore23Opt = 3;		// process optional additional 3 TGs on core 2 and 3 each
	static const unsigned MinToneGenerators = TGsCore1 + 2*TGsCore23;
	static const unsigned AllToneGenerators = TGsCore1 + TGsCore1Opt + 2*TGsCore23 + 2*TGsCore23Opt;
	static const unsigned DefToneGenerators = MinToneGenerators;
#else
	// Pi 2 or 3 quad core
	static const unsigned TGsCore1 = 2;		// process 2 TGs on core 1
	static const unsigned TGsCore23 = 3;		// process 3 TGs on core 2 and 3 each
	static const unsigned TGsCore1Opt = 0;
	static const unsigned TGsCore23Opt = 0;
	static const unsigned MinToneGenerators = TGsCore1 + 2*TGsCore23;
	static const unsigned AllToneGenerators = MinToneGenerators;
	static const unsigned DefToneGenerators = AllToneGenerators;
#endif
#endif
	
// Set maximum polyphony, depending on PI version.  This can be changed via config settings
#if RASPPI == 1
	static const unsigned MaxNotes = 8;
	static const unsigned DefaultNotes = 8;
#elif RASPPI == 4
	static const unsigned MaxNotes = 32;
	static const unsigned DefaultNotes = 24;
#elif RASPPI == 5
	static const unsigned MaxNotes = 32;
	static const unsigned DefaultNotes = 32;
#else
	static const unsigned MaxNotes = 16;
	static const unsigned DefaultNotes = 16;
#endif

	static const unsigned MaxChunkSize = 4096;

#if RASPPI <= 3
	static const unsigned MaxUSBMIDIDevices = 2;
#else
	static const unsigned MaxUSBMIDIDevices = 4;
#endif

	static const unsigned MinLCDColumns = 16;		// HD44780 LCD
	static const unsigned MinLCDRows = 2;

public:
	CConfig (FATFS *pFileSystem);
	~CConfig (void);

	void Load (void);
	
	// TGs and Polyphony
	unsigned GetToneGenerators (void) const;
	unsigned GetPolyphony (void) const;
	unsigned GetTGsCore1 (void) const;
	unsigned GetTGsCore23 (void) const;
	
	// USB Mode
	bool GetUSBGadget (void) const;
	unsigned GetUSBGadgetPin (void) const;
	bool GetUSBGadgetMode (void) const;	// true if in USB gadget mode depending on USBGadget and USBGadgetPin
	void SetUSBGadgetMode (bool USBGadgetMode);

	// Sound device
	const char *GetSoundDevice (void) const;
	unsigned GetSampleRate (void) const;
	unsigned GetChunkSize (void) const;
	unsigned GetDACI2CAddress (void) const;		// 0 for auto probing
	bool GetChannelsSwapped (void) const;
	unsigned GetEngineType (void) const;
	bool GetQuadDAC8Chan (void) const; // false if not specified

	// MIDI
	unsigned GetMIDIBaudRate (void) const;
	const char *GetMIDIThruIn (void) const;	// "" if not specified
	const char *GetMIDIThruOut (void) const;	// "" if not specified
	bool GetMIDIRXProgramChange (void) const;	// true if not specified
	bool GetIgnoreAllNotesOff (void) const;
	bool GetMIDIAutoVoiceDumpOnPC (void) const; // false if not specified
	bool GetHeaderlessSysExVoices (void) const; // false if not specified
	bool GetExpandPCAcrossBanks (void) const; // true if not specified
	unsigned GetMIDISystemCCVol (void) const;
	unsigned GetMIDISystemCCPan (void) const;
	unsigned GetMIDISystemCCDetune (void) const;
	unsigned GetMIDIGlobalExpression (void) const;

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
	bool     GetSSD1306LCDRotate (void) const;
	bool     GetSSD1306LCDMirror (void) const;

	// SPI support
	unsigned GetSPIBus (void) const;
	unsigned GetSPIMode (void) const;
	unsigned GetSPIClockKHz (void) const;

	// ST7789 LCD
	bool     GetST7789Enabled (void) const;
	unsigned GetST7789Data (void) const;
	unsigned GetST7789Select (void) const;
	unsigned GetST7789Reset (void) const;
	unsigned GetST7789Backlight (void) const;
	unsigned GetST7789Width (void) const;
	unsigned GetST7789Height (void) const;
	unsigned GetST7789Rotation (void) const;
	bool     GetST7789SmallFont (void) const;

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

	// GPIO Button Program and TG Selection
	// GPIO pin numbers are chip numbers, not header positions
	unsigned GetButtonPinPgmUp (void) const;
	unsigned GetButtonPinPgmDown (void) const;
	unsigned GetButtonPinBankUp (void) const;
	unsigned GetButtonPinBankDown (void) const;
	unsigned GetButtonPinTGUp (void) const;
	unsigned GetButtonPinTGDown (void) const;

	// Action type for buttons: "click", "doubleclick", "longpress", ""
	const char *GetButtonActionPgmUp (void) const;
	const char *GetButtonActionPgmDown (void) const;
	const char *GetButtonActionBankUp (void) const;
	const char *GetButtonActionBankDown (void) const;
	const char *GetButtonActionTGUp (void) const;
	const char *GetButtonActionTGDown (void) const;

	// MIDI Button Navigation
	unsigned GetMIDIButtonCh   (void) const;
	unsigned GetMIDIButtonNotes (void) const;
	unsigned GetMIDIButtonPrev (void) const;
	unsigned GetMIDIButtonNext (void) const;
	unsigned GetMIDIButtonBack (void) const;
	unsigned GetMIDIButtonSelect (void) const;
	unsigned GetMIDIButtonHome (void) const;

	// MIDI Button Program and TG Selection
	unsigned GetMIDIButtonPgmUp (void) const;
	unsigned GetMIDIButtonPgmDown (void) const;
	unsigned GetMIDIButtonBankUp (void) const;
	unsigned GetMIDIButtonBankDown (void) const;
	unsigned GetMIDIButtonTGUp (void) const;
	unsigned GetMIDIButtonTGDown (void) const;
	
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
	unsigned GetPerformanceSelectChannel (void) const;

	unsigned GetMasterVolume() const { return m_nMasterVolume; }

	// Network
	bool GetNetworkEnabled (void) const;
	bool GetNetworkDHCP (void) const;
	const char *GetNetworkType (void) const;
	const char *GetNetworkHostname (void) const;
	CIPAddress GetNetworkIPAddress (void) const;
	CIPAddress GetNetworkSubnetMask (void) const;
	CIPAddress GetNetworkDefaultGateway (void) const;
	CIPAddress GetNetworkDNSServer (void) const;
	bool GetSyslogEnabled (void) const;
	CIPAddress GetNetworkSyslogServerIPAddress (void) const;
	bool GetNetworkFTPEnabled (void) const;

private:
	CPropertiesFatFsFile m_Properties;
	
	unsigned m_nToneGenerators;
	unsigned m_nPolyphony;
	
	bool m_bUSBGadget;
	unsigned m_nUSBGadgetPin;
	bool m_bUSBGadgetMode;

	std::string m_SoundDevice;
	unsigned m_nSampleRate;
	unsigned m_nChunkSize;
	unsigned m_nDACI2CAddress;
	bool m_bChannelsSwapped;
	unsigned m_EngineType;
	bool m_bQuadDAC8Chan;

	unsigned m_nMIDIBaudRate;
	std::string m_MIDIThruIn;
	std::string m_MIDIThruOut;
	bool m_bMIDIRXProgramChange;
	bool m_bIgnoreAllNotesOff;
	bool m_bMIDIAutoVoiceDumpOnPC;
	bool m_bHeaderlessSysExVoices;
	bool m_bExpandPCAcrossBanks;
	unsigned m_nMIDISystemCCVol;
	unsigned m_nMIDISystemCCPan;
	unsigned m_nMIDISystemCCDetune;
	unsigned m_nMIDIGlobalExpression;

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
	bool     m_bSSD1306LCDRotate;
	bool     m_bSSD1306LCDMirror;

	unsigned m_nSPIBus;
	unsigned m_nSPIMode;
	unsigned m_nSPIClockKHz;

	bool     m_bST7789Enabled;
	unsigned m_nST7789Data;
	unsigned m_nST7789Select;
	unsigned m_nST7789Reset;
	unsigned m_nST7789Backlight;
	unsigned m_nST7789Width;
	unsigned m_nST7789Height;
	unsigned m_nST7789Rotation;
	unsigned m_bST7789SmallFont;

	unsigned m_nLCDColumns;
	unsigned m_nLCDRows;
	
	unsigned m_nButtonPinPrev;
	unsigned m_nButtonPinNext;
	unsigned m_nButtonPinBack;
	unsigned m_nButtonPinSelect;
	unsigned m_nButtonPinHome;
	unsigned m_nButtonPinShortcut;
	unsigned m_nButtonPinPgmUp;
	unsigned m_nButtonPinPgmDown;
	unsigned m_nButtonPinBankUp;
	unsigned m_nButtonPinBankDown;
	unsigned m_nButtonPinTGUp;
	unsigned m_nButtonPinTGDown;

	std::string m_ButtonActionPrev;
	std::string m_ButtonActionNext;
	std::string m_ButtonActionBack;
	std::string m_ButtonActionSelect;
	std::string m_ButtonActionHome;
	std::string m_ButtonActionPgmUp;
	std::string m_ButtonActionPgmDown;
	std::string m_ButtonActionBankUp;
	std::string m_ButtonActionBankDown;
	std::string m_ButtonActionTGUp;
	std::string m_ButtonActionTGDown;
	
	unsigned m_nDoubleClickTimeout;
	unsigned m_nLongPressTimeout;

	unsigned m_nMIDIButtonCh;
	unsigned m_nMIDIButtonNotes;
	unsigned m_nMIDIButtonPrev;
	unsigned m_nMIDIButtonNext;
	unsigned m_nMIDIButtonBack;
	unsigned m_nMIDIButtonSelect;
	unsigned m_nMIDIButtonHome;
	unsigned m_nMIDIButtonPgmUp;
	unsigned m_nMIDIButtonPgmDown;
	unsigned m_nMIDIButtonBankUp;
	unsigned m_nMIDIButtonBankDown;
	unsigned m_nMIDIButtonTGUp;
	unsigned m_nMIDIButtonTGDown;

	bool m_bEncoderEnabled;
	unsigned m_nEncoderPinClock;
	unsigned m_nEncoderPinData;

	bool m_bMIDIDumpEnabled;
	bool m_bProfileEnabled;
	bool m_bPerformanceSelectToLoad;
	unsigned m_bPerformanceSelectChannel;

	unsigned m_nMasterVolume; // Master volume 0-127

	// Network
	bool m_bNetworkEnabled;
	bool m_bNetworkDHCP;
	std::string m_NetworkType;
	std::string m_NetworkHostname;
	CIPAddress m_INetworkIPAddress;
	CIPAddress m_INetworkSubnetMask;
	CIPAddress m_INetworkDefaultGateway;
	CIPAddress m_INetworkDNSServer;
	bool m_bSyslogEnabled;
	CIPAddress m_INetworkSyslogServerIPAddress;
	bool m_bNetworkFTPEnabled;
};

#endif
