//
// mididevice.h
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
#ifndef _mididevice_h
#define _mididevice_h

#include "config.h"
#include <string>
#include <unordered_map>
#include <circle/types.h>
#include <circle/spinlock.h>
#include "userinterface.h"

#define MAX_DX7_SYSEX_LENGTH 4104
#define MAX_MIDI_MESSAGE MAX_DX7_SYSEX_LENGTH

class CMiniDexed;

class CMIDIDevice
{
public:
	enum TChannel
	{
		Channels = 16,
		OmniMode = Channels,
		Disabled,
		ChannelUnknown
	};

public:
	CMIDIDevice (CMiniDexed *pSynthesizer, CConfig *pConfig, CUserInterface *pUI);
	virtual ~CMIDIDevice (void);

	void SetChannel (u8 ucChannel, unsigned nTG);
	u8 GetChannel (unsigned nTG) const;

	virtual void Send (const u8 *pMessage, size_t nLength, unsigned nCable = 0) {}
	// Change signature to specify device name
	void SendSystemExclusiveVoice(uint8_t nVoice, const std::string& deviceName, unsigned nCable, uint8_t nTG);
	const std::string& GetDeviceName() const { return m_DeviceName; }

protected:
	void MIDIMessageHandler (const u8 *pMessage, size_t nLength, unsigned nCable = 0);
	void AddDevice (const char *pDeviceName);
	void HandleSystemExclusive(const uint8_t* pMessage, const size_t nLength, const unsigned nCable, const uint8_t nTG);

private:
	bool HandleMIDISystemCC(const u8 ucCC, const u8 ucCCval);

private:
	CMiniDexed *m_pSynthesizer;
	CConfig *m_pConfig;
	CUserInterface *m_pUI;

	u8 m_ChannelMap[CConfig::AllToneGenerators];
	u8 m_PreviousChannelMap[CConfig::AllToneGenerators]; // Store previous channels for OMNI OFF restore
	
	unsigned m_nMIDISystemCCVol;
	unsigned m_nMIDISystemCCPan;
	unsigned m_nMIDISystemCCDetune;
	u32	 m_MIDISystemCCBitmap[4]; // to allow for 128 bit entries
	unsigned m_nMIDIGlobalExpression;

	std::string m_DeviceName;

	typedef std::unordered_map<std::string, CMIDIDevice *> TDeviceMap;
	static TDeviceMap s_DeviceMap;

	CSpinLock m_MIDISpinLock;
};

#endif
