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

#define MIDI_NOTE_OFF		0b1000
#define MIDI_NOTE_ON		0b1001
#define MIDI_AFTERTOUCH		0b1010			// TODO
#define MIDI_CHANNEL_AFTERTOUCH 0b1101   // right now Synth_Dexed just manage Channel Aftertouch not Polyphonic AT -> 0b1010
#define MIDI_CONTROL_CHANGE	0b1011
	#define MIDI_CC_BANK_SELECT_MSB		0
	#define MIDI_CC_MODULATION			1
	#define MIDI_CC_BREATH_CONTROLLER	2 
	#define MIDI_CC_FOOT_PEDAL 		4
	#define MIDI_CC_PORTAMENTO_TIME		5
	#define MIDI_CC_VOLUME				7
	#define MIDI_CC_PAN_POSITION		10
	#define MIDI_CC_BANK_SELECT_LSB		32
	#define MIDI_CC_BANK_SUSTAIN		64
	#define MIDI_CC_PORTAMENTO			65
	#define MIDI_CC_SOSTENUTO			66
	#define MIDI_CC_RESONANCE			71
	#define MIDI_CC_FREQUENCY_CUTOFF	74
	#define MIDI_CC_REVERB_LEVEL		91
	#define MIDI_CC_DETUNE_LEVEL		94
	#define MIDI_CC_ALL_SOUND_OFF		120
	#define MIDI_CC_ALL_NOTES_OFF		123
#define MIDI_PROGRAM_CHANGE	0b1100
#define MIDI_PITCH_BEND		0b1110

class CMiniDexed;

struct TMIDIRoute
{
	enum TRouteOP
	{
		P2 = 0x82,
	};

	u8 ucSCable;
	u8 ucSCh;
	u8 ucSType;
	u8 ucSP1;
	u8 ucSP2;
	u8 ucDCh;
	u8 ucDType;
	u8 ucDP1;
	u8 ucDP2;
	bool bSkip;
	bool bToggle;
	bool bHandled;
	bool bSkipHandled;
};

void GetRoutedMIDI (TMIDIRoute *m_pRouteMap, u8 *pCable, u8 *pChannel, u8 *pType, u8 *pP1, u8 *pP2, bool *bSkip);

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

	void SetRouteMap (TMIDIRoute *pRouteMap);

	virtual void Send (const u8 *pMessage, size_t nLength, unsigned nCable = 0) {}
	virtual void SendSystemExclusiveVoice(uint8_t nVoice, const unsigned nCable, uint8_t nTG);

protected:
	void MIDIMessageHandler (const u8 *pMessage, size_t nLength, unsigned nCable = 0);
	void AddDevice (const char *pDeviceName);
	void HandleSystemExclusive(const uint8_t* pMessage, const size_t nLength, const unsigned nCable, const uint8_t nTG);

	virtual void MIDIListener (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2);

private:
	bool HandleMIDISystemCC(const u8 ucCC, const u8 ucCCval);

private:
	CMiniDexed *m_pSynthesizer;
	CConfig *m_pConfig;
	CUserInterface *m_pUI;

	u8 m_ChannelMap[CConfig::AllToneGenerators];
	
	unsigned m_nMIDISystemCCVol;
	unsigned m_nMIDISystemCCPan;
	unsigned m_nMIDISystemCCDetune;
	u32	 m_MIDISystemCCBitmap[4]; // to allow for 128 bit entries

	std::string m_DeviceName;

	TMIDIRoute *m_pRouteMap;

	typedef std::unordered_map<std::string, CMIDIDevice *> TDeviceMap;
	static TDeviceMap s_DeviceMap;

	CSpinLock m_MIDISpinLock;
};

#endif
