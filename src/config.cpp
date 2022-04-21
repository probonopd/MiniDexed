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

	m_bLCDEnabled = m_Properties.GetNumber ("LCDEnabled", 0) != 0;
	m_nLCDPinEnable = m_Properties.GetNumber ("LCDPinEnable", 17);
	m_nLCDPinRegisterSelect = m_Properties.GetNumber ("LCDPinRegisterSelect", 27);
	m_nLCDPinReadWrite = m_Properties.GetNumber ("LCDPinReadWrite", 16);
	m_nLCDPinData4 = m_Properties.GetNumber ("LCDPinData4", 22);
	m_nLCDPinData5 = m_Properties.GetNumber ("LCDPinData5", 23);
	m_nLCDPinData6 = m_Properties.GetNumber ("LCDPinData6", 24);
	m_nLCDPinData7 = m_Properties.GetNumber ("LCDPinData7", 25);

	m_bEncoderEnabled = m_Properties.GetNumber ("EncoderEnabled", 0) != 0;
	m_nEncoderPinClock = m_Properties.GetNumber ("EncoderPinClock", 5);
	m_nEncoderPinData = m_Properties.GetNumber ("EncoderPinData", 6);
	m_nEncoderPinSwitch = m_Properties.GetNumber ("EncoderPinSwitch", 26);

	m_bMIDIDumpEnabled  = m_Properties.GetNumber ("MIDIDumpEnabled", 0) != 0;
	m_bProfileEnabled = m_Properties.GetNumber ("ProfileEnabled", 0) != 0;
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

unsigned CConfig::GetEncoderPinSwitch (void) const
{
	return m_nEncoderPinSwitch;
}

bool CConfig::GetMIDIDumpEnabled (void) const
{
	return m_bMIDIDumpEnabled;
}

bool CConfig::GetProfileEnabled (void) const
{
	return m_bProfileEnabled;
}
