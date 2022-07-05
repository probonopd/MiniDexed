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

#define MIDI_CC_BANK_SELECT_MSB         0       // TODO
#define MIDI_CC_MODULATION                      1
#define MIDI_CC_VOLUME                          7
#define MIDI_CC_PAN_POSITION            10
#define MIDI_CC_BANK_SELECT_LSB         32
#define MIDI_CC_BANK_SUSTAIN            64
#define MIDI_CC_RESONANCE                       71
#define MIDI_CC_FREQUENCY_CUTOFF        74
#define MIDI_CC_REVERB_LEVEL            91
#define MIDI_CC_DETUNE_LEVEL            94
#define MIDI_CC_ALL_SOUND_OFF           120
#define MIDI_CC_ALL_NOTES_OFF           123

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
	CMIDIDevice (CMiniDexed *pSynthesizer, CConfig *pConfig);
	virtual ~CMIDIDevice (void);

	void SetChannel (u8 ucChannel, unsigned nTG);
	u8 GetChannel (unsigned nTG) const;

	virtual void Send (const u8 *pMessage, size_t nLength, unsigned nCable = 0) {}
	virtual void SendSystemExclusiveVoice(uint8_t nVoice, uint8_t nTG);
	virtual void SendSystemExclusiveConfig();
	virtual void SendProgramChange(uint8_t pgm, uint8_t nTG);
	virtual void SendBankChange(uint8_t bank, uint8_t nTG);
	virtual void SendBankName( uint8_t nTG);
	virtual void SendCtrlChange(uint8_t ctrl, uint8_t val, uint8_t nTG);
	virtual void SendCtrlChange14Bit(uint8_t ctrl, int16_t val, uint8_t nTG);
protected:
	void MIDIMessageHandler (const u8 *pMessage, size_t nLength, unsigned nCable = 0);
	void AddDevice (const char *pDeviceName);
	void HandleSystemExclusive(const uint8_t* pMessage, const size_t nLength, const unsigned nCable, const uint8_t nTG);
	int16_t checkSystemExclusive(const uint8_t* sysex, const uint16_t len);
private:
	CMiniDexed *m_pSynthesizer;
	CConfig *m_pConfig;

	u8 m_ChannelMap[CConfig::ToneGenerators];

	std::string m_DeviceName;

	typedef std::unordered_map<std::string, CMIDIDevice *> TDeviceMap;
	static TDeviceMap s_DeviceMap;

	CSpinLock m_MIDISpinLock;
};

#endif
