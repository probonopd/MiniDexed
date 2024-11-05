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
	m_pUI (pUI),
	m_pRouteMap ()
{
	for (unsigned nTG = 0; nTG < CConfig::AllToneGenerators; nTG++)
	{
		m_ChannelMap[nTG] = Disabled;
	}

	m_nMIDISystemCCVol = m_pConfig->GetMIDISystemCCVol();
	m_nMIDISystemCCPan = m_pConfig->GetMIDISystemCCPan();
	m_nMIDISystemCCDetune = m_pConfig->GetMIDISystemCCDetune();

	m_MIDISystemCCBitmap[0] = 0;
	m_MIDISystemCCBitmap[1] = 0;
	m_MIDISystemCCBitmap[2] = 0;
	m_MIDISystemCCBitmap[3] = 0;

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
				fprintf (stderr, "MIDI%u: %02X\n", nCable, (unsigned) pMessage[0]);
			}
			break;

		case 2:
			fprintf (stderr, "MIDI%u: %02X %02X\n", nCable,
				(unsigned) pMessage[0], (unsigned) pMessage[1]);
			break;

		case 3:
			fprintf (stderr, "MIDI%u: %02X %02X %02X\n", nCable,
				(unsigned) pMessage[0], (unsigned) pMessage[1],
				(unsigned) pMessage[2]);
			break;
				
		default:
			switch(pMessage[0])
			{
				case MIDI_SYSTEM_EXCLUSIVE_BEGIN:
					fprintf(stderr, "MIDI%u: SysEx data length: [%d]:",nCable, uint16_t(nLength));
					for (uint16_t i = 0; i < nLength; i++)
					{
						if((i % 16) == 0)
							fprintf(stderr, "\n%04d:",i);
						fprintf(stderr, " 0x%02x",pMessage[i]);
					}
					fprintf(stderr, "\n");
					break;
				default:
					fprintf(stderr, "MIDI%u: Unhandled MIDI event type %0x02x\n",nCable,pMessage[0]);
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

	u8 ucCable   = nCable;
	u8 ucStatus  = pMessage[0];
	u8 ucChannel = ucStatus & 0x0F;
	u8 ucType    = ucStatus >> 4;
	u8 ucP1      = pMessage[1];
	u8 ucP2      = nLength >= 3 ? pMessage[2] : 0xFF;
	bool bSkip    = false;

	if (m_pRouteMap)
		GetRoutedMIDI (m_pRouteMap, &ucCable, &ucChannel, &ucType, &ucP1, &ucP2, &bSkip);

	if (bSkip)
	{
		// skip (and release mutex at the end)
	}
	// GLOBAL MIDI SYSEX
	//
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
		// Convert to value between 0.0 and 1.0
		float32_t fMasterVolume = (float32_t)nMasterVolume / 16384.0;
		//printf("Master volume: %f (%d)\n",fMasterVolume, nMasterVolume);
		m_pSynthesizer->setMasterVolume(fMasterVolume);
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
					if (ucP1 == MIDI_CC_BANK_SELECT_MSB)
					{
						m_pSynthesizer->BankSelectMSBPerformance (ucP2);
					}
					else if (ucP1 == MIDI_CC_BANK_SELECT_LSB)
					{
						m_pSynthesizer->BankSelectLSBPerformance (ucP2);
					}
					else
					{
						// Ignore any other CC messages at this time
					}
				}
			}
			if (nLength == 3)
			{
				m_pUI->UIMIDICmdHandler (ucChannel, ucType, ucP1, ucP2);
			}
			break;

		case MIDI_NOTE_OFF:
		case MIDI_NOTE_ON:
			if (nLength < 3)
			{
				break;
			}
			m_pUI->UIMIDICmdHandler (ucChannel, ucType, ucP1, ucP2);
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
						m_pSynthesizer->ProgramChangePerformance (ucP1);
					}
				}
			}
			break;
		}

		// Process MIDI for each active Tone Generator
		bool bSystemCCHandled = false;
		bool bSystemCCChecked = false;
		for (unsigned nTG = 0; nTG < m_pConfig->GetToneGenerators() && !bSystemCCHandled; nTG++)
		{
			if (m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterEnabled, nTG) == 0)
				continue;

			if (ucStatus == MIDI_SYSTEM_EXCLUSIVE_BEGIN)
			{
				// MIDI SYSEX per MIDI channel
				uint8_t ucSysExChannel = (pMessage[2] & 0x0F);
				if (m_ChannelMap[nTG] == ucSysExChannel || m_ChannelMap[nTG] == OmniMode)
				{
					LOGNOTE("MIDI-SYSEX: channel: %u, len: %u, TG: %u",m_ChannelMap[nTG],nLength,nTG);
					HandleSystemExclusive(pMessage, nLength, nCable, nTG);
				}
			}
			else
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
		
						if (ucP2 > 0)
						{
							if (ucP2 <= 127)
							{
								m_pSynthesizer->keydown (ucP1,
											 ucP2, nTG);
							}
						}
						else
						{
							m_pSynthesizer->keyup (ucP1, nTG);
						}
						break;
		
					case MIDI_NOTE_OFF:
						if (nLength < 3)
						{
							break;
						}
		
						m_pSynthesizer->keyup (ucP1, nTG);
						break;
		
					case MIDI_CHANNEL_AFTERTOUCH:
						
						m_pSynthesizer->setAftertouch (ucP1, nTG);
						m_pSynthesizer->ControllersRefresh (nTG);
						break;
							
					case MIDI_CONTROL_CHANGE:
						if (nLength < 3)
						{
							break;
						}
		
						switch (ucP1)
						{
						case MIDI_CC_MODULATION:
							m_pSynthesizer->setModWheel (ucP2, nTG);
							m_pSynthesizer->ControllersRefresh (nTG);
							break;
								
						case MIDI_CC_FOOT_PEDAL:
							m_pSynthesizer->setFootController (ucP2, nTG);
							m_pSynthesizer->ControllersRefresh (nTG);
							break;

						case MIDI_CC_PORTAMENTO_TIME:
							m_pSynthesizer->setPortamentoTime (maplong (ucP2, 0, 127, 0, 99), nTG);
							break;

						case MIDI_CC_BREATH_CONTROLLER:
							m_pSynthesizer->setBreathController (ucP2, nTG);
							m_pSynthesizer->ControllersRefresh (nTG);
							break;
								
						case MIDI_CC_VOLUME:
							m_pSynthesizer->SetVolume (ucP2, nTG);
							break;
		
						case MIDI_CC_PAN_POSITION:
							m_pSynthesizer->SetPan (ucP2, nTG);
							break;
		
						case MIDI_CC_BANK_SELECT_MSB:
							m_pSynthesizer->BankSelectMSB (ucP2, nTG);
							break;
		
						case MIDI_CC_BANK_SELECT_LSB:
							m_pSynthesizer->BankSelectLSB (ucP2, nTG);
							break;
		
						case MIDI_CC_BANK_SUSTAIN:
							m_pSynthesizer->setSustain (ucP2 >= 64, nTG);
							break;

						case MIDI_CC_SOSTENUTO:
							m_pSynthesizer->setSostenuto (ucP2 >= 64, nTG);
							break;
		
						case MIDI_CC_PORTAMENTO:
							m_pSynthesizer->setPortamentoMode (ucP2 >= 64, nTG);
							break;
		
						case MIDI_CC_RESONANCE:
							m_pSynthesizer->SetResonance (maplong (ucP2, 0, 127, 0, 99), nTG);
							break;
							
						case MIDI_CC_FREQUENCY_CUTOFF:
							m_pSynthesizer->SetCutoff (maplong (ucP2, 0, 127, 0, 99), nTG);
							break;
		
						case MIDI_CC_REVERB_LEVEL:
							m_pSynthesizer->SetReverbSend (maplong (ucP2, 0, 127, 0, 99), nTG);
							break;
		
						case MIDI_CC_DETUNE_LEVEL:
							if (ucP2 == 0)
							{
								// "0 to 127, with 0 being no celeste (detune) effect applied at all."
								m_pSynthesizer->SetMasterTune (0, nTG);
							}
							else
							{
								m_pSynthesizer->SetMasterTune (maplong (ucP2, 1, 127, -99, 99), nTG);
							}
							break;
		
						case MIDI_CC_ALL_SOUND_OFF:
							m_pSynthesizer->panic (ucP2, nTG);
							break;
		
						case MIDI_CC_ALL_NOTES_OFF:
							// As per "MIDI 1.0 Detailed Specification" v4.2
							// From "ALL NOTES OFF" states:
							// "Receivers should ignore an All Notes Off message while Omni is on (Modes 1 & 2)"
							if (!m_pConfig->GetIgnoreAllNotesOff () && m_ChannelMap[nTG] != OmniMode)
							{
								m_pSynthesizer->notesOff (ucP2, nTG);
							}
							break;

						default:
							// Check for system-level, cross-TG MIDI Controls, but only do it once.
							// Also, if successfully handled, then no need to process other TGs,
							// so it is possible to break out of the main TG loop too.
							// Note: We handle this here so we get the TG MIDI channel checking.
							if (!bSystemCCChecked) {
								bSystemCCHandled = HandleMIDISystemCC(ucP1, ucP2);
								bSystemCCChecked = true;
							}
							break;
						}
						break;
		
					case MIDI_PROGRAM_CHANGE:
						// do program change only if enabled in config and not in "Performance Select Channel" mode
						if( m_pConfig->GetMIDIRXProgramChange() && ( m_pSynthesizer->GetPerformanceSelectChannel() == Disabled) ) {
							//printf("Program Change to %d (%d)\n", ucChannel, m_pSynthesizer->GetPerformanceSelectChannel());
							m_pSynthesizer->ProgramChange (ucP1, nTG);
						}
						break;
		
					case MIDI_PITCH_BEND: {
						if (nLength < 3)
						{
							break;
						}
		
						s16 nValue = ucP1;
						nValue |= (s16) ucP2 << 7;
						nValue -= 0x2000;
		
						m_pSynthesizer->setPitchbend (nValue, nTG);
						} break;
		
					default:
						break;
					}
				}
			}
		}

		if (m_pRouteMap)
			MIDIListener(ucCable, ucChannel, ucType, ucP1, ucP2);
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
					m_pSynthesizer->SetMasterTune (0, tg);
				}
				else
				{
					m_pSynthesizer->SetMasterTune (maplong (ucCCval, 1, 127, -99, 99), tg);
				}
				return true;
			}
		}
	}
	
	return false;
}

void CMIDIDevice::SetRouteMap (TMIDIRoute *pRouteMap)
{
	m_pRouteMap = pRouteMap;
}

void CMIDIDevice::HandleSystemExclusive(const uint8_t* pMessage, const size_t nLength, const unsigned nCable, const uint8_t nTG)
{
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
    case 64:
      LOGDBG("SysEx Function parameter change: %d Value %d",pMessage[4],pMessage[5]);
      m_pSynthesizer->setMonoMode(pMessage[5],nTG);
      break;
    case 65:
      LOGDBG("SysEx Function parameter change: %d Value %d",pMessage[4],pMessage[5]);
      m_pSynthesizer->setPitchbendRange(pMessage[5],nTG);
      break;
    case 66:
      LOGDBG("SysEx Function parameter change: %d Value %d",pMessage[4],pMessage[5]);
      m_pSynthesizer->setPitchbendStep(pMessage[5],nTG);
      break;
    case 67:
      LOGDBG("SysEx Function parameter change: %d Value %d",pMessage[4],pMessage[5]);
      m_pSynthesizer->setPortamentoMode(pMessage[5],nTG);
      break;
    case 68:
      LOGDBG("SysEx Function parameter change: %d Value %d",pMessage[4],pMessage[5]);
      m_pSynthesizer->setPortamentoGlissando(pMessage[5],nTG);
      break;
    case 69:
      LOGDBG("SysEx Function parameter change: %d Value %d",pMessage[4],pMessage[5]);
      m_pSynthesizer->setPortamentoTime(pMessage[5],nTG);
      break;
    case 70:
      LOGDBG("SysEx Function parameter change: %d Value %d",pMessage[4],pMessage[5]);
      m_pSynthesizer->setModWheelRange(pMessage[5],nTG);
      break;
    case 71:
      LOGDBG("SysEx Function parameter change: %d Value %d",pMessage[4],pMessage[5]);
      m_pSynthesizer->setModWheelTarget(pMessage[5],nTG);
      break;
    case 72:
      LOGDBG("SysEx Function parameter change: %d Value %d",pMessage[4],pMessage[5]);
      m_pSynthesizer->setFootControllerRange(pMessage[5],nTG);
      break;
    case 73:
      LOGDBG("SysEx Function parameter change: %d Value %d",pMessage[4],pMessage[5]);
      m_pSynthesizer->setFootControllerTarget(pMessage[5],nTG);
      break;
    case 74:
      LOGDBG("SysEx Function parameter change: %d Value %d",pMessage[4],pMessage[5]);
      m_pSynthesizer->setBreathControllerRange(pMessage[5],nTG);
      break;
    case 75:
      LOGDBG("SysEx Function parameter change: %d Value %d",pMessage[4],pMessage[5]);
      m_pSynthesizer->setBreathControllerTarget(pMessage[5],nTG);
      break;
    case 76:
      LOGDBG("SysEx Function parameter change: %d Value %d",pMessage[4],pMessage[5]);
      m_pSynthesizer->setAftertouchRange(pMessage[5],nTG);
      break;
    case 77:
      LOGDBG("SysEx Function parameter change: %d Value %d",pMessage[4],pMessage[5]);
      m_pSynthesizer->setAftertouchTarget(pMessage[5],nTG);
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
        SendSystemExclusiveVoice(sysex_return-500, nCable, nTG);
      }
      break;
  }
}

void CMIDIDevice::MIDIListener (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2)
{
}


void CMIDIDevice::SendSystemExclusiveVoice(uint8_t nVoice, const unsigned nCable, uint8_t nTG)
{
  uint8_t voicedump[163];

  // Get voice sysex dump from TG
  m_pSynthesizer->getSysExVoiceDump(voicedump, nTG);

  TDeviceMap::const_iterator Iterator;

  // send voice dump to all MIDI interfaces
  for(Iterator = s_DeviceMap.begin(); Iterator != s_DeviceMap.end(); ++Iterator)
  {
    Iterator->second->Send (voicedump, sizeof(voicedump)*sizeof(uint8_t));
    // LOGDBG("Send SYSEX voice dump %u to \"%s\"",nVoice,Iterator->first.c_str());
  }
} 

void GetRoutedMIDI (TMIDIRoute *pMap, u8 *pCable, u8 *pCh, u8 *pType, u8 *pP1, u8 *pP2, bool *bSkip)
{
	assert (pMap);
	for (TMIDIRoute *r = pMap; r->ucSCable != 0xFF ; r++)
	{
		if (r->ucSCable == *pCable &&
			(r->ucSCh == *pCh || r->ucSCh >= 16) &&
			(r->ucSType == *pType || r->ucSType >= 16) &&
			(r->ucSP1 == *pP1 || r->ucSP1 > 127) &&
			(r->ucSP2 == *pP2 || r->ucSP2 > 127))
		{
			if (r->ucSType == MIDI_NOTE_ON)
				r->bHandled = false;

			if (r->bSkip ||
				r->bSkipHandled && r->ucSType == MIDI_NOTE_OFF && (r-1)->bHandled ||
				r->bSkipHandled && r->ucSType == MIDI_AFTERTOUCH && (r-2)->bHandled) {
				*bSkip = true;
				return;
			}

			if (r->bSkipHandled && r->ucSType == MIDI_NOTE_OFF)
				(r-1)->bHandled = true;
			if (r->bSkipHandled && r->ucSType == MIDI_AFTERTOUCH)
				(r-2)->bHandled = true;

			*pCh = r->ucDCh;
			*pType = r->ucDType;
			if (r->ucDP1 <= 127)
				*pP1 = r->ucDP1;
			if (r->ucDP1 == TMIDIRoute::P2)
				*pP1 = *pP2;
			if (r->ucDP2 <= 127)
				*pP2 = r->ucDP2;
			if (r->bToggle)
				r->ucDP2 = r->ucDP2 ? 0x0 : 0x7F;
			return;
		}
	}
}
