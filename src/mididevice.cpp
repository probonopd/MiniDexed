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

LOGMODULE ("mididevice");

#define MIDI_NOTE_OFF		0b1000
#define MIDI_NOTE_ON		0b1001
#define MIDI_AFTERTOUCH		0b1010			// TODO
#define MIDI_CONTROL_CHANGE	0b1011
#define MIDI_PROGRAM_CHANGE	0b1100
#define MIDI_PITCH_BEND		0b1110

#define MIDI_SYSTEM_EXCLUSIVE_BEGIN	0xF0
#define MIDI_SYSTEM_EXCLUSIVE_END	0xF7
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
		default:
			switch(pMessage[0])
			{
				case MIDI_SYSTEM_EXCLUSIVE_BEGIN:
					printf("MIDI%u: SysEx data length: [%d]:",nCable, uint16_t(nLength));
					for (uint16_t i = 0; i < nLength; i++)
					{
						if((i % 16) == 0)
							printf("\n%04d:",i);
						printf(" 0x%02x",pMessage[i]);
					}
					printf("\n");
					break;
				default:
					printf("MIDI%u: Unhandled MIDI event type %0x02x\n",nCable,pMessage[0]);
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
		LOGERR("MIDI message is shorter than 2 bytes!");
		return;
	}

	m_MIDISpinLock.Acquire ();

	u8 ucStatus  = pMessage[0];
	u8 ucChannel = ucStatus & 0x0F;
	u8 ucType    = ucStatus >> 4;

	// GLOBAL MIDI SYSEX
	if (pMessage[0] == MIDI_SYSTEM_EXCLUSIVE_BEGIN && pMessage[3] == 0x04 &&  pMessage[4] == 0x01 && pMessage[nLength-1] == MIDI_SYSTEM_EXCLUSIVE_END) // MASTER VOLUME
	{
		float32_t nMasterVolume=((pMessage[5] & 0x7c) & ((pMessage[6] & 0x7c) <<7))/(1<<14);
		LOGNOTE("Master volume: %f",nMasterVolume);
		m_pSynthesizer->setMasterVolume(nMasterVolume);
	}
	else
	{
		for (unsigned nTG = 0; nTG < CConfig::ToneGenerators; nTG++)
		{
			if (ucStatus == MIDI_SYSTEM_EXCLUSIVE_BEGIN)
			{
				// MIDI SYSEX per MIDI channel
				uint8_t ucSysExChannel = (pMessage[2] & 0x07);
				if ( nTG == ucSysExChannel || m_ChannelMap[nTG] == OmniMode )
				{
					LOGNOTE("MIDI-SYSEX: channel: %u, len: %u, TG: %u",m_ChannelMap[nTG],nLength,nTG);
					//printf("MIDI-SYSEX: channel: %u, len: %lu, TG: %u",m_ChannelMap[nTG],nLength,nTG);
					HandleSystemExclusive(pMessage, nLength, nCable, ucSysExChannel);
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
		
						case MIDI_CC_DETUNE_LEVEL+32:
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

void CMIDIDevice::HandleSystemExclusive(const uint8_t* pMessage, const size_t nLength, const unsigned nCable, const uint8_t nTG)
{
  int16_t sysex_return;

  if ( nTG >= CConfig::ToneGenerators ) return;
 
  sysex_return = m_pSynthesizer->checkSystemExclusive(pMessage, nLength, nTG);
  uint8_t instanceID = pMessage[2]&0xF;

  if ( instanceID != nTG ) { printf("WARNING instanceID and nTG do not match!!!!!\n"); }

  LOGDBG("SYSEX handler return value: %d", sysex_return);
  //printf("SYSEX handler return value: %d for TG %i\n", sysex_return, instanceID);

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
      m_pSynthesizer->setMonoMode(pMessage[5], instanceID);
      break;
    case 65:
      LOGDBG("SysEx Function parameter change: %d Value %d",pMessage[4],pMessage[5]);
      m_pSynthesizer->setPitchbendRange(pMessage[5],instanceID);
      break;
    case 66:
      LOGDBG("SysEx Function parameter change: %d Value %d",pMessage[4],pMessage[5]);
      m_pSynthesizer->setPitchbendStep(pMessage[5],instanceID);
      break;
    case 67:
      LOGDBG("SysEx Function parameter change: %d Value %d",pMessage[4],pMessage[5]);
      m_pSynthesizer->setPortamentoMode(pMessage[5],instanceID);
      break;
    case 68:
      LOGDBG("SysEx Function parameter change: %d Value %d",pMessage[4],pMessage[5]);
      m_pSynthesizer->setPortamentoGlissando(pMessage[5],instanceID);
      break;
    case 69:
      LOGDBG("SysEx Function parameter change: %d Value %d",pMessage[4],pMessage[5]);
      m_pSynthesizer->setPortamentoTime(pMessage[5],instanceID);
      break;
    case 70:
      LOGDBG("SysEx Function parameter change: %d Value %d",pMessage[4],pMessage[5]);
      m_pSynthesizer->setModWheelRange(pMessage[5],instanceID);
      break;
    case 71:
      LOGDBG("SysEx Function parameter change: %d Value %d",pMessage[4],pMessage[5]);
      m_pSynthesizer->setModWheelTarget(pMessage[5],instanceID);
      break;
    case 72:
      LOGDBG("SysEx Function parameter change: %d Value %d",pMessage[4],pMessage[5]);
      m_pSynthesizer->setFootControllerRange(pMessage[5],instanceID);
      break;
    case 73:
      LOGDBG("SysEx Function parameter change: %d Value %d",pMessage[4],pMessage[5]);
      m_pSynthesizer->setFootControllerTarget(pMessage[5],instanceID);
      break;
    case 74:
      LOGDBG("SysEx Function parameter change: %d Value %d",pMessage[4],pMessage[5]);
      m_pSynthesizer->setBreathControllerRange(pMessage[5],instanceID);
      break;
    case 75:
      LOGDBG("SysEx Function parameter change: %d Value %d",pMessage[4],pMessage[5]);
      m_pSynthesizer->setBreathControllerTarget(pMessage[5],instanceID);
      break;
    case 76:
      LOGDBG("SysEx Function parameter change: %d Value %d",pMessage[4],pMessage[5]);
      m_pSynthesizer->setAftertouchRange(pMessage[5],instanceID);
      break;
    case 77:
      LOGDBG("SysEx Function parameter change: %d Value %d",pMessage[4],pMessage[5]);
      m_pSynthesizer->setAftertouchTarget(pMessage[5],instanceID);
      break;
/* BeZo patches */
    case 78:						// bank select
	LOGDBG("Bank Select for TG %i\n", instanceID);
	m_pSynthesizer->BankSelectLSB (pMessage[5], instanceID);
	break;
    case 79:						// pgm select
	LOGDBG("Patch Select for TG %i\n", instanceID);
	m_pSynthesizer->ProgramChange (pMessage[5], instanceID);
	break;
    case 80:						// Set midi channel
	LOGDBG("Set midi channel for TG %i", instanceID);
	m_pSynthesizer->SetMIDIChannel(pMessage[5], instanceID);
	break;
    case 81:						// Set Cutoff
	LOGDBG("Set Cutoff for TG %i", instanceID);
        m_pSynthesizer->SetCutoff(pMessage[5], instanceID);
	break;
    case 82:						// Set Reso
	LOGDBG("Set Resonanece for TG %i", instanceID);
        m_pSynthesizer->SetResonance(pMessage[5], instanceID);
        break;
    case 83:						// Reverb level
	LOGDBG("Set Reverb Level for TG %i", instanceID);
        m_pSynthesizer->SetReverbSend (pMessage[5], instanceID);
	break;
    case 84:						// Transpose
	LOGDBG("Set Transpose for TG %i", instanceID);
//        m_pSynthesizer->SetTranspose (pMessage[5], instanceID);
	break;
    case 85:						// Detune
	LOGDBG("Set detune for TG %i", instanceID);
	if (pMessage[5] == 0)
        {
        	// "0 to 127, with 0 being no celeste (detune) effect applied at all."
                m_pSynthesizer->SetMasterTune (0, instanceID);
        }
        else
        {
                m_pSynthesizer->SetMasterTune (maplong (pMessage[5], 1, 127, -99, 99), instanceID);
        }
	break;
    case 86:						// Panning
	LOGDBG("Set panning for TG %i", instanceID);
        m_pSynthesizer->SetPan(pMessage[5], instanceID);
	break;
    case 87:						// Note Limit Low 
	LOGDBG("Set Note Limit High mode for TG %i", instanceID);
	break;
    case 88:						// Note Limit High
	LOGDBG("Set Note Limit High mode for TG %i", instanceID);
	break;
    case 89:						// Compressor toggle
	LOGDBG("Set Compressor ");
        m_pSynthesizer->SetParameter (CMiniDexed::ParameterCompressorEnable, pMessage[5] );
	break;
    case 90:						// Reverb toggle
	LOGDBG("Set Reverb Enable");
        m_pSynthesizer->SetParameter (CMiniDexed::ParameterReverbEnable, pMessage[5] );
	break;
    case 91:						// Reverb Size
	LOGDBG("Set Reverb Size");
        m_pSynthesizer->SetParameter (CMiniDexed::ParameterReverbSize, pMessage[5] );
	break;
    case 92:						// Reverb Low Damp
	LOGDBG("Set Reverb Low Damp");
        m_pSynthesizer->SetParameter (CMiniDexed::ParameterReverbLowDamp, pMessage[5]);
	break;
    case 93:						// Reverb High Damp
	LOGDBG("Set Reverb High Damp");
        m_pSynthesizer->SetParameter (CMiniDexed::ParameterReverbHighDamp, pMessage[5] );
	break;
    case 94:						// Reverb Lowpass
	LOGDBG("Set Reverb Low pass");
        m_pSynthesizer->SetParameter (CMiniDexed::ParameterReverbLowPass, pMessage[5]);
	break;
    case 95:						// Reverb Diffusion
	LOGDBG("Set Reverb Diffusion");
        m_pSynthesizer->SetParameter (CMiniDexed::ParameterReverbDiffusion, pMessage[5] );
	break;
    case 96:						// Reverb Master Level
	LOGDBG("Set Reverb Master Level");
        m_pSynthesizer->SetParameter (CMiniDexed::ParameterReverbLevel, pMessage[5] );
	break;
    case 600:						// Config requestnTG
        LOGDBG("Config request received\n");
	SendSystemExclusiveConfig();
	break;
    case 601:
        LOGDBG("Get Bank Name request received\n");
	SendBankName(instanceID);
	break;
/* End of BeZo patches */
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
        m_pSynthesizer->setVoiceDataElement(pMessage[4] + ((pMessage[3] & 0x03) * 128), pMessage[5],instanceID);
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
        SendSystemExclusiveVoice(sysex_return-500, instanceID);
      }
      break;
  }
}

void CMIDIDevice::SendSystemExclusiveVoice(uint8_t nVoice, uint8_t nTG)
{
  uint8_t voicedump[163];

  // Get voice sysex dump from TG
  m_pSynthesizer->getSysExVoiceDump(voicedump, nTG);

  TDeviceMap::const_iterator Iterator;

  // send voice dump to all MIDI interfaces
  for(Iterator = s_DeviceMap.begin(); Iterator != s_DeviceMap.end(); ++Iterator)
  {
    Iterator->second->Send (voicedump, sizeof(voicedump)*sizeof(uint8_t));
    LOGDBG("Send SYSEX voice dump %u to \"%s\"",nVoice,Iterator->first.c_str());
  }
} 

void CMIDIDevice::SendSystemExclusiveConfig()
{
  uint8_t count = 0;
  uint8_t configdump[204];

  configdump[count++] = 0xF0;
  configdump[count++] = 0x43;
  configdump[count++] = 0x31;

  // FX Settings
  configdump[count++] = ((m_pSynthesizer->GetParameter(CMiniDexed::ParameterCompressorEnable) & 0x7F)<<1) |
			 m_pSynthesizer->GetParameter(CMiniDexed::ParameterReverbEnable) & 0x7F;
  configdump[count++] = m_pSynthesizer->GetParameter(CMiniDexed::ParameterReverbSize) & 0x7F;
  configdump[count++] = m_pSynthesizer->GetParameter(CMiniDexed::ParameterReverbHighDamp) & 0x7F;
  configdump[count++] = m_pSynthesizer->GetParameter(CMiniDexed::ParameterReverbLowDamp) & 0x7F;
  configdump[count++] = m_pSynthesizer->GetParameter(CMiniDexed::ParameterReverbLowPass) & 0x7F;
  configdump[count++] = m_pSynthesizer->GetParameter(CMiniDexed::ParameterReverbDiffusion) & 0x7F;
  configdump[count++] = m_pSynthesizer->GetParameter(CMiniDexed::ParameterReverbLevel) & 0x7F;
  configdump[count++] = m_pSynthesizer->getMasterVolume() & 0x7F;

  for ( uint8_t instance = 0 ; instance < CConfig::ToneGenerators; instance++)
  {
    configdump[count++] = m_pSynthesizer->GetTGParameter(CMiniDexed::TGParameterVoiceBank, instance) & 0x7F;
    configdump[count++] = m_pSynthesizer->GetTGParameter(CMiniDexed::TGParameterProgram, instance) & 0x7F;
    configdump[count++] = m_pSynthesizer->GetTGParameter(CMiniDexed::TGParameterMIDIChannel, instance) & 0x7F;
    configdump[count++] = m_pSynthesizer->GetTGParameter(CMiniDexed::TGParameterVolume, instance) & 0x7F;
    configdump[count++] = m_pSynthesizer->GetTGParameter(CMiniDexed::TGParameterPan, instance) & 0x7F;
    int16_t mastertune = m_pSynthesizer->GetTGParameter(CMiniDexed::TGParameterMasterTune, instance);
    configdump[count++] = (mastertune >> 9)&0x7f;
    configdump[count++] = (mastertune & 0x7f);
    configdump[count++] = m_pSynthesizer->GetTGParameter(CMiniDexed::TGParameterCutoff, instance) & 0x7F;
    configdump[count++] = m_pSynthesizer->GetTGParameter(CMiniDexed::TGParameterResonance, instance) & 0x7F;
    configdump[count++] = 0; // Note limit low
    configdump[count++] = 127; // Note limit high
    configdump[count++] = 0; // Note shift
    configdump[count++] = m_pSynthesizer->GetTGParameter(CMiniDexed::TGParameterReverbSend, instance) & 0x7F;
    configdump[count++] = m_pSynthesizer->GetTGParameter(CMiniDexed::TGParameterPitchBendRange, instance) & 0x7F;
    configdump[count++] = m_pSynthesizer->GetTGParameter(CMiniDexed::TGParameterPitchBendStep, instance) & 0x7F;
    configdump[count++] = m_pSynthesizer->GetTGParameter(CMiniDexed::TGParameterPortamentoMode, instance) & 0x7F;
    configdump[count++] = m_pSynthesizer->GetTGParameter(CMiniDexed::TGParameterPortamentoGlissando, instance) & 0x7F;
    configdump[count++] = m_pSynthesizer->GetTGParameter(CMiniDexed::TGParameterPortamentoTime, instance) & 0x7F;
    configdump[count++] = 0;
    configdump[count++] = 0;
    configdump[count++] = 0;
    configdump[count++] = 0;
    configdump[count++] = 0;
    configdump[count++] = 0;
  }
  configdump[count++] = 0xF7;

  TDeviceMap::const_iterator Iterator;

  // send voice dump to all MIDI interfaces
  for(Iterator = s_DeviceMap.begin(); Iterator != s_DeviceMap.end(); ++Iterator)
  {
    Iterator->second->Send (configdump, count);
    LOGDBG("Send SYSEX config dump to \"%s\"",Iterator->first.c_str());
  }
}

void CMIDIDevice::SendProgramChange(uint8_t pgm, uint8_t nTG)
{
  uint8_t PgmChange[2] = { (uint8_t)(0xC0|(nTG & 0x0F)), (uint8_t)(pgm & 0x7f) };

  TDeviceMap::const_iterator Iterator;
  // send voice dump to all MIDI interfaces
  for(Iterator = s_DeviceMap.begin(); Iterator != s_DeviceMap.end(); ++Iterator)
  {
    Iterator->second->Send (PgmChange, 2);
    LOGDBG("Send Program Change %i to \"%s\"",pgm&0x7f,Iterator->first.c_str());
  }
}

void CMIDIDevice::SendBankChange(uint8_t bank, uint8_t nTG)
{
  SendCtrlChange14Bit(0, bank, nTG);
}

void CMIDIDevice::SendCtrlChange(uint8_t ctrl, uint8_t val, uint8_t nTG)
{
  uint8_t CtrlMsg[3] = { (uint8_t)(0xB0|(nTG & 0x0F)), (uint8_t)(ctrl&0x7f), (uint8_t)(val&0x7f) };

  TDeviceMap::const_iterator Iterator;

  // send voice dump to all MIDI interfaces
  for(Iterator = s_DeviceMap.begin(); Iterator != s_DeviceMap.end(); ++Iterator)
  {
    Iterator->second->Send (CtrlMsg, 3);
    LOGDBG("Send Ctrl change %02X = %i to \"%s\"",ctrl&0x7f, val&0x7f,Iterator->first.c_str());
  }
}

void CMIDIDevice::SendCtrlChange14Bit(uint8_t ctrl, int16_t val, uint8_t nTG)
{
    uint8_t lsb = (val & 0x7f);
    uint8_t msb = (val >> 9)&0x7f;
    SendCtrlChange(ctrl,msb,nTG);
    SendCtrlChange(ctrl+32, lsb, nTG);
}

void CMIDIDevice::SendBankName(uint8_t nTG)
{
  char *bankname = (char*)calloc(32,sizeof(char));
  snprintf(bankname,sizeof(bankname), "%s", m_pSynthesizer->GetSysExFileLoader()->GetBankName(m_pSynthesizer->GetTGParameter(CMiniDexed::TGParameterVoiceBank,nTG)).c_str());
  uint8_t banksysex[40] =  { 0xF0, 0x43, (uint8_t)(0x50|nTG), 0,0,32 };
  memcpy(banksysex+6,bankname,32);
  banksysex[38] = 00;
  banksysex[39] = 0xF7;
  TDeviceMap::const_iterator Iterator;

  // send voice dump to all MIDI interfaces
  for(Iterator = s_DeviceMap.begin(); Iterator != s_DeviceMap.end(); ++Iterator)
  {
    Iterator->second->Send (banksysex, sizeof(banksysex)*sizeof(uint8_t));
    LOGDBG("Send Bank Name Sysex to \"%s\"",Iterator->first.c_str());
  }
}

int16_t CMidiDevice::checkSystemExclusive(const uint8_t* sysex, const uint16_t len)
/*
        -1:     	SysEx end status byte not detected.
        -2:     	SysEx vendor not Yamaha.
        -3:     	Unknown SysEx parameter change.
        -4:     	Unknown SysEx voice or function.
        -5:     	Not a SysEx voice bulk upload.
        -6:     	Wrong length for SysEx voice bulk upload (not 155).
        -7:     	Checksum error for one voice.
        -8:     	Not a SysEx bank bulk upload.
        -9:     	Wrong length for SysEx bank bulk upload (not 4096).
        -10:    	Checksum error for bank.
        -11:    	Unknown SysEx message.
	64-77:		Function parameter changed.
	100:		Voice loaded.
	200:		Bank loaded.
	300-455:	Voice parameter changed.
	500-531:	Send patch request.
	600		Send config request
        601		Send Bank Name
*/
{
  int32_t bulk_checksum_calc = 0;
  const int8_t bulk_checksum = sysex[161];

  // Check for SYSEX end byte
  if (sysex[len - 1] != 0xf7)
    return(-1);

  // check for Yamaha sysex
  if (sysex[1] != 0x43)
    return(-2);

  // Decode SYSEX by means of length
  switch (len)
  {
    case 4:
      if ((sysex[2] & 0x30) == 0x30) // Send config request
        return 600;
      if ((sysex[2] & 0x70) == 0x40) // Send config request
        return 601;
      break;
    case 5:
      if ((sysex[2] & 0x70) == 0x20) // Send voice request
        return(500 + (sysex[3] & 0x7c));
      break;
    case 7: // parse parameter change
      if (((sysex[3] & 0x7c) >> 2) != 0 && ((sysex[3] & 0x7c) >> 2) != 2)
        return(-3);

      if ((sysex[3] & 0x7c) >> 2 == 0) // Voice parameter
      {
        setVoiceDataElement((sysex[4] & 0x7f) + ((sysex[3] & 0x03) * 128), sysex[5]);
	doRefreshVoice();
	return((sysex[4] & 0x7f) + ((sysex[3] & 0x03) * 128)+300);
      }
      else if ((sysex[3] & 0x7c) >> 2 == 2) // Function parameter
        return(sysex[4]);
      else
	return(-4);
      break;
    case 163: // 1 Voice bulk upload
      if ((sysex[3] & 0x7f) != 0)
        return(-5);

      if (((sysex[4] << 7) | sysex[5]) != 0x9b)
        return(-6);

      // checksum calculation
      for (uint8_t i = 0; i < 155 ; i++)
        bulk_checksum_calc -= sysex[i + 6];
      bulk_checksum_calc &= 0x7f;

      if (bulk_checksum_calc != bulk_checksum)
        return(-7);

      return(100);
      break;
    case 4104: // 1 Bank bulk upload
      if ((sysex[3] & 0x7f) != 9)
        return(-8);

      if (((sysex[4] << 7) | sysex[5]) != 0x1000)
        return(-9);

      // checksum calculation
      for (uint16_t i = 0; i < 4096 ; i++)
        bulk_checksum_calc -= sysex[i + 6];
      bulk_checksum_calc &= 0x7f;

      if (bulk_checksum_calc != bulk_checksum)
        return(-10);

      return(200);
      break;
    default:
      return(-11);
  }
  return(SHRT_MIN);
}
