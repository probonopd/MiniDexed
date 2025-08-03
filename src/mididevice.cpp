//
// mididevice.cpp
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

#include <circle/logger.h>
#include "mididevice.h"
#include "minidexed.h"
#include "config.h"
#include <stdio.h>
#include <assert.h>
#include "midi.h"
#include "userinterface.h"

LOGMODULE ("mididevice");

// MIDI "System" level (i.e. all TG) custom CC maps
// Note: Even if number of TGs is not 8, there are only 8
//       available to be used in the mappings here.
#define NUM_MIDI_CC_MAPS 8
const unsigned MIDISystemCCMap[NUM_MIDI_CC_MAPS][8] = {
	{0,0,0,0,0,0,0,0}, // 0 = disabled
	{16,17,18,19,80,81,82,83}, // 1 = General Purpose Controllers 1-8
	{20,21,22,23,24,25,26,27},
	{52,53,54,55,56,57,58,59},
	{102,103,104,105,106,107,108,109},
	{110,111,112,113,114,115,116,117},
	{3,9,14,15,28,29,30,31},
	{35,41,46,47,60,61,62,63}
};

#define MIDI_SYSTEM_EXCLUSIVE_BEGIN	0xF0
#define MIDI_SYSTEM_EXCLUSIVE_END	0xF7
#define MIDI_TIMING_CLOCK	0xF8
#define MIDI_ACTIVE_SENSING	0xFE

CMIDIDevice::TDeviceMap CMIDIDevice::s_DeviceMap;

CMIDIDevice::CMIDIDevice (CMiniDexed *pSynthesizer, CConfig *pConfig, CUserInterface *pUI)
:	m_pSynthesizer (pSynthesizer),
	m_pConfig (pConfig),
	m_pUI (pUI)
{
	for (unsigned nTG = 0; nTG < CConfig::AllToneGenerators; nTG++)
	{
		m_ChannelMap[nTG] = Disabled;
		m_PreviousChannelMap[nTG] = Disabled; // Initialize previous channel map
	}

	m_nMIDISystemCCVol = m_pConfig->GetMIDISystemCCVol();
	m_nMIDISystemCCPan = m_pConfig->GetMIDISystemCCPan();
	m_nMIDISystemCCDetune = m_pConfig->GetMIDISystemCCDetune();

	m_MIDISystemCCBitmap[0] = 0;
	m_MIDISystemCCBitmap[1] = 0;
	m_MIDISystemCCBitmap[2] = 0;
	m_MIDISystemCCBitmap[3] = 0;

	m_nMIDIGlobalExpression = m_pConfig->GetMIDIGlobalExpression();
	// convert from config channels 1..16 to internal channels
	if ((m_nMIDIGlobalExpression >= 1) && (m_nMIDIGlobalExpression <= 16)) {
		m_nMIDIGlobalExpression = m_nMIDIGlobalExpression - 1;
	} else {
		// Either disabled or OMNI means disabled
		m_nMIDIGlobalExpression = Disabled;
	}

	for (int tg=0; tg<8; tg++)
	{
		if (m_nMIDISystemCCVol != 0) {
			u8 cc = MIDISystemCCMap[m_nMIDISystemCCVol][tg];
			m_MIDISystemCCBitmap[cc>>5] |= (1<<(cc%32));
		}
		if (m_nMIDISystemCCPan != 0) {
			u8 cc = MIDISystemCCMap[m_nMIDISystemCCPan][tg];
			m_MIDISystemCCBitmap[cc>>5] |= (1<<(cc%32));
		}
		if (m_nMIDISystemCCDetune != 0) {
			u8 cc = MIDISystemCCMap[m_nMIDISystemCCDetune][tg];
			m_MIDISystemCCBitmap[cc>>5] |= (1<<(cc%32));
		}
	}
	if (m_pConfig->GetMIDIDumpEnabled ()) {
		LOGNOTE("MIDI System CC Map: %08X %08X %08X %08X", m_MIDISystemCCBitmap[3],m_MIDISystemCCBitmap[2],m_MIDISystemCCBitmap[1],m_MIDISystemCCBitmap[0]);
	}
}

CMIDIDevice::~CMIDIDevice (void)
{
	m_pSynthesizer = 0;
}

void CMIDIDevice::SetChannel (u8 ucChannel, unsigned nTG)
{
	assert (nTG < CConfig::AllToneGenerators);
	
	// When changing to OMNI mode, store the previous channel
	if (ucChannel == OmniMode && m_ChannelMap[nTG] != OmniMode) {
		m_PreviousChannelMap[nTG] = m_ChannelMap[nTG];
	}
	
	m_ChannelMap[nTG] = ucChannel;
}

u8 CMIDIDevice::GetChannel (unsigned nTG) const
{
	assert (nTG < CConfig::AllToneGenerators);
	return m_ChannelMap[nTG];
}

void CMIDIDevice::MIDIMessageHandler (const u8 *pMessage, size_t nLength, unsigned nCable)
{
	// The packet contents are just normal MIDI data - see
	// https://www.midi.org/specifications/item/table-1-summary-of-midi-message

	if (m_pConfig->GetMIDIDumpEnabled ())
	{
		switch (nLength)
		{
		case 1:
			if (   pMessage[0] != MIDI_TIMING_CLOCK
			    && pMessage[0] != MIDI_ACTIVE_SENSING)
			{
				LOGNOTE("MIDI%u: %02X", nCable, (unsigned) pMessage[0]);
			}
			break;

		case 2:
			LOGNOTE("MIDI%u: %02X %02X", nCable,
				(unsigned) pMessage[0], (unsigned) pMessage[1]);
			break;

		case 3:
			LOGNOTE("MIDI%u: %02X %02X %02X", nCable,
				(unsigned) pMessage[0], (unsigned) pMessage[1],
				(unsigned) pMessage[2]);
			break;
				
		default:
			switch(pMessage[0])
			{
				case MIDI_SYSTEM_EXCLUSIVE_BEGIN:
					LOGNOTE("MIDI%u: SysEx data length: [%d]:",nCable, uint16_t(nLength));
					for (uint16_t i = 0; i < nLength; i++)
					{
						if((i % 16) == 0)
							LOGNOTE("%04d:",i);
						LOGNOTE(" 0x%02x",pMessage[i]);
					}
					LOGNOTE("");
					break;
				default:
					LOGNOTE("MIDI%u: Unhandled MIDI event type %0x02x",nCable,pMessage[0]);
			}
			break;
		}
	}

	// Only for debugging:
/*
	if(pMessage[0]==MIDI_SYSTEM_EXCLUSIVE_BEGIN)
	{
		printf("MIDI%u: SysEx data length: [%d]:",nCable, uint16_t(nLength));
		for (uint16_t i = 0; i < nLength; i++)
		{
			if((i % 16) == 0)
				printf("\n%04d:",i);
			printf(" 0x%02x",pMessage[i]);
		}
		printf("\n");
	}
*/

	// Handle MIDI Thru
	if (m_DeviceName.compare (m_pConfig->GetMIDIThruIn ()) == 0)
	{
		TDeviceMap::const_iterator Iterator;

		Iterator = s_DeviceMap.find (m_pConfig->GetMIDIThruOut ());
		if (Iterator != s_DeviceMap.end ())
		{
			Iterator->second->Send (pMessage, nLength, nCable);
		}
	}

	if (nLength < 2)
	{
		// LOGERR("MIDI message is shorter than 2 bytes!");
		return;
	}

	m_MIDISpinLock.Acquire ();

	u8 ucStatus  = pMessage[0];
	u8 ucChannel = ucStatus & 0x0F;
	u8 ucType    = ucStatus >> 4;

	// GLOBAL MIDI SYSEX

	// Set MIDI Channel for TX816/TX216 SysEx; in MiniDexed, we interpret the device parameter as the number of the TG (unlike the TX816/TX216 which has a hardware switch to select the TG)
	if (nLength == 7 &&
	    pMessage[0] == MIDI_SYSTEM_EXCLUSIVE_BEGIN &&
	    pMessage[1] == 0x43 &&
	    // pMessage[2] & 0x0F = TG number
	    pMessage[3] == 0x04 &&
	    pMessage[4] == 0x01 &&
	    // pMessage[5] = target MIDI channel (0-15)
	    pMessage[6] == MIDI_SYSTEM_EXCLUSIVE_END)
	{
		uint8_t mTG = pMessage[2] & 0x0F;
		uint8_t val = pMessage[5];
		LOGNOTE("MIDI-SYSEX: Set TG%d to MIDI Channel %d", mTG + 1, val & 0x0F);
		m_pSynthesizer->SetMIDIChannel(val & 0x0F, mTG);
	}
	// Master Volume is set using a MIDI SysEx message as follows:
	//   F0  Start of SysEx
	//   7F  System Realtime SysEx
	//   7F  SysEx "channel" - 7F = all devices
	//   04  Device Control
	//   01  Master Volume Device Control
	//   LL  Low 7-bits of 14-bit volume
	//   HH  High 7-bits of 14-bit volume
	//   F7  End SysEx
	//
	//  See MIDI Specification "Device Control"
	//   "Master Volume and Master Balance"
	//
	// Need to scale the volume parameter to fit
	// a 14-bit value: 0..16383
	// and then split into LSB/MSB.	
	else if (nLength == 8 &&
	    pMessage[0] == MIDI_SYSTEM_EXCLUSIVE_BEGIN &&
	    pMessage[1] == 0x7F &&
	    pMessage[2] == 0x7F &&
	    pMessage[3] == 0x04 &&
	    pMessage[4] == 0x01 &&
	    // pMessage[5] and pMessage[6] = LSB+MSB
	    pMessage[7] == MIDI_SYSTEM_EXCLUSIVE_END
	  ) // MASTER VOLUME
	{
		// Convert LSB/MSB to 14-bit integer volume
		uint32_t nMasterVolume=((pMessage[5] & 0x7F) | ((pMessage[6] & 0x7F) <<7));
		m_pSynthesizer->SetParameter (CMiniDexed::ParameterMasterVolume, maplong (nMasterVolume, 0, 16383, 0, 127));
	}
	else
	{
		// Perform any MiniDexed level MIDI handling before specific Tone Generators
		unsigned nPerfCh = m_pSynthesizer->GetPerformanceSelectChannel();
		switch (ucType)
		{
		case MIDI_CONTROL_CHANGE:
			// Check for performance PC messages
			if (nPerfCh != Disabled)
			{
				if ((ucChannel == nPerfCh) || (nPerfCh == OmniMode))
				{
					if (pMessage[1] == MIDI_CC_BANK_SELECT_MSB)
					{
						m_pSynthesizer->BankSelectMSBPerformance (pMessage[2]);
					}
					else if (pMessage[1] == MIDI_CC_BANK_SELECT_LSB)
					{
						m_pSynthesizer->BankSelectLSBPerformance (pMessage[2]);
					}
					else
					{
						// Ignore any other CC messages at this time
						LOGNOTE("Ignoring CC %d (%d) on Performance Select Channel %d\n", pMessage[1], pMessage[2], nPerfCh);
					}
				}
			}
			if (m_nMIDIGlobalExpression != Disabled)
			{
				// Expression is global so check for expression MIDI channel
				// NB: OMNI not supported
				if (ucChannel == m_nMIDIGlobalExpression) {
					// Send to all TGs regardless of their own channel
					for (unsigned nTG = 0; nTG < m_pConfig->GetToneGenerators(); nTG++) {
						m_pSynthesizer->SetExpression (pMessage[2], nTG);
					}
				}
			}
			if (nLength == 3)
			{
				m_pUI->UIMIDICmdHandler (ucChannel, ucType, pMessage[1], pMessage[2]);
			}
			break;

		case MIDI_NOTE_OFF:
		case MIDI_NOTE_ON:
			if (nLength < 3)
			{
				break;
			}
			m_pUI->UIMIDICmdHandler (ucChannel, ucType, pMessage[1], pMessage[2]);
			break;

		case MIDI_PROGRAM_CHANGE:
			// Check for performance PC messages
			if( m_pConfig->GetMIDIRXProgramChange() )
			{
				if( nPerfCh != Disabled)
				{
					if ((ucChannel == nPerfCh) || (nPerfCh == OmniMode))
					{
						//printf("Performance Select Channel %d\n", nPerfCh);
						m_pSynthesizer->ProgramChangePerformance (pMessage[1]);
					}
				}
			}
			break;
		}

		// Process MIDI for each active Tone Generator
		bool bSystemCCHandled = false;
		bool bSystemCCChecked = false;
		if (ucStatus == MIDI_SYSTEM_EXCLUSIVE_BEGIN) {
			uint8_t ucSysExChannel = (pMessage[2] & 0x0F);
			for (unsigned nTG = 0; nTG < m_pConfig->GetToneGenerators(); nTG++) {
				if (m_ChannelMap[nTG] == ucSysExChannel || m_ChannelMap[nTG] == OmniMode) {
					LOGNOTE("MIDI-SYSEX: channel: %u, len: %u, TG: %u",m_ChannelMap[nTG],nLength,nTG);

					// Check for TX216/TX816 style performance sysex messages
					
					if (nLength == 7 && pMessage[3] == 0x04)
					{
						// TX816/TX216 Performance SysEx message
						uint8_t mTG = pMessage[2] & 0x0F; // mTG = module/tone generator number (0-7)
						uint8_t par = pMessage[4];
						uint8_t val = pMessage[5];

						if (!(m_ChannelMap[nTG] == mTG || m_ChannelMap[nTG] == OmniMode)) continue;

						LOGNOTE("MIDI-SYSEX: Assuming TX216/TX816 style performance sysex message because 4th byte is 0x04");

						switch (par)
						{
						case 2: // Poly/Mono
							LOGNOTE("MIDI-SYSEX: Set Poly/Mono %d to %d", nTG, val & 0x0F);
							m_pSynthesizer->setMonoMode(val ? true : false, nTG);
							break;
						case 3: // Pitch Bend Range
							LOGNOTE("MIDI-SYSEX: Set Pitch Bend Range %d to %d", nTG, val & 0x0F);
							m_pSynthesizer->setPitchbendRange(val, nTG);
							break;
						case 4: // Pitch Bend Step
							LOGNOTE("MIDI-SYSEX: Set Pitch Bend Step %d to %d", nTG, val & 0x0F);
							m_pSynthesizer->setPitchbendStep(val, nTG);
							break;
						case 5: // Portamento Time
							LOGNOTE("MIDI-SYSEX: Set Portamento Time %d to %d", nTG, val & 0x0F);
							m_pSynthesizer->setPortamentoTime(val, nTG);
							break;
						case 6: // Portamento/Glissando
							LOGNOTE("MIDI-SYSEX: Set Portamento/Glissando %d to %d", nTG, val & 0x0F);
							m_pSynthesizer->setPortamentoGlissando(val, nTG);
							break;
						case 7: // Portamento Mode
							LOGNOTE("MIDI-SYSEX: Set Portamento Mode %d to %d", nTG, val & 0x0F);
							m_pSynthesizer->setPortamentoMode(val, nTG);
							break;
						case 9: // Mod Wheel Sensitivity
						{
							int scaled = (val * 99) / 15;
							LOGNOTE("MIDI-SYSEX: Set Mod Wheel Sensitivity %d to %d (scaled %d)", nTG, val & 0x0F, scaled);
							m_pSynthesizer->setModWheelRange(scaled, nTG);
						}
						break;
						case 10: // Mod Wheel Assign
							LOGNOTE("MIDI-SYSEX: Set Mod Wheel Assign %d to %d", nTG, val & 0x0F);
							m_pSynthesizer->setModWheelTarget(val, nTG);
							break;
						case 11: // Foot Controller Sensitivity
						{
							int scaled = (val * 99) / 15;
							LOGNOTE("MIDI-SYSEX: Set Foot Controller Sensitivity %d to %d (scaled %d)", nTG, val & 0x0F, scaled);
							m_pSynthesizer->setFootControllerRange(scaled, nTG);
						}
						break;
						case 12: // Foot Controller Assign
							LOGNOTE("MIDI-SYSEX: Set Foot Controller Assign %d to %d", nTG, val & 0x0F);
							m_pSynthesizer->setFootControllerTarget(val, nTG);
							break;
						case 13: // Aftertouch Sensitivity
						{
							int scaled = (val * 99) / 15;
							LOGNOTE("MIDI-SYSEX: Set Aftertouch Sensitivity %d to %d (scaled %d)", nTG, val & 0x0F, scaled);
							m_pSynthesizer->setAftertouchRange(scaled, nTG);
						}
						break;
						case 14: // Aftertouch Assign
							LOGNOTE("MIDI-SYSEX: Set Aftertouch Assign %d to %d", nTG, val & 0x0F);
							m_pSynthesizer->setAftertouchTarget(val, nTG);
							break;
						case 15: // Breath Controller Sensitivity
						{
							int scaled = (val * 99) / 15;
							LOGNOTE("MIDI-SYSEX: Set Breath Controller Sensitivity %d to %d (scaled %d)", nTG, val & 0x0F, scaled);
							m_pSynthesizer->setBreathControllerRange(scaled, nTG);
						}
						break;
						case 16: // Breath Controller Assign
							LOGNOTE("MIDI-SYSEX: Set Breath Controller Assign %d to %d", nTG, val & 0x0F);
							m_pSynthesizer->setBreathControllerTarget(val, nTG);
							break;
						case 26: // Audio Output Level Attenuator
							{
								LOGNOTE("MIDI-SYSEX: Set Audio Output Level Attenuator %d to %d", nTG, val & 0x0F);
								// Example: F0 43 10 04 1A 00 F7 to F0 43 10 04 1A 07 F7
								unsigned attenVal = val & 0x07;
								// unsigned newVolume = (unsigned)(127.0 * pow(attenVal / 7.0, 2.0) + 0.5); // Logarithmic mapping
								// But on the T816, there is an exponential (not logarithmic!) mapping, and 0 results in the same volume as 1:
								// 7=127, 6=63, 5=31, 4=15, 3=7, 2=3, 1=1, 0=1
								unsigned newVolume = (attenVal == 0) ? 0 : (127 >> (7 - attenVal));
								if (newVolume == 0) newVolume = 1; // 0 is like 1 to avoid silence
								m_pSynthesizer->SetVolume(newVolume, nTG);
							}
							break;
						case 64: // Master Tuning
							LOGNOTE("MIDI-SYSEX: Set Master Tuning");
							// TX812 scales from -75 to +75 cents.
							m_pSynthesizer->SetMasterTune(maplong(val, 1, 127, -37, 37), nTG); // Would need 37.5 here, due to wrong constrain on dexed_synth module?
							break;
						default:
							// Unknown or unsupported parameter
							LOGNOTE("MIDI-SYSEX: Unknown parameter %d for TG %d", par, nTG);
							break;
						}
					}
					else
					{
						HandleSystemExclusive(pMessage, nLength, nCable, nTG);
						if (nLength == 5) {
							break; // Send dump request only to the first TG that matches the MIDI channel requested via the SysEx message device ID
						}
					}
				}
			}
		} else {
			for (unsigned nTG = 0; nTG < m_pConfig->GetToneGenerators() && !bSystemCCHandled; nTG++) {
				if (   m_ChannelMap[nTG] == ucChannel
				    || m_ChannelMap[nTG] == OmniMode) {
					switch (ucType)
					{
					case MIDI_NOTE_ON:
						if (nLength < 3)
						{
							break;
						}
		
						if (pMessage[2] > 0)
						{
							if (pMessage[2] <= 127)
							{
								m_pSynthesizer->keydown (pMessage[1],
											 pMessage[2], nTG);
							}
						}
						else
						{
							m_pSynthesizer->keyup (pMessage[1], nTG);
						}
						break;
		
					case MIDI_NOTE_OFF:
						if (nLength < 3)
						{
							break;
						}
		
						m_pSynthesizer->keyup (pMessage[1], nTG);
						break;
		
					case MIDI_CHANNEL_AFTERTOUCH:
						
						m_pSynthesizer->setAftertouch (pMessage[1], nTG);
						m_pSynthesizer->ControllersRefresh (nTG);
						break;
							
					case MIDI_CONTROL_CHANGE:
						if (nLength < 3)
						{
							break;
						}
		
						switch (pMessage[1])
						{
						case MIDI_CC_MODULATION:
							m_pSynthesizer->setModWheel (pMessage[2], nTG);
							m_pSynthesizer->ControllersRefresh (nTG);
							break;
								
						case MIDI_CC_FOOT_PEDAL:
							m_pSynthesizer->setFootController (pMessage[2], nTG);
							m_pSynthesizer->ControllersRefresh (nTG);
							break;

						case MIDI_CC_PORTAMENTO_TIME:
							m_pSynthesizer->setPortamentoTime (maplong (pMessage[2], 0, 127, 0, 99), nTG);
							break;

						case MIDI_CC_BREATH_CONTROLLER:
							m_pSynthesizer->setBreathController (pMessage[2], nTG);
							m_pSynthesizer->ControllersRefresh (nTG);
							break;
								
						case MIDI_CC_VOLUME:
							m_pSynthesizer->SetVolume (pMessage[2], nTG);
							break;
		
						case MIDI_CC_PAN_POSITION:
							m_pSynthesizer->SetPan (pMessage[2], nTG);
							break;
		
						case MIDI_CC_EXPRESSION:
							if (m_nMIDIGlobalExpression == Disabled) {
								// Expression is per channel only
								m_pSynthesizer->SetExpression (pMessage[2], nTG);
							}
							break;
		
						case MIDI_CC_BANK_SELECT_MSB:
							m_pSynthesizer->BankSelectMSB (pMessage[2], nTG);
							break;
		
						case MIDI_CC_BANK_SELECT_LSB:
							m_pSynthesizer->BankSelectLSB (pMessage[2], nTG);
							break;
		
						case MIDI_CC_SUSTAIN:
							m_pSynthesizer->setSustain (pMessage[2] >= 64, nTG);
							break;

						case MIDI_CC_SOSTENUTO:
							m_pSynthesizer->setSostenuto (pMessage[2] >= 64, nTG);
							break;

						case MIDI_CC_PORTAMENTO:
							m_pSynthesizer->setPortamentoMode (pMessage[2] >= 64, nTG);
							break;

						case MIDI_CC_HOLD2:
							m_pSynthesizer->setHoldMode (pMessage[2] >= 64, nTG);
							break;

						case MIDI_CC_RESONANCE:
							m_pSynthesizer->SetResonance (maplong (pMessage[2], 0, 127, 0, 99), nTG);
							break;
							
						case MIDI_CC_FREQUENCY_CUTOFF:
							m_pSynthesizer->SetCutoff (maplong (pMessage[2], 0, 127, 0, 99), nTG);
							break;
		
						case MIDI_CC_REVERB_LEVEL:
							m_pSynthesizer->SetReverbSend (maplong (pMessage[2], 0, 127, 0, 99), nTG);
							break;
		
						case MIDI_CC_DETUNE_LEVEL:
							if (pMessage[2] == 0)
							{
								// 0 to 127, with 0 being no detune effect applied at all
								m_pSynthesizer->SetMasterTune (0, nTG);
							}
							else
							{
								// Scale to -99 to +99 cents
								m_pSynthesizer->SetMasterTune (maplong (pMessage[2], 1, 127, -99, 99), nTG);
							}
							break;
		
						case MIDI_CC_ALL_SOUND_OFF:
							m_pSynthesizer->panic (pMessage[2], nTG);
							break;
		
						case MIDI_CC_ALL_NOTES_OFF:
							// As per "MIDI 1.0 Detailed Specification" v4.2
							// From "ALL NOTES OFF" states:
							// "Receivers should ignore an All Notes Off message while Omni is on (Modes 1 & 2)"
							if (!m_pConfig->GetIgnoreAllNotesOff () && m_ChannelMap[nTG] != OmniMode)
							{
								m_pSynthesizer->notesOff (pMessage[2], nTG);
							}
							break;

						case MIDI_CC_OMNI_MODE_OFF:
							// Sets to "Omni Off" mode
							if (m_ChannelMap[nTG] == OmniMode) {
								// Restore the previous channel if available, otherwise use current channel
								u8 channelToRestore = (m_PreviousChannelMap[nTG] != Disabled) ? 
									m_PreviousChannelMap[nTG] : ucChannel;
								m_pSynthesizer->SetMIDIChannel(channelToRestore, nTG);
								LOGDBG("Omni Mode Off: TG %d restored to MIDI channel %d", nTG, channelToRestore+1);
							}
							break;
						
						case MIDI_CC_OMNI_MODE_ON:
							// Sets to "Omni On" mode
							m_pSynthesizer->SetMIDIChannel(OmniMode, nTG);
							LOGDBG("Omni Mode On: TG %d set to OMNI", nTG);
							break;

						case MIDI_CC_MONO_MODE_ON:
							// Sets monophonic mode
							m_pSynthesizer->setMonoMode(1, nTG);
							LOGDBG("Mono Mode On: TG %d set to MONO", nTG);
							break;

						case MIDI_CC_POLY_MODE_ON:
							// Sets polyphonic mode
							m_pSynthesizer->setMonoMode(0, nTG);
							LOGDBG("Poly Mode On: TG %d set to POLY", nTG);
							break;

						default:
							// Check for system-level, cross-TG MIDI Controls, but only do it once.
							// Also, if successfully handled, then no need to process other TGs,
							// so it is possible to break out of the main TG loop too.
							// Note: We handle this here so we get the TG MIDI channel checking.
							if (!bSystemCCChecked) {
								bSystemCCHandled = HandleMIDISystemCC(pMessage[1], pMessage[2]);
								bSystemCCChecked = true;
							}
							break;
						}
						break;
		
					case MIDI_PROGRAM_CHANGE:
						// do program change only if enabled in config and not in "Performance Select Channel" mode
						if( m_pConfig->GetMIDIRXProgramChange() && ( m_pSynthesizer->GetPerformanceSelectChannel() == Disabled) ) {
							//printf("Program Change to %d (%d)\n", ucChannel, m_pSynthesizer->GetPerformanceSelectChannel());
							m_pSynthesizer->ProgramChange (pMessage[1], nTG);
						}
						break;
		
					case MIDI_PITCH_BEND: {
						if (nLength < 3)
						{
							break;
						}
		
						s16 nValue = pMessage[1];
						nValue |= (s16) pMessage[2] << 7;
						nValue -= 0x2000;
		
						m_pSynthesizer->setPitchbend (nValue, nTG);
						} break;
		
					default:
						break;
					}
				}
			}
		}
	}
	m_MIDISpinLock.Release ();
}

void CMIDIDevice::AddDevice (const char *pDeviceName)
{
	assert (pDeviceName);

	assert (m_DeviceName.empty ());
	m_DeviceName = pDeviceName;
	assert (!m_DeviceName.empty ());

	s_DeviceMap.insert (std::pair<std::string, CMIDIDevice *> (pDeviceName, this));
}

bool CMIDIDevice::HandleMIDISystemCC(const u8 ucCC, const u8 ucCCval)
{
	// This only makes sense when there are at least 8 TGs.
	// Note: If more than 8 TGs then only 8 TGs are controllable this way.
	if (m_pConfig->GetToneGenerators() < 8) {
		return false;
	}

	// Quickly reject any CCs not in the configured maps
	if ((m_MIDISystemCCBitmap[ucCC>>5] & (1<<(ucCC%32))) == 0) {
		// Not in the map
		return false;
	}

	// Not looking for duplicate CCs so return once handled
	for (unsigned tg=0; tg<8; tg++) {
		if (m_nMIDISystemCCVol != 0) {
			if (ucCC == MIDISystemCCMap[m_nMIDISystemCCVol][tg]) {
				m_pSynthesizer->SetVolume (ucCCval, tg);
				return true;
			}
		}
		if (m_nMIDISystemCCPan != 0) {
			if (ucCC == MIDISystemCCMap[m_nMIDISystemCCPan][tg]) {
				m_pSynthesizer->SetPan (ucCCval, tg);
				return true;
			}
		}
		if (m_nMIDISystemCCDetune != 0) {
			if (ucCC == MIDISystemCCMap[m_nMIDISystemCCDetune][tg]) {
				if (ucCCval == 0)
				{
					// 0 to 127, with 0 being no detune effect applied at all
					m_pSynthesizer->SetMasterTune (0, tg);
				}
				else
				{
					// Scale to -99 to +99 cents
					m_pSynthesizer->SetMasterTune (maplong (ucCCval, 1, 127, -99, 99), tg);
				}
				return true;
			}
		}
	}
	
	return false;
}

void CMIDIDevice::HandleSystemExclusive(const uint8_t* pMessage, const size_t nLength, const unsigned nCable, const uint8_t nTG)
{

  LOGDBG("HandleSystemExclusive: TG %d, length %zu", nTG, nLength);

  // Check if it is a dump request; these have the format F0 43 2n ff F7
  // with n = the MIDI channel and ff = 00 for voice or 09 for bank
  // It was confirmed that on the TX816, the device number is interpreted as the MIDI channel; 
  if (nLength == 5 && pMessage[3] == 0x00)
  {
	LOGDBG("SysEx voice dump request: device %d", nTG);
	SendSystemExclusiveVoice(nTG, m_DeviceName, nCable, nTG);
	return;
  }
  else if (nLength == 5 && pMessage[3] == 0x09)
  {
	LOGDBG("SysEx bank dump request: device %d", nTG);
	LOGDBG("Still to be implemented");
	return;
  }

  int16_t sysex_return;

  sysex_return = m_pSynthesizer->checkSystemExclusive(pMessage, nLength, nTG);
  LOGDBG("SYSEX handler return value: %d", sysex_return);

  switch (sysex_return)
  {
    case -1:
      LOGERR("SysEx end status byte not detected.");
      break;
    case -2:
      LOGERR("SysEx vendor not Yamaha.");
      break;
    case -3:
      LOGERR("Unknown SysEx parameter change.");
      break;
    case -4:
      LOGERR("Unknown SysEx voice or function.");
      break;
    case -5:
      LOGERR("Not a SysEx voice bulk upload.");
      break;
    case -6:
      LOGERR("Wrong length for SysEx voice bulk upload (not 155).");
      break;
    case -7:
      LOGERR("Checksum error for one voice.");
      break;
    case -8:
      LOGERR("Not a SysEx bank bulk upload.");
      break;
    case -9:
      LOGERR("Wrong length for SysEx bank bulk upload (not 4096).");
    case -10:
      LOGERR("Checksum error for bank.");
      break;
    case -11:
      LOGERR("Unknown SysEx message.");
      break;
    case 100:
      // load sysex-data into voice memory
      LOGDBG("One Voice bulk upload");
      m_pSynthesizer->loadVoiceParameters(pMessage,nTG);
      break;
    case 200:
      LOGDBG("Bank bulk upload.");
      //TODO: add code for storing a bank bulk upload
      LOGNOTE("Currently code  for storing a bulk bank upload is missing!");
      break;
    case 455:
      // Parameter 155 + 300 added by Synth_Dexed = 455
      LOGDBG("Operators enabled: %d%d%d%d%d%d", (pMessage[5] & 0x20) ? 1 : 0, (pMessage[5] & 0x10) ? 1 : 0, (pMessage[5] & 0x08) ? 1 : 0, (pMessage[5] & 0x04) ? 1 : 0, (pMessage[5] & 0x02) ? 1 : 0, (pMessage[5] & 0x01) ? 1 : 0);
      m_pSynthesizer->setOPMask(pMessage[5], nTG);
      break;
    default:
      if(sysex_return >= 300 && sysex_return < 500)
      {
        LOGDBG("SysEx voice parameter change: Parameter %d value: %d",pMessage[4] + ((pMessage[3] & 0x03) * 128), pMessage[5]);
        m_pSynthesizer->setVoiceDataElement(pMessage[4] + ((pMessage[3] & 0x03) * 128), pMessage[5],nTG);
        switch(pMessage[4] + ((pMessage[3] & 0x03) * 128))
        {
          case 134:
            m_pSynthesizer->notesOff(0,nTG);
            break;
        }
      }
      else if(sysex_return >= 500 && sysex_return < 600)
      {
        LOGDBG("SysEx send voice %u request",sysex_return-500);
        SendSystemExclusiveVoice(sysex_return-500, m_DeviceName, nCable, nTG);
      }
      break;
  }
}

void CMIDIDevice::SendSystemExclusiveVoice(uint8_t nVoice, const std::string& deviceName, unsigned nCable, uint8_t nTG)
{
	// Example: F0 43 20 00 F7
    uint8_t voicedump[163];
    m_pSynthesizer->getSysExVoiceDump(voicedump, nTG);
    TDeviceMap::const_iterator Iterator = s_DeviceMap.find(deviceName);
    if (Iterator != s_DeviceMap.end()) {
        Iterator->second->Send(voicedump, sizeof(voicedump), nCable);
        LOGDBG("Send SYSEX voice dump %u to \"%s\"", nVoice, deviceName.c_str());
    } else {
        LOGWARN("No device found in s_DeviceMap for name: %s", deviceName.c_str());
    }
}
