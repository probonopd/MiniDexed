//
// config.cpp
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
#include "config.h"

CConfig::CConfig (FATFS *pFileSystem)
:	m_Properties ("minidexed.ini", pFileSystem)
{
}

CConfig::~CConfig (void)
{
}

void CConfig::Load (void)
{
	m_Properties.Load ();

	m_SoundDevice = m_Properties.GetString ("SoundDevice", "pwm");

	m_nSampleRate = m_Properties.GetNumber ("SampleRate", 48000);
#ifdef ARM_ALLOW_MULTI_CORE
	m_nChunkSize = m_Properties.GetNumber ("ChunkSize", m_SoundDevice == "hdmi" ? 384*6 : 256);
#else
	m_nChunkSize = m_Properties.GetNumber ("ChunkSize", m_SoundDevice == "hdmi" ? 384*6 : 1024);
#endif
	m_nDACI2CAddress = m_Properties.GetNumber ("DACI2CAddress", 0);
	m_bChannelsSwapped = m_Properties.GetNumber ("ChannelsSwapped", 0) != 0;

	m_nMIDIBaudRate = m_Properties.GetNumber ("MIDIBaudRate", 31250);

	const char *pMIDIThru = m_Properties.GetString ("MIDIThru");
	if (pMIDIThru)
	{
		std::string Arg (pMIDIThru);

		size_t nPos = Arg.find (',');
		if (nPos != std::string::npos)
		{
			m_MIDIThruIn = Arg.substr (0, nPos);
			m_MIDIThruOut = Arg.substr (nPos+1);

			if (   m_MIDIThruIn.empty ()
			    || m_MIDIThruOut.empty ())
			{
				m_MIDIThruIn.clear ();
				m_MIDIThruOut.clear ();
			}
		}
	}
	
	m_bMIDIRXProgramChange = m_Properties.GetNumber ("MIDIRXProgramChange", 1) != 0;


	m_bLCDEnabled = m_Properties.GetNumber ("LCDEnabled", 0) != 0;
	m_nLCDPinEnable = m_Properties.GetNumber ("LCDPinEnable", 4);
	m_nLCDPinRegisterSelect = m_Properties.GetNumber ("LCDPinRegisterSelect", 27);
	m_nLCDPinReadWrite = m_Properties.GetNumber ("LCDPinReadWrite", 0);
	m_nLCDPinData4 = m_Properties.GetNumber ("LCDPinData4", 22);
	m_nLCDPinData5 = m_Properties.GetNumber ("LCDPinData5", 23);
	m_nLCDPinData6 = m_Properties.GetNumber ("LCDPinData6", 24);
	m_nLCDPinData7 = m_Properties.GetNumber ("LCDPinData7", 25);
	m_nLCDI2CAddress = m_Properties.GetNumber ("LCDI2CAddress", 0);

	m_nSSD1306LCDI2CAddress = m_Properties.GetNumber ("SSD1306LCDI2CAddress", 0);
	m_nSSD1306LCDWidth = m_Properties.GetNumber ("SSD1306LCDWidth", 128);
	m_nSSD1306LCDHeight = m_Properties.GetNumber ("SSD1306LCDHeight", 32);

	m_nLCDColumns = m_Properties.GetNumber ("LCDColumns", 16);
	m_nLCDRows = m_Properties.GetNumber ("LCDRows", 2);

	m_nButtonPinPrev = m_Properties.GetNumber ("ButtonPinPrev", 0);
	m_nButtonPinNext = m_Properties.GetNumber ("ButtonPinNext", 0);
	m_nButtonPinBack = m_Properties.GetNumber ("ButtonPinBack", 11);
	m_nButtonPinSelect = m_Properties.GetNumber ("ButtonPinSelect", 11);
	m_nButtonPinHome = m_Properties.GetNumber ("ButtonPinHome", 11);
	m_nButtonPinShortcut = m_Properties.GetNumber ("ButtonPinShortcut", 11);

	m_ButtonActionPrev = m_Properties.GetString ("ButtonActionPrev", "");
	m_ButtonActionNext = m_Properties.GetString ("ButtonActionNext", "");
	m_ButtonActionBack = m_Properties.GetString ("ButtonActionBack", "doubleclick");
	m_ButtonActionSelect = m_Properties.GetString ("ButtonActionSelect", "click");
	m_ButtonActionHome = m_Properties.GetString ("ButtonActionHome", "longpress");

	m_nDoubleClickTimeout = m_Properties.GetNumber ("DoubleClickTimeout", 400);
	m_nLongPressTimeout = m_Properties.GetNumber ("LongPressTimeout", 600);

	m_bEncoderEnabled = m_Properties.GetNumber ("EncoderEnabled", 0) != 0;
	m_nEncoderPinClock = m_Properties.GetNumber ("EncoderPinClock", 10);
	m_nEncoderPinData = m_Properties.GetNumber ("EncoderPinData", 9);

	m_bMIDIDumpEnabled  = m_Properties.GetNumber ("MIDIDumpEnabled", 0) != 0;
	m_bProfileEnabled = m_Properties.GetNumber ("ProfileEnabled", 0) != 0;
	m_bPerformanceSelectToLoad = m_Properties.GetNumber ("PerformanceSelectToLoad", 1) != 0;
}

const char *CConfig::GetSoundDevice (void) const
{
	return m_SoundDevice.c_str ();
}

unsigned CConfig::GetSampleRate (void) const
{
	return m_nSampleRate;
}

unsigned CConfig::GetChunkSize (void) const
{
	return m_nChunkSize;
}

unsigned CConfig::GetDACI2CAddress (void) const
{
	return m_nDACI2CAddress;
}

bool CConfig::GetChannelsSwapped (void) const
{
	return m_bChannelsSwapped;
}

unsigned CConfig::GetMIDIBaudRate (void) const
{
	return m_nMIDIBaudRate;
}

const char *CConfig::GetMIDIThruIn (void) const
{
	return m_MIDIThruIn.c_str ();
}

const char *CConfig::GetMIDIThruOut (void) const
{
	return m_MIDIThruOut.c_str ();
}

bool CConfig::GetMIDIRXProgramChange (void) const
{
	return m_bMIDIRXProgramChange;
}

bool CConfig::GetLCDEnabled (void) const
{
	return m_bLCDEnabled;
}

unsigned CConfig::GetLCDPinEnable (void) const
{
	return m_nLCDPinEnable;
}

unsigned CConfig::GetLCDPinRegisterSelect (void) const
{
	return m_nLCDPinRegisterSelect;
}

unsigned CConfig::GetLCDPinReadWrite (void) const
{
	return m_nLCDPinReadWrite;
}

unsigned CConfig::GetLCDPinData4 (void) const
{
	return m_nLCDPinData4;
}

unsigned CConfig::GetLCDPinData5 (void) const
{
	return m_nLCDPinData5;
}

unsigned CConfig::GetLCDPinData6 (void) const
{
	return m_nLCDPinData6;
}

unsigned CConfig::GetLCDPinData7 (void) const
{
	return m_nLCDPinData7;
}

unsigned CConfig::GetLCDI2CAddress (void) const
{
	return m_nLCDI2CAddress;
}

unsigned CConfig::GetSSD1306LCDI2CAddress (void) const
{
	return m_nSSD1306LCDI2CAddress;
}

unsigned CConfig::GetSSD1306LCDWidth (void) const
{
	return m_nSSD1306LCDWidth;
}

unsigned CConfig::GetSSD1306LCDHeight (void) const
{
	return m_nSSD1306LCDHeight;
}

unsigned CConfig::GetLCDColumns (void) const
{
	return m_nLCDColumns;
}

unsigned CConfig::GetLCDRows (void) const
{
	return m_nLCDRows;
}

unsigned CConfig::GetButtonPinPrev (void) const
{
	return m_nButtonPinPrev;
}

unsigned CConfig::GetButtonPinNext (void) const
{
	return m_nButtonPinNext;
}

unsigned CConfig::GetButtonPinBack (void) const
{
	return m_nButtonPinBack;
}

unsigned CConfig::GetButtonPinSelect (void) const
{
	return m_nButtonPinSelect;
}

unsigned CConfig::GetButtonPinHome (void) const
{
	return m_nButtonPinHome;
}

unsigned CConfig::GetButtonPinShortcut (void) const
{
	return m_nButtonPinShortcut;
}

const char *CConfig::GetButtonActionPrev (void) const
{
	return m_ButtonActionPrev.c_str();
}

const char *CConfig::GetButtonActionNext (void) const
{
	return m_ButtonActionNext.c_str();
}

const char *CConfig::GetButtonActionBack (void) const
{
	return m_ButtonActionBack.c_str();
}

const char *CConfig::GetButtonActionSelect (void) const
{
	return m_ButtonActionSelect.c_str();
}

const char *CConfig::GetButtonActionHome (void) const
{
	return m_ButtonActionHome.c_str();
}

unsigned CConfig::GetDoubleClickTimeout (void) const
{
	return m_nDoubleClickTimeout;
}

unsigned CConfig::GetLongPressTimeout (void) const
{
	return m_nLongPressTimeout;
}

bool CConfig::GetEncoderEnabled (void) const
{
	return m_bEncoderEnabled;
}

unsigned CConfig::GetEncoderPinClock (void) const
{
	return m_nEncoderPinClock;
}

unsigned CConfig::GetEncoderPinData (void) const
{
	return m_nEncoderPinData;
}

bool CConfig::GetMIDIDumpEnabled (void) const
{
	return m_bMIDIDumpEnabled;
}

bool CConfig::GetProfileEnabled (void) const
{
	return m_bProfileEnabled;
}

bool CConfig::GetPerformanceSelectToLoad (void) const
{
	return m_bPerformanceSelectToLoad;
}
