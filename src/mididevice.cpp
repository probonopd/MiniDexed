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
#include "mididevice.h"
#include "minidexed.h"
#include "config.h"
#include <stdio.h>
#include <assert.h>

#define MIDI_NOTE_OFF		0b1000
#define MIDI_NOTE_ON		0b1001
#define MIDI_AFTERTOUCH		0b1010			// TODO
#define MIDI_CONTROL_CHANGE	0b1011
	#define MIDI_CC_BANK_SELECT_MSB		0	// TODO
	#define MIDI_CC_MODULATION			1
	#define MIDI_CC_VOLUME				7
	#define MIDI_CC_PAN_POSITION		10
	#define MIDI_CC_BANK_SELECT_LSB		32
	#define MIDI_CC_BANK_SUSTAIN		64
	#define MIDI_CC_RESONANCE			71
	#define MIDI_CC_FREQUENCY_CUTOFF	74
	#define MIDI_CC_REVERB_LEVEL		91
	#define MIDI_CC_DETUNE_LEVEL		94
	#define MIDI_CC_ALL_SOUND_OFF		120
	#define MIDI_CC_ALL_NOTES_OFF		123
#define MIDI_PROGRAM_CHANGE	0b1100
#define MIDI_PITCH_BEND		0b1110

#define MIDI_TIMING_CLOCK	0xF8
#define MIDI_ACTIVE_SENSING	0xFE

CMIDIDevice::TDeviceMap CMIDIDevice::s_DeviceMap;

CMIDIDevice::CMIDIDevice (CMiniDexed *pSynthesizer, CConfig *pConfig)
:	m_pSynthesizer (pSynthesizer),
	m_pConfig (pConfig)
{
	for (unsigned nTG = 0; nTG < CConfig::ToneGenerators; nTG++)
	{
		m_ChannelMap[nTG] = Disabled;
	}
}

CMIDIDevice::~CMIDIDevice (void)
{
	m_pSynthesizer = 0;
}

void CMIDIDevice::SetChannel (u8 ucChannel, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	m_ChannelMap[nTG] = ucChannel;
}

u8 CMIDIDevice::GetChannel (unsigned nTG) const
{
	assert (nTG < CConfig::ToneGenerators);
	return m_ChannelMap[nTG];
}

void CMIDIDevice::MIDIMessageHandler (const u8 *pMessage, size_t nLength, unsigned nCable)
{
	assert (m_pSynthesizer != 0);

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
				printf ("MIDI%u: %02X\n", nCable, (unsigned) pMessage[0]);
			}
			break;

		case 2:
			printf ("MIDI%u: %02X %02X\n", nCable,
				(unsigned) pMessage[0], (unsigned) pMessage[1]);
			break;

		case 3:
			printf ("MIDI%u: %02X %02X %02X\n", nCable,
				(unsigned) pMessage[0], (unsigned) pMessage[1],
				(unsigned) pMessage[2]);
			break;
		}
	}

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
		return;
	}

	u8 ucStatus  = pMessage[0];
	u8 ucChannel = ucStatus & 0x0F;
	u8 ucType    = ucStatus >> 4;

	for (unsigned nTG = 0; nTG < CConfig::ToneGenerators; nTG++)
	{
		if (   m_ChannelMap[nTG] == ucChannel
		    || m_ChannelMap[nTG] == OmniMode)
		{
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

				case MIDI_CC_VOLUME:
					m_pSynthesizer->SetVolume (pMessage[2], nTG);
					break;

				case MIDI_CC_PAN_POSITION:
					m_pSynthesizer->SetPan (pMessage[2], nTG);
					break;

				case MIDI_CC_BANK_SELECT_LSB:
					m_pSynthesizer->BankSelectLSB (pMessage[2], nTG);
					break;

				case MIDI_CC_BANK_SUSTAIN:
					m_pSynthesizer->setSustain (pMessage[2] >= 64, nTG);
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
						// "0 to 127, with 0 being no celeste (detune) effect applied at all."
						m_pSynthesizer->SetMasterTune (0, nTG);
					}
					else
					{
						m_pSynthesizer->SetMasterTune (maplong (pMessage[2], 1, 127, -99, 99), nTG);
					}
					break;

				case MIDI_CC_ALL_SOUND_OFF:
					m_pSynthesizer->panic (pMessage[2], nTG);
					break;

				case MIDI_CC_ALL_NOTES_OFF:
					m_pSynthesizer->notesOff (pMessage[2], nTG);
					break;
				}
				break;

			case MIDI_PROGRAM_CHANGE:
				// do program change only if enabled in config
				if( m_pConfig->GetMIDIRXProgramChange() )
					m_pSynthesizer->ProgramChange (pMessage[1], nTG);
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

void CMIDIDevice::AddDevice (const char *pDeviceName)
{
	assert (pDeviceName);

	assert (m_DeviceName.empty ());
	m_DeviceName = pDeviceName;
	assert (!m_DeviceName.empty ());

	s_DeviceMap.insert (std::pair<std::string, CMIDIDevice *> (pDeviceName, this));
}
