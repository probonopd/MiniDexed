//
// minidexed.cpp
//
// MiniDexed - Dexed FM synthesizer for bare metal Raspberry Pi
// Copyright (C) 2022  The MiniDexed Team
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
#include "minidexed.h"
#include <circle/logger.h>
#include <circle/memory.h>
#include <circle/sound/pwmsoundbasedevice.h>
#include <circle/sound/i2ssoundbasedevice.h>
#include <circle/sound/hdmisoundbasedevice.h>
#include <circle/gpiopin.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <chrono>

LOGMODULE ("minidexed");

CMiniDexed::CMiniDexed (CConfig *pConfig, CInterruptSystem *pInterrupt,
			CGPIOManager *pGPIOManager, CI2CMaster *pI2CMaster, CSPIMaster *pSPIMaster, FATFS *pFileSystem)
:
#ifdef ARM_ALLOW_MULTI_CORE
	CMultiCoreSupport (CMemorySystem::Get ()),
#endif
	m_pConfig (pConfig),
	m_UI (this, pGPIOManager, pI2CMaster, pSPIMaster, pConfig),
	m_PerformanceConfig (pFileSystem),
	m_PCKeyboard (this, pConfig, &m_UI),
	m_SerialMIDI (this, pInterrupt, pConfig, &m_UI),
	m_bUseSerial (false),
	m_pSoundDevice (0),
	m_bChannelsSwapped (pConfig->GetChannelsSwapped ()),
#ifdef ARM_ALLOW_MULTI_CORE
	m_nActiveTGsLog2 (0),
#endif
	m_GetChunkTimer ("GetChunk",
			 1000000U * pConfig->GetChunkSize ()/2 / pConfig->GetSampleRate ()),
	m_bProfileEnabled (m_pConfig->GetProfileEnabled ()),
	m_bSavePerformance (false),
	m_bSavePerformanceNewFile (false),
	m_bSetNewPerformance (false),
	m_bSetNewPerformanceBank (false),
	m_bSetFirstPerformance (false),
	m_bDeletePerformance (false),
	m_bLoadPerformanceBusy(false),
	m_bLoadPerformanceBankBusy(false)
{
	assert (m_pConfig);

	m_nClockCounter = 0;
	m_mClockTime = 0;
	m_nTempo = 120;

	for (unsigned i = 0; i < CConfig::ToneGenerators; i++)
	{
		m_nVoiceBankID[i] = 0;
		m_nVoiceBankIDMSB[i] = 0;
		m_nProgram[i] = 0;
		m_nVolume[i] = 100;
		m_nPan[i] = 64;
		m_nMasterTune[i] = 0;
		m_nCutoff[i] = 99;
		m_nResonance[i] = 0;
		m_nMIDIChannel[i] = CMIDIDevice::Disabled;
		m_nPitchBendRange[i] = 2;
		m_nPitchBendStep[i] = 0;
		m_nPortamentoMode[i] = 0;
		m_nPortamentoGlissando[i] = 0;
		m_nPortamentoTime[i] = 0;
		m_bMonoMode[i]=0; 
		m_nNoteLimitLow[i] = 0;
		m_nNoteLimitHigh[i] = 127;
		m_nNoteShift[i] = 0;
		
		m_nModulationWheelRange[i]=99;
		m_nModulationWheelTarget[i]=7;
		m_nFootControlRange[i]=99;
		m_nFootControlTarget[i]=0;	
		m_nBreathControlRange[i]=99;	
		m_nBreathControlTarget[i]=0;	
		m_nAftertouchRange[i]=99;	
		m_nAftertouchTarget[i]=0;

#ifdef ARM_ALLOW_MULTI_CORE
		memset(m_OutputLevel[i][0], 0, CConfig::MaxChunkSize * sizeof(float32_t));
		memset(m_OutputLevel[i][1], 0, CConfig::MaxChunkSize * sizeof(float32_t));
#endif
		
		m_InsertFXSpinLock[i] = new CSpinLock();
		m_InsertFX[i] = new AudioEffectNone(pConfig->GetSampleRate ());
		m_nReverbSend[i] = 0;
		m_uchOPMask[i] = 0b111111;	// All operators on

		m_pTG[i] = new CDexedAdapter (CConfig::MaxNotes, pConfig->GetSampleRate ());
		assert (m_pTG[i]);
		m_MidiArpSpinLock[i] = new CSpinLock();
		m_MidiArp[i] = new MidiEffect(pConfig->GetSampleRate(), m_pTG[i]);

		m_pTG[i]->setEngineType(pConfig->GetEngineType ());
		m_pTG[i]->activate ();
	}
		
	if (pConfig->GetUSBGadgetMode())
	{
#if RASPPI==5
		LOGNOTE ("USB Gadget (Device) Mode NOT supported on RPI 5");
#else
		LOGNOTE ("USB In Gadget (Device) Mode");
#endif
	}
	else
	{
		LOGNOTE ("USB In Host Mode");
	}

	for (unsigned i = 0; i < CConfig::MaxUSBMIDIDevices; i++)
	{
		m_pMIDIKeyboard[i] = new CMIDIKeyboard (this, pConfig, &m_UI, i);
		assert (m_pMIDIKeyboard[i]);
	}

	// select the sound device
	const char *pDeviceName = pConfig->GetSoundDevice ();
	if (strcmp (pDeviceName, "i2s") == 0)
	{
		LOGNOTE ("I2S mode");

		m_pSoundDevice = new CI2SSoundBaseDevice (pInterrupt, pConfig->GetSampleRate (),
							  pConfig->GetChunkSize (), false,
							  pI2CMaster, pConfig->GetDACI2CAddress ());
	}
	else if (strcmp (pDeviceName, "hdmi") == 0)
	{
#if RASPPI==5
		LOGNOTE ("HDMI mode NOT supported on RPI 5.");
#else
		LOGNOTE ("HDMI mode");

		m_pSoundDevice = new CHDMISoundBaseDevice (pInterrupt, pConfig->GetSampleRate (),
							   pConfig->GetChunkSize ());

		// The channels are swapped by default in the HDMI sound driver.
		// TODO: Remove this line, when this has been fixed in the driver.
		m_bChannelsSwapped = !m_bChannelsSwapped;
#endif
	}
	else
	{
		LOGNOTE ("PWM mode");

		m_pSoundDevice = new CPWMSoundBaseDevice (pInterrupt, pConfig->GetSampleRate (),
							  pConfig->GetChunkSize ());
	}

#ifdef ARM_ALLOW_MULTI_CORE
	for (unsigned nCore = 0; nCore < CORES; nCore++)
	{
		m_CoreStatus[nCore] = CoreStatusInit;
	}
#endif

	setMasterVolume(1.0);

	// BEGIN setup tg_mixer
	tg_mixer = new AudioStereoMixer<CConfig::ToneGenerators>(pConfig->GetChunkSize()/2);
	// END setup tgmixer

	// BEGIN setup reverb
	SetParameter (ParameterSendFXType, EFFECT_REVERB);
	reverb_send_mixer = new AudioStereoMixer<CConfig::ToneGenerators>(pConfig->GetChunkSize()/2);
	reverb = new AudioEffectPlateReverb(pConfig->GetSampleRate());
	SetParameter (ParameterReverbEnable, 1);
	SetParameter (ParameterReverbSize, 70);
	SetParameter (ParameterReverbHighDamp, 50);
	SetParameter (ParameterReverbLowDamp, 50);
	SetParameter (ParameterReverbLowPass, 30);
	SetParameter (ParameterReverbDiffusion, 65);
	SetParameter (ParameterReverbLevel, 99);
	// END setup reverb

	SetParameter (ParameterCompressorEnable, 1);

	SetPerformanceSelectChannel(m_pConfig->GetPerformanceSelectChannel());
		
	SetParameter (ParameterPerformanceBank, 0);
};

bool CMiniDexed::Initialize (void)
{
	assert (m_pConfig);
	assert (m_pSoundDevice);

	if (!m_UI.Initialize ())
	{
		return false;
	}

	m_SysExFileLoader.Load (m_pConfig->GetHeaderlessSysExVoices ());

	if (m_SerialMIDI.Initialize ())
	{
		LOGNOTE ("Serial MIDI interface enabled");

		m_bUseSerial = true;
	}
	
	if (m_pConfig->GetMIDIRXProgramChange())
	{
		int nPerfCh = GetParameter(ParameterPerformanceSelectChannel);
		if (nPerfCh == CMIDIDevice::Disabled) {
			LOGNOTE("Program Change: Enabled for Voices");
		} else if (nPerfCh == CMIDIDevice::OmniMode) {
			LOGNOTE("Program Change: Enabled for Performances (Omni)");
		} else {
			LOGNOTE("Program Change: Enabled for Performances (CH %d)", nPerfCh+1);
		}
	} else {
		LOGNOTE("Program Change: Disabled");
	}

	for (unsigned i = 0; i < CConfig::ToneGenerators; i++)
	{
		assert (m_pTG[i]);

		SetVolume (100, i);
		ProgramChange (0, i);

		m_pTG[i]->setTranspose (24);

		m_pTG[i]->setPBController (2, 0);
		m_pTG[i]->setMWController (99, 1, 0); 

		m_pTG[i]->setFCController (99, 1, 0); 
		m_pTG[i]->setBCController (99, 1, 0);
		m_pTG[i]->setATController (99, 1, 0);
		
		tg_mixer->pan(i,mapfloat(m_nPan[i],0,127,0.0f,1.0f));
		tg_mixer->gain(i,1.0f);
		reverb_send_mixer->pan(i,mapfloat(m_nPan[i],0,127,0.0f,1.0f));
		reverb_send_mixer->gain(i,mapfloat(m_nReverbSend[i],0,99,0.0f,1.0f));
	}

	m_PerformanceConfig.Init();
	if (m_PerformanceConfig.Load ())
	{
		LoadPerformanceParameters(); 
	}
	else
	{
		SetMIDIChannel (CMIDIDevice::OmniMode, 0);
	}
	
	// setup and start the sound device
	if (!m_pSoundDevice->AllocateQueueFrames (m_pConfig->GetChunkSize ()))
	{
		LOGERR ("Cannot allocate sound queue");

		return false;
	}

#ifndef ARM_ALLOW_MULTI_CORE
	m_pSoundDevice->SetWriteFormat (SoundFormatSigned16, 1);	// 16-bit Mono
#else
	m_pSoundDevice->SetWriteFormat (SoundFormatSigned16, 2);	// 16-bit Stereo
#endif

	m_nQueueSizeFrames = m_pSoundDevice->GetQueueSizeFrames ();

	m_pSoundDevice->Start ();

#ifdef ARM_ALLOW_MULTI_CORE
	// start secondary cores
	if (!CMultiCoreSupport::Initialize ())
	{
		return false;
	}
#endif
	
	return true;
}

void CMiniDexed::Process (bool bPlugAndPlayUpdated)
{
#ifndef ARM_ALLOW_MULTI_CORE
	ProcessSound ();
#endif

	for (unsigned i = 0; i < CConfig::MaxUSBMIDIDevices; i++)
	{
		assert (m_pMIDIKeyboard[i]);
		m_pMIDIKeyboard[i]->Process (bPlugAndPlayUpdated);
	}

	m_PCKeyboard.Process (bPlugAndPlayUpdated);

	if (m_bUseSerial)
	{
		m_SerialMIDI.Process ();
	}

	m_UI.Process ();

	if (m_bSavePerformance)
	{
		DoSavePerformance ();

		m_bSavePerformance = false;
	}

	if (m_bSavePerformanceNewFile)
	{
		DoSavePerformanceNewFile ();
		m_bSavePerformanceNewFile = false;
	}
	
	if (m_bSetNewPerformanceBank && !m_bLoadPerformanceBusy && !m_bLoadPerformanceBankBusy)
	{
		DoSetNewPerformanceBank ();
		if (m_nSetNewPerformanceBankID == GetActualPerformanceBankID())
		{
			m_bSetNewPerformanceBank = false;
		}
		
		// If there is no pending SetNewPerformance already, then see if we need to find the first performance to load
		// NB: If called from the UI, then there will not be a SetNewPerformance, so load the first existing one.
		//     If called from MIDI, there will probably be a SetNewPerformance alongside the Bank select.
		if (!m_bSetNewPerformance && m_bSetFirstPerformance)
		{
			DoSetFirstPerformance();
		}
	}
	
	if (m_bSetNewPerformance && !m_bSetNewPerformanceBank && !m_bLoadPerformanceBusy && !m_bLoadPerformanceBankBusy)
	{
		DoSetNewPerformance ();
		if (m_nSetNewPerformanceID == GetActualPerformanceID())
		{
			m_bSetNewPerformance = false;
		}
	}
	
	if(m_bDeletePerformance)
	{
		DoDeletePerformance ();
		m_bDeletePerformance = false;
	}
		
	if (m_bProfileEnabled)
	{
		m_GetChunkTimer.Dump ();
	}
}

#ifdef ARM_ALLOW_MULTI_CORE

void CMiniDexed::Run (unsigned nCore)
{
	assert (1 <= nCore && nCore < CORES);

	if (nCore == 1)
	{
		m_CoreStatus[nCore] = CoreStatusIdle;			// core 1 ready

		// wait for cores 2 and 3 to be ready
		for (unsigned nCore = 2; nCore < CORES; nCore++)
		{
			while (m_CoreStatus[nCore] != CoreStatusIdle)
			{
				// just wait
			}
		}

		while (m_CoreStatus[nCore] != CoreStatusExit)
		{
			ProcessSound ();
		}
	}
	else								// core 2 and 3
	{
		while (1)
		{
			m_CoreStatus[nCore] = CoreStatusIdle;		// ready to be kicked
			while (m_CoreStatus[nCore] == CoreStatusIdle)
			{
				// just wait
			}

			// now kicked from core 1

			if (m_CoreStatus[nCore] == CoreStatusExit)
			{
				m_CoreStatus[nCore] = CoreStatusUnknown;

				break;
			}

			assert (m_CoreStatus[nCore] == CoreStatusBusy);

			// process the TGs, assigned to this core (2 or 3)

			assert (m_nFramesToProcess <= CConfig::MaxChunkSize);
			unsigned nTG = CConfig::TGsCore1 + (nCore-2)*CConfig::TGsCore23;
			for (unsigned i = 0; i < CConfig::TGsCore23; i++, nTG++)
			{
				assert (m_pTG[nTG]);

				m_MidiArpSpinLock[nTG]->Acquire();
				m_MidiArp[nTG]->process(m_nFramesToProcess);
				m_MidiArpSpinLock[nTG]->Release();
				
				m_pTG[nTG]->getSamples (m_OutputLevel[nTG][0],m_nFramesToProcess);
				
				m_InsertFXSpinLock[nTG]->Acquire();
				m_InsertFX[nTG]->process(m_OutputLevel[nTG][0], m_OutputLevel[nTG][0], m_OutputLevel[nTG][0], m_OutputLevel[nTG][1], m_nFramesToProcess);
				m_InsertFXSpinLock[nTG]->Release();
			}
		}
	}
}

#endif

CSysExFileLoader *CMiniDexed::GetSysExFileLoader (void)
{
	return &m_SysExFileLoader;
}

CPerformanceConfig *CMiniDexed::GetPerformanceConfig (void)
{
	return &m_PerformanceConfig;
}

void CMiniDexed::BankSelect (unsigned nBank, unsigned nTG)
{
	nBank=constrain((int)nBank,0,16383);

	assert (nTG < CConfig::ToneGenerators);
	
	if (GetSysExFileLoader ()->IsValidBank(nBank))
	{
		// Only change if we have the bank loaded
		m_nVoiceBankID[nTG] = nBank;

		m_UI.ParameterChanged ();
	}
}

void CMiniDexed::BankSelectPerformance (unsigned nBank)
{
	nBank=constrain((int)nBank,0,16383);

	if (GetPerformanceConfig ()->IsValidPerformanceBank(nBank))
	{
		// Only change if we have the bank loaded
		m_nVoiceBankIDPerformance = nBank;
		SetNewPerformanceBank (nBank);

		m_UI.ParameterChanged ();
	}
}

void CMiniDexed::BankSelectMSB (unsigned nBankMSB, unsigned nTG)
{
	nBankMSB=constrain((int)nBankMSB,0,127);

	assert (nTG < CConfig::ToneGenerators);
	// MIDI Spec 1.0 "BANK SELECT" states:
	//   "The transmitter must transmit the MSB and LSB as a pair,
	//   and the Program Change must be sent immediately after
	//   the Bank Select pair."
	//
	// So it isn't possible to validate the selected bank ID until
	// we receive both MSB and LSB so just store the MSB for now.
	m_nVoiceBankIDMSB[nTG] = nBankMSB;
}

void CMiniDexed::BankSelectMSBPerformance (unsigned nBankMSB)
{
	nBankMSB=constrain((int)nBankMSB,0,127);
	m_nVoiceBankIDMSBPerformance = nBankMSB;
}

void CMiniDexed::BankSelectLSB (unsigned nBankLSB, unsigned nTG)
{
	nBankLSB=constrain((int)nBankLSB,0,127);

	assert (nTG < CConfig::ToneGenerators);
	unsigned nBank = m_nVoiceBankID[nTG];
	unsigned nBankMSB = m_nVoiceBankIDMSB[nTG];
	nBank = (nBankMSB << 7) + nBankLSB;

	// Now should have both MSB and LSB so enable the BankSelect
	BankSelect(nBank, nTG);
}

void CMiniDexed::BankSelectLSBPerformance (unsigned nBankLSB)
{
	nBankLSB=constrain((int)nBankLSB,0,127);

	unsigned nBank = m_nVoiceBankIDPerformance;
	unsigned nBankMSB = m_nVoiceBankIDMSBPerformance;
	nBank = (nBankMSB << 7) + nBankLSB;

	// Now should have both MSB and LSB so enable the BankSelect
	BankSelectPerformance(nBank);
}

void CMiniDexed::ProgramChange (unsigned nProgram, unsigned nTG)
{
	assert (m_pConfig);

	unsigned nBankOffset;
	bool bPCAcrossBanks = m_pConfig->GetExpandPCAcrossBanks();
	if (bPCAcrossBanks)
	{
		// Note: This doesn't actually change the bank in use
		//       but will allow PC messages of 0..127
		//       to select across four consecutive banks of voices.
		//
		//   So if the current bank = 5 then:
		//       PC  0-31  = Bank 5, Program 0-31
		//       PC 32-63  = Bank 6, Program 0-31
		//       PC 64-95  = Bank 7, Program 0-31
		//       PC 96-127 = Bank 8, Program 0-31
		nProgram=constrain((int)nProgram,0,127);
		nBankOffset = nProgram >> 5;
		nProgram = nProgram % 32;
	}
	else
	{
		nBankOffset = 0;
		nProgram=constrain((int)nProgram,0,31);
	}

	assert (nTG < CConfig::ToneGenerators);
	m_nProgram[nTG] = nProgram;

	uint8_t Buffer[156];
	m_SysExFileLoader.GetVoice (m_nVoiceBankID[nTG]+nBankOffset, nProgram, Buffer);

	assert (m_pTG[nTG]);
	m_pTG[nTG]->loadVoiceParameters (Buffer);

	if (m_pConfig->GetMIDIAutoVoiceDumpOnPC())
	{
		// Only do the voice dump back out over MIDI if we have a specific
		// MIDI channel configured for this TG
		if (m_nMIDIChannel[nTG] < CMIDIDevice::Channels)
		{
			m_SerialMIDI.SendSystemExclusiveVoice(nProgram,0,nTG);
		}
	}

	m_UI.ParameterChanged ();
}

void CMiniDexed::ProgramChangePerformance (unsigned nProgram)
{
	if (m_nParameter[ParameterPerformanceSelectChannel] != CMIDIDevice::Disabled)
	{
		// Program Change messages change Performances.
		if (m_PerformanceConfig.IsValidPerformance(nProgram))
		{
			SetNewPerformance(nProgram);
		}
		m_UI.ParameterChanged ();
	}
}

void CMiniDexed::SetVolume (unsigned nVolume, unsigned nTG)
{
	nVolume=constrain((int)nVolume,0,127);

	assert (nTG < CConfig::ToneGenerators);
	m_nVolume[nTG] = nVolume;

	assert (m_pTG[nTG]);
	m_pTG[nTG]->setGain (nVolume / 127.0f);

	m_UI.ParameterChanged ();
}

void CMiniDexed::SetPan (unsigned nPan, unsigned nTG)
{
	nPan=constrain((int)nPan,0,127);

	assert (nTG < CConfig::ToneGenerators);
	m_nPan[nTG] = nPan;
	
	tg_mixer->pan(nTG,mapfloat(nPan,0,127,0.0f,1.0f));
	reverb_send_mixer->pan(nTG,mapfloat(nPan,0,127,0.0f,1.0f));

	m_UI.ParameterChanged ();
}

void CMiniDexed::setInsertFXType (unsigned nType, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);

	// If the effect type is already set just return
	if (m_InsertFX[nTG]->getId() == nType) {
		return;
	}

	m_InsertFXSpinLock[nTG]->Acquire();
	delete m_InsertFX[nTG];
	m_InsertFX[nTG] = newAudioEffect(nType, m_pConfig->GetSampleRate());
	m_InsertFX[nTG]->setTempo(m_nTempo);
	m_InsertFXSpinLock[nTG]->Release();

	m_UI.ParameterChanged ();
}

void CMiniDexed::setMidiFXType (unsigned nType, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);

	// If the effect type is already set just return
	if (m_MidiArp[nTG]->getId() == nType) {
		return;
	}

	m_MidiArpSpinLock[nTG]->Acquire();
	delete m_MidiArp[nTG];
	m_MidiArp[nTG] = newMidiEffect(nType, m_pConfig->GetSampleRate(), m_pTG[nTG]);
	m_MidiArp[nTG]->setTempo(m_nTempo);
	m_MidiArpSpinLock[nTG]->Release();

	m_UI.ParameterChanged ();
}

void CMiniDexed::setSendFXType (unsigned nType)
{
	m_SendFXSpinLock.Acquire();
	if (m_SendFX != NULL)
	{
		delete m_SendFX;
	}
	m_SendFX = newAudioEffect(nType, m_pConfig->GetSampleRate());
	m_SendFX->setTempo(m_nTempo);
	m_SendFX->initializeSendFX();
	m_SendFXSpinLock.Release();

	m_UI.ParameterChanged ();
}

void CMiniDexed::setSendFXLevel (unsigned nValue)
{
	nValue = constrain((int)nValue, 0, 100);
	m_SendFXLevel = (float32_t) nValue / 100.0f;	
}

void CMiniDexed::SetReverbSend (unsigned nReverbSend, unsigned nTG)
{
	nReverbSend=constrain((int)nReverbSend,0,99);

	assert (nTG < CConfig::ToneGenerators);
	m_nReverbSend[nTG] = nReverbSend;

	reverb_send_mixer->gain(nTG,mapfloat(nReverbSend,0,99,0.0f,1.0f));
	
	m_UI.ParameterChanged ();
}

void CMiniDexed::SetMasterTune (int nMasterTune, unsigned nTG)
{
	nMasterTune=constrain((int)nMasterTune,-99,99);

	assert (nTG < CConfig::ToneGenerators);
	m_nMasterTune[nTG] = nMasterTune;

	assert (m_pTG[nTG]);
	m_pTG[nTG]->setMasterTune ((int8_t) nMasterTune);

	m_UI.ParameterChanged ();
}

void CMiniDexed::SetCutoff (int nCutoff, unsigned nTG)
{
	nCutoff = constrain (nCutoff, 0, 99);

	assert (nTG < CConfig::ToneGenerators);
	m_nCutoff[nTG] = nCutoff;

	assert (m_pTG[nTG]);
	m_pTG[nTG]->setFilterCutoff (mapfloat (nCutoff, 0, 99, 0.0f, 1.0f));

	m_UI.ParameterChanged ();
}

void CMiniDexed::SetResonance (int nResonance, unsigned nTG)
{
	nResonance = constrain (nResonance, 0, 99);

	assert (nTG < CConfig::ToneGenerators);
	m_nResonance[nTG] = nResonance;

	assert (m_pTG[nTG]);
	m_pTG[nTG]->setFilterResonance (mapfloat (nResonance, 0, 99, 0.0f, 1.0f));

	m_UI.ParameterChanged ();
}



void CMiniDexed::SetMIDIChannel (uint8_t uchChannel, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (uchChannel < CMIDIDevice::ChannelUnknown);

	m_nMIDIChannel[nTG] = uchChannel;

	for (unsigned i = 0; i < CConfig::MaxUSBMIDIDevices; i++)
	{
		assert (m_pMIDIKeyboard[i]);
		m_pMIDIKeyboard[i]->SetChannel (uchChannel, nTG);
	}

	m_PCKeyboard.SetChannel (uchChannel, nTG);

	if (m_bUseSerial)
	{
		m_SerialMIDI.SetChannel (uchChannel, nTG);
	}

#ifdef ARM_ALLOW_MULTI_CORE
	unsigned nActiveTGs = 0;
	for (unsigned nTG = 0; nTG < CConfig::ToneGenerators; nTG++)
	{
		if (m_nMIDIChannel[nTG] != CMIDIDevice::Disabled)
		{
			nActiveTGs++;
		}
	}

	assert (nActiveTGs <= 8);
	static const unsigned Log2[] = {0, 0, 1, 2, 2, 3, 3, 3, 3};
	m_nActiveTGsLog2 = Log2[nActiveTGs];
#endif

	m_UI.ParameterChanged ();
}

unsigned CMiniDexed::getTempo (void)
{
	return this->m_nTempo;
}

void CMiniDexed::setTempo(unsigned nValue)
{
	m_nTempo = nValue;

	// Set Tempo to FXs
	m_SendFX->setTempo(m_nTempo);
	for (unsigned nTG = 0; nTG < CConfig::ToneGenerators; nTG++)
	{
		m_InsertFX[nTG]->setTempo(m_nTempo);
		m_MidiArp[nTG]->setTempo(m_nTempo);
	}

	// Update UI
	m_UI.ParameterChanged();
}


void CMiniDexed::handleClock (void)
{
	if (m_nClockCounter == 0)
	{
		// Set milis
		auto now = std::chrono::high_resolution_clock::now();
		m_mClockTime = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

	}
	m_nClockCounter++;

	if (m_nClockCounter > 23) {
		m_nClockCounter = 0;
		// Calculate BPM
		auto now = std::chrono::high_resolution_clock::now();
		unsigned timeDelta = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() - m_mClockTime;
		unsigned newTempo = roundf(60000 / timeDelta);
		if (m_nTempo != newTempo)
		{
			this->setTempo(newTempo);
		}
	}
}

void CMiniDexed::keyup (int16_t pitch, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);

	pitch = ApplyNoteLimits (pitch, nTG);
	if (pitch >= 0)
	{
		if (m_MidiArp[nTG]->getBypass())
		{
			m_pTG[nTG]->keyup(pitch);
		}
		else
		{
			m_MidiArpSpinLock[nTG]->Acquire();
			m_MidiArp[nTG]->keyup(pitch);
			m_MidiArpSpinLock[nTG]->Release();
		}
	}
}

void CMiniDexed::keydown (int16_t pitch, uint8_t velocity, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);

	pitch = ApplyNoteLimits (pitch, nTG);
	if (pitch >= 0)
	{
		if (m_MidiArp[nTG]->getBypass())
		{
			m_pTG[nTG]->keydown (pitch, velocity);
		}
		else
		{
			m_MidiArpSpinLock[nTG]->Acquire();
			m_MidiArp[nTG]->keydown(pitch, velocity);
			m_MidiArpSpinLock[nTG]->Release();
		}
	}
}

int16_t CMiniDexed::ApplyNoteLimits (int16_t pitch, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);

	if (   pitch < (int16_t) m_nNoteLimitLow[nTG]
	    || pitch > (int16_t) m_nNoteLimitHigh[nTG])
	{
		return -1;
	}

	pitch += m_nNoteShift[nTG];

	if (   pitch < 0
	    || pitch > 127)
	{
		return -1;
	}

	return pitch;
}

void CMiniDexed::setSustain(bool sustain, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);
	m_pTG[nTG]->setSustain (sustain);
}

void CMiniDexed::panic(uint8_t value, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);
	if (value == 0) {
		m_pTG[nTG]->panic ();
	}
}

void CMiniDexed::notesOff(uint8_t value, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);
	if (value == 0) {
		m_pTG[nTG]->notesOff ();
	}
}

void CMiniDexed::setModWheel (uint8_t value, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);
	m_pTG[nTG]->setModWheel (value);
}


void CMiniDexed::setFootController (uint8_t value, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);
	m_pTG[nTG]->setFootController (value);
}

void CMiniDexed::setBreathController (uint8_t value, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);
	m_pTG[nTG]->setBreathController (value);
}

void CMiniDexed::setAftertouch (uint8_t value, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);
	m_pTG[nTG]->setAftertouch (value);
}

void CMiniDexed::setPitchbend (int16_t value, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);
	m_pTG[nTG]->setPitchbend (value);
}

void CMiniDexed::ControllersRefresh (unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);
	m_pTG[nTG]->ControllersRefresh ();
}

void CMiniDexed::SetParameter (TParameter Parameter, int nValue)
{
	assert (reverb);

	assert (Parameter < ParameterUnknown);
	m_nParameter[Parameter] = nValue;

	switch (Parameter)
	{
	case ParameterCompressorEnable:
		for (unsigned nTG = 0; nTG < CConfig::ToneGenerators; nTG++)
		{
			assert (m_pTG[nTG]);
			m_pTG[nTG]->setCompressor (!!nValue);
		}
		break;

	case ParameterSendFXType:
		setSendFXType(nValue);
		break;
	
	case ParameterSendFXLevel:
		setSendFXLevel(nValue);
		break;

	case ParameterReverbEnable:
		nValue=constrain((int)nValue,0,1);
		m_ReverbSpinLock.Acquire ();
		reverb->setBypass (!nValue);
		m_ReverbSpinLock.Release ();
		break;

	case ParameterReverbSize:
		nValue=constrain((int)nValue,0,99);
		m_ReverbSpinLock.Acquire ();
		reverb->size (nValue / 99.0f);
		m_ReverbSpinLock.Release ();
		break;

	case ParameterReverbHighDamp:
		nValue=constrain((int)nValue,0,99);
		m_ReverbSpinLock.Acquire ();
		reverb->hidamp (nValue / 99.0f);
		m_ReverbSpinLock.Release ();
		break;

	case ParameterReverbLowDamp:
		nValue=constrain((int)nValue,0,99);
		m_ReverbSpinLock.Acquire ();
		reverb->lodamp (nValue / 99.0f);
		m_ReverbSpinLock.Release ();
		break;

	case ParameterReverbLowPass:
		nValue=constrain((int)nValue,0,99);
		m_ReverbSpinLock.Acquire ();
		reverb->lowpass (nValue / 99.0f);
		m_ReverbSpinLock.Release ();
		break;

	case ParameterReverbDiffusion:
		nValue=constrain((int)nValue,0,99);
		m_ReverbSpinLock.Acquire ();
		reverb->diffusion (nValue / 99.0f);
		m_ReverbSpinLock.Release ();
		break;

	case ParameterReverbLevel:
		nValue=constrain((int)nValue,0,99);
		m_ReverbSpinLock.Acquire ();
		reverb->level (nValue / 99.0f);
		m_ReverbSpinLock.Release ();
		break;

	case ParameterPerformanceSelectChannel:
		// Nothing more to do
		break;

	case ParameterPerformanceBank:
		BankSelectPerformance(nValue);
		break;

	case ParameterTempo:
		this->setTempo(nValue);
		break;

	default:
		assert (0);
		break;
	}
}

int CMiniDexed::GetParameter (TParameter Parameter)
{
	assert (Parameter < ParameterUnknown);

	switch (Parameter)
	{
	case ParameterSendFXType:
		return m_SendFX->getId();
	case ParameterSendFXLevel:
		return roundf(m_SendFXLevel * 100);
	case ParameterTempo:
		return this->getTempo();
	default:
		return m_nParameter[Parameter];
	}
}

void CMiniDexed::SetTGParameter (TTGParameter Parameter, int nValue, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);

	switch (Parameter)
	{
	case TGParameterVoiceBank:	BankSelect (nValue, nTG);	break;
	case TGParameterVoiceBankMSB:	BankSelectMSB (nValue, nTG);	break;
	case TGParameterVoiceBankLSB:	BankSelectLSB (nValue, nTG);	break;
	case TGParameterProgram:	ProgramChange (nValue, nTG);	break;
	case TGParameterVolume:		SetVolume (nValue, nTG);	break;
	case TGParameterPan:		SetPan (nValue, nTG);		break;
	case TGParameterMasterTune:	SetMasterTune (nValue, nTG);	break;
	case TGParameterCutoff:		SetCutoff (nValue, nTG);	break;
	case TGParameterResonance:	SetResonance (nValue, nTG);	break;
	case TGParameterPitchBendRange:	setPitchbendRange (nValue, nTG);	break;
	case TGParameterPitchBendStep:	setPitchbendStep (nValue, nTG);	break;
	case TGParameterPortamentoMode:		setPortamentoMode (nValue, nTG);	break;
	case TGParameterPortamentoGlissando:	setPortamentoGlissando (nValue, nTG);	break;
	case TGParameterPortamentoTime:		setPortamentoTime (nValue, nTG);	break;
	case TGParameterMonoMode:		setMonoMode (nValue , nTG);	break; 
	
	case TGParameterMWRange:					setModController(0, 0, nValue, nTG); break;
	case TGParameterMWPitch:					setModController(0, 1, nValue, nTG); break;
	case TGParameterMWAmplitude:				setModController(0, 2, nValue, nTG); break;
	case TGParameterMWEGBias:					setModController(0, 3, nValue, nTG); break;
	
	case TGParameterFCRange:					setModController(1, 0, nValue, nTG); break;
	case TGParameterFCPitch:					setModController(1, 1, nValue, nTG); break;
	case TGParameterFCAmplitude:				setModController(1, 2, nValue, nTG); break;
	case TGParameterFCEGBias:					setModController(1, 3, nValue, nTG); break;
	
	case TGParameterBCRange:					setModController(2, 0, nValue, nTG); break;
	case TGParameterBCPitch:					setModController(2, 1, nValue, nTG); break;
	case TGParameterBCAmplitude:				setModController(2, 2, nValue, nTG); break;
	case TGParameterBCEGBias:					setModController(2, 3, nValue, nTG); break;
	
	case TGParameterATRange:					setModController(3, 0, nValue, nTG); break;
	case TGParameterATPitch:					setModController(3, 1, nValue, nTG); break;
	case TGParameterATAmplitude:				setModController(3, 2, nValue, nTG); break;
	case TGParameterATEGBias:					setModController(3, 3, nValue, nTG); break;
	
	case TGParameterMIDIChannel:
		assert (0 <= nValue && nValue <= 255);
		SetMIDIChannel ((uint8_t) nValue, nTG);
		break;

	case TGParameterReverbSend:	SetReverbSend (nValue, nTG);	break;
	case TGParameterInsertFXType: setInsertFXType(nValue, nTG); break;
	case TGParameterMidiFXType: setMidiFXType(nValue, nTG); break;
	
	default:
		assert (0);
		break;
	}
}

int CMiniDexed::GetTGParameter (TTGParameter Parameter, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);

	switch (Parameter)
	{
	case TGParameterVoiceBank:	return m_nVoiceBankID[nTG];
	case TGParameterVoiceBankMSB:	return m_nVoiceBankID[nTG] >> 7;
	case TGParameterVoiceBankLSB:	return m_nVoiceBankID[nTG] & 0x7F;
	case TGParameterProgram:	return m_nProgram[nTG];
	case TGParameterVolume:		return m_nVolume[nTG];
	case TGParameterPan:		return m_nPan[nTG];
	case TGParameterMasterTune:	return m_nMasterTune[nTG];
	case TGParameterCutoff:		return m_nCutoff[nTG];
	case TGParameterResonance:	return m_nResonance[nTG];
	case TGParameterMIDIChannel:	return m_nMIDIChannel[nTG];
	case TGParameterReverbSend:	return m_nReverbSend[nTG];
	case TGParameterInsertFXType:	return m_InsertFX[nTG]->getId();
	case TGParameterMidiFXType:	return m_MidiArp[nTG]->getId();
	case TGParameterPitchBendRange:	return m_nPitchBendRange[nTG];
	case TGParameterPitchBendStep:	return m_nPitchBendStep[nTG];
	case TGParameterPortamentoMode:		return m_nPortamentoMode[nTG];
	case TGParameterPortamentoGlissando:	return m_nPortamentoGlissando[nTG];
	case TGParameterPortamentoTime:		return m_nPortamentoTime[nTG];
	case TGParameterMonoMode:		return m_bMonoMode[nTG] ? 1 : 0; 
	
	case TGParameterMWRange:					return getModController(0, 0, nTG);
	case TGParameterMWPitch:					return getModController(0, 1, nTG);
	case TGParameterMWAmplitude:				return getModController(0, 2, nTG); 
	case TGParameterMWEGBias:					return getModController(0, 3, nTG); 
	
	case TGParameterFCRange:					return getModController(1, 0,  nTG); 
	case TGParameterFCPitch:					return getModController(1, 1,  nTG); 
	case TGParameterFCAmplitude:				return getModController(1, 2,  nTG); 
	case TGParameterFCEGBias:					return getModController(1, 3,  nTG); 
	
	case TGParameterBCRange:					return getModController(2, 0,  nTG); 
	case TGParameterBCPitch:					return getModController(2, 1,  nTG); 
	case TGParameterBCAmplitude:				return getModController(2, 2,  nTG); 
	case TGParameterBCEGBias:					return getModController(2, 3,  nTG); 
	
	case TGParameterATRange:					return getModController(3, 0,  nTG); 
	case TGParameterATPitch:					return getModController(3, 1,  nTG); 
	case TGParameterATAmplitude:				return getModController(3, 2,  nTG); 
	case TGParameterATEGBias:					return getModController(3, 3,  nTG); 
	
	default:
		assert (0);
		return 0;
	}
}

void CMiniDexed::SetMidiFXParameter (unsigned Parameter, int nValue, unsigned nTG, unsigned nFXType) {
	assert (nTG < CConfig::ToneGenerators);
	assert (m_MidiArp[nTG]->getId() == nFXType);

	m_MidiArp[nTG]->setParameter(Parameter, nValue);
}

int CMiniDexed::GetMidiFXParameter (unsigned Parameter, unsigned nTG, unsigned nFXType) {
	assert (nTG < CConfig::ToneGenerators);
	assert (m_MidiArp[nTG]->getId() == nFXType);

	return m_MidiArp[nTG]->getParameter(Parameter);
}

void CMiniDexed::SetTGFXParameter (unsigned Parameter, int nValue, unsigned nTG, unsigned nFXType) {
	assert (nTG < CConfig::ToneGenerators);
	assert (m_InsertFX[nTG]->getId() == nFXType);

	m_InsertFX[nTG]->setParameter(Parameter, nValue);
}

int CMiniDexed::GetTGFXParameter (unsigned Parameter, unsigned nTG, unsigned nFXType) {
	assert (nTG < CConfig::ToneGenerators);
	assert (m_InsertFX[nTG]->getId() == nFXType);

	return m_InsertFX[nTG]->getParameter(Parameter);
}

void CMiniDexed::SetSendFXParameter (unsigned Parameter, int nValue, unsigned nFXType) {
	assert (m_SendFX->getId() == nFXType);

	m_SendFX->setParameter(Parameter, nValue);
}

int CMiniDexed::GetSendFXParameter (unsigned Parameter, unsigned nFXType) {
	assert (m_SendFX->getId() == nFXType);

	return m_SendFX->getParameter(Parameter);
}

void CMiniDexed::SetVoiceParameter (uint8_t uchOffset, uint8_t uchValue, unsigned nOP, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);
	assert (nOP <= 6);

	if (nOP < 6)
	{
		if (uchOffset == DEXED_OP_ENABLE)
		{
			if (uchValue)
			{
				m_uchOPMask[nTG] |= 1 << nOP;
			}
			else
			{
				m_uchOPMask[nTG] &= ~(1 << nOP);
			}

			m_pTG[nTG]->setOPAll (m_uchOPMask[nTG]);

			return;
		}

		nOP = 5 - nOP;		// OPs are in reverse order
	}

	uchOffset += nOP * 21;
	assert (uchOffset < 156);

	m_pTG[nTG]->setVoiceDataElement (uchOffset, uchValue);
}

uint8_t CMiniDexed::GetVoiceParameter (uint8_t uchOffset, unsigned nOP, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);
	assert (nOP <= 6);

	if (nOP < 6)
	{
		if (uchOffset == DEXED_OP_ENABLE)
		{
			return !!(m_uchOPMask[nTG] & (1 << nOP));
		}

		nOP = 5 - nOP;		// OPs are in reverse order
	}

	uchOffset += nOP * 21;
	assert (uchOffset < 156);

	return m_pTG[nTG]->getVoiceDataElement (uchOffset);
}

std::string CMiniDexed::GetVoiceName (unsigned nTG)
{
	char VoiceName[11];
	memset (VoiceName, 0, sizeof VoiceName);

	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);
	m_pTG[nTG]->setName (VoiceName);

	std::string Result (VoiceName);

	return Result;
}

#ifndef ARM_ALLOW_MULTI_CORE

void CMiniDexed::ProcessSound (void)
{
	assert (m_pSoundDevice);

	unsigned nFrames = m_nQueueSizeFrames - m_pSoundDevice->GetQueueFramesAvail ();
	if (nFrames >= m_nQueueSizeFrames/2)
	{
		if (m_bProfileEnabled)
		{
			m_GetChunkTimer.Start ();
		}

		float32_t SampleBuffer[2][nFrames];

		m_MidiArpSpinLock[0]->Acquire();
		m_MidiArp[0]->process(nFrames);
		m_MidiArpSpinLock[0]->Release();
		
		m_pTG[0]->getSamples (SampleBuffer[0], nFrames);
		
		m_InsertFXSpinLock[0]->Acquire();
		m_InsertFX[0]->process(SampleBuffer[0], SampleBuffer[0], SampleBuffer[0], SampleBuffer[1], nFrames);
		m_InsertFXSpinLock[0]->Release();

		// Convert dual float array (left, right) to single int16 array (left/right)
		float32_t tmp_float[nFrames*2];
		for(uint16_t i=0; i<nFrames;i++)
		{
			tmp_float[i*2]=SampleBuffer[0][i];
			tmp_float[(i*2)+1]=SampleBuffer[1][i];
		}
		// Convert single float array (mono) to int16 array
		int16_t tmp_int[nFrames];
		arm_float_to_q15(tmp_float,tmp_int,nFrames*2);

		if (m_pSoundDevice->Write (tmp_int, sizeof(tmp_int)) != (int) sizeof(tmp_int))
		{
			LOGERR ("Sound data dropped");
		}

		if (m_bProfileEnabled)
		{
			m_GetChunkTimer.Stop ();
		}
	}
}

#else	// #ifdef ARM_ALLOW_MULTI_CORE

void CMiniDexed::ProcessSound (void)
{
	assert (m_pSoundDevice);

	unsigned nFrames = m_nQueueSizeFrames - m_pSoundDevice->GetQueueFramesAvail ();
	if (nFrames >= m_nQueueSizeFrames/2)
	{
		if (m_bProfileEnabled)
		{
			m_GetChunkTimer.Start ();
		}

		m_nFramesToProcess = nFrames;

		// kick secondary cores
		for (unsigned nCore = 2; nCore < CORES; nCore++)
		{
			assert (m_CoreStatus[nCore] == CoreStatusIdle);
			m_CoreStatus[nCore] = CoreStatusBusy;
		}

		// process the TGs assigned to core 1
		assert (nFrames <= CConfig::MaxChunkSize);
		for (unsigned i = 0; i < CConfig::TGsCore1; i++)
		{
			assert (m_pTG[i]);
			
			m_MidiArpSpinLock[i]->Acquire();
			m_MidiArp[i]->process(nFrames);
			m_MidiArpSpinLock[i]->Release();

			m_pTG[i]->getSamples (m_OutputLevel[i][0], nFrames);
			
			m_InsertFXSpinLock[i]->Acquire();
			m_InsertFX[i]->process(m_OutputLevel[i][0], m_OutputLevel[i][0], m_OutputLevel[i][0], m_OutputLevel[i][1], nFrames);
			m_InsertFXSpinLock[i]->Release();
		}

		// wait for cores 2 and 3 to complete their work
		for (unsigned nCore = 2; nCore < CORES; nCore++)
		{
			while (m_CoreStatus[nCore] != CoreStatusIdle)
			{
				// just wait
			}
		}

		//
		// Audio signal path after tone generators starts here
		//

		assert (CConfig::ToneGenerators == 8);

		uint8_t indexL=0, indexR=1;
		
		// BEGIN TG mixing
		float32_t tmp_float[nFrames*2];
		int16_t tmp_int[nFrames*2];

		if(nMasterVolume > 0.0)
		{
			for (uint8_t i = 0; i < CConfig::ToneGenerators; i++)
			{
				tg_mixer->doAddMix(i, m_OutputLevel[i][0], m_OutputLevel[i][1]);
				reverb_send_mixer->doAddMix(i, m_OutputLevel[i][0], m_OutputLevel[i][1]);
			}
			// END TG mixing
	
			// BEGIN create SampleBuffer for holding audio data
			float32_t SampleBuffer[2][nFrames];
			// END create SampleBuffer for holding audio data

			// get the mix of all TGs
			tg_mixer->getMix(SampleBuffer[indexL], SampleBuffer[indexR]);

			// BEGIN adding reverb
			if (m_nParameter[ParameterReverbEnable])
			{
				float32_t ReverbBuffer[2][nFrames];
				float32_t ReverbSendBuffer[2][nFrames];

				arm_fill_f32(0.0f, ReverbBuffer[indexL], nFrames);
				arm_fill_f32(0.0f, ReverbBuffer[indexR], nFrames);
				arm_fill_f32(0.0f, ReverbSendBuffer[indexR], nFrames);
				arm_fill_f32(0.0f, ReverbSendBuffer[indexL], nFrames);
	
				m_SendFXSpinLock.Acquire ();
				m_ReverbSpinLock.Acquire ();
	
       		    reverb_send_mixer->getMix(ReverbSendBuffer[indexL], ReverbSendBuffer[indexR]);
				//reverb->doReverb(ReverbSendBuffer[indexL],ReverbSendBuffer[indexR],ReverbBuffer[indexL], ReverbBuffer[indexR],nFrames);
				m_SendFX->process(ReverbSendBuffer[indexL], ReverbSendBuffer[indexR], ReverbBuffer[indexL], ReverbBuffer[indexR], nFrames);
	
				// scale down and add left reverb buffer by reverb level 
				arm_scale_f32(ReverbBuffer[indexL], m_SendFXLevel, ReverbBuffer[indexL], nFrames);
				arm_add_f32(SampleBuffer[indexL], ReverbBuffer[indexL], SampleBuffer[indexL], nFrames);
				// scale down and add right reverb buffer by reverb level 
				arm_scale_f32(ReverbBuffer[indexR], m_SendFXLevel, ReverbBuffer[indexR], nFrames);
				arm_add_f32(SampleBuffer[indexR], ReverbBuffer[indexR], SampleBuffer[indexR], nFrames);
	
				m_ReverbSpinLock.Release ();
				m_SendFXSpinLock.Release ();
			}
			// END adding reverb
	
			// swap stereo channels if needed prior to writing back out
			if (m_bChannelsSwapped)
			{
				indexL=1;
				indexR=0;
			}

			// Convert dual float array (left, right) to single int16 array (left/right)
			for(uint16_t i=0; i<nFrames;i++)
			{
				if(nMasterVolume >0.0 && nMasterVolume <1.0)
				{
					tmp_float[i*2]=SampleBuffer[indexL][i] * nMasterVolume;
					tmp_float[(i*2)+1]=SampleBuffer[indexR][i] * nMasterVolume;
				}
				else if(nMasterVolume == 1.0)
				{
					tmp_float[i*2]=SampleBuffer[indexL][i];
					tmp_float[(i*2)+1]=SampleBuffer[indexR][i];
				}
			}
			arm_float_to_q15(tmp_float,tmp_int,nFrames*2);
		}
		else
			arm_fill_q15(0, tmp_int, nFrames * 2);

		if (m_pSoundDevice->Write (tmp_int, sizeof(tmp_int)) != (int) sizeof(tmp_int))
		{
			LOGERR ("Sound data dropped");
		}

		if (m_bProfileEnabled)
		{
			m_GetChunkTimer.Stop ();
		}
	}
}

#endif

unsigned CMiniDexed::GetPerformanceSelectChannel (void)
{
	// Stores and returns Select Channel using MIDI Device Channel definitions
	return (unsigned) GetParameter (ParameterPerformanceSelectChannel);
}

void CMiniDexed::SetPerformanceSelectChannel (unsigned uCh)
{
	// Turns a configuration setting to MIDI Device Channel definitions
	// Mirrors the logic in Performance Config for handling MIDI channel configuration
	if (uCh == 0)
	{
		SetParameter (ParameterPerformanceSelectChannel, CMIDIDevice::Disabled);
	}
	else if (uCh < CMIDIDevice::Channels)
	{
		SetParameter (ParameterPerformanceSelectChannel, uCh - 1);
	}
	else
	{
		SetParameter (ParameterPerformanceSelectChannel, CMIDIDevice::OmniMode);
	}
}

bool CMiniDexed::SavePerformance (bool bSaveAsDeault)
{
	if (m_PerformanceConfig.GetInternalFolderOk())
	{
		m_bSavePerformance = true;
		m_bSaveAsDeault=bSaveAsDeault;

		return true;
	}
	else
	{
		return false;
	}
}

bool CMiniDexed::DoSavePerformance (void)
{
	for (unsigned nTG = 0; nTG < CConfig::ToneGenerators; nTG++)
	{
		m_PerformanceConfig.SetBankNumber (m_nVoiceBankID[nTG], nTG);
		m_PerformanceConfig.SetVoiceNumber (m_nProgram[nTG], nTG);
		m_PerformanceConfig.SetMIDIChannel (m_nMIDIChannel[nTG], nTG);
		m_PerformanceConfig.SetVolume (m_nVolume[nTG], nTG);
		m_PerformanceConfig.SetPan (m_nPan[nTG], nTG);
		
		m_PerformanceConfig.SetInsertFX (m_InsertFX[nTG]->getId(), nTG);
		std::vector<unsigned> pParams = m_InsertFX[nTG]->getParameters();
		m_PerformanceConfig.SetInsertFXParams (pParams, nTG);
		pParams.clear();
		pParams.shrink_to_fit();
				
		m_PerformanceConfig.SetDetune (m_nMasterTune[nTG], nTG);
		m_PerformanceConfig.SetCutoff (m_nCutoff[nTG], nTG);
		m_PerformanceConfig.SetResonance (m_nResonance[nTG], nTG);
		m_PerformanceConfig.SetPitchBendRange (m_nPitchBendRange[nTG], nTG);
		m_PerformanceConfig.SetPitchBendStep	(m_nPitchBendStep[nTG], nTG);
		m_PerformanceConfig.SetPortamentoMode (m_nPortamentoMode[nTG], nTG);
		m_PerformanceConfig.SetPortamentoGlissando (m_nPortamentoGlissando[nTG], nTG);
		m_PerformanceConfig.SetPortamentoTime (m_nPortamentoTime[nTG], nTG);

		m_PerformanceConfig.SetNoteLimitLow (m_nNoteLimitLow[nTG], nTG);
		m_PerformanceConfig.SetNoteLimitHigh (m_nNoteLimitHigh[nTG], nTG);
		m_PerformanceConfig.SetNoteShift (m_nNoteShift[nTG], nTG);
		m_pTG[nTG]->getVoiceData(m_nRawVoiceData);  
 		m_PerformanceConfig.SetVoiceDataToTxt (m_nRawVoiceData, nTG); 
		m_PerformanceConfig.SetMonoMode (m_bMonoMode[nTG], nTG); 
				
		m_PerformanceConfig.SetModulationWheelRange (m_nModulationWheelRange[nTG], nTG);
		m_PerformanceConfig.SetModulationWheelTarget (m_nModulationWheelTarget[nTG], nTG);
		m_PerformanceConfig.SetFootControlRange (m_nFootControlRange[nTG], nTG);
		m_PerformanceConfig.SetFootControlTarget (m_nFootControlTarget[nTG], nTG);
		m_PerformanceConfig.SetBreathControlRange (m_nBreathControlRange[nTG], nTG);
		m_PerformanceConfig.SetBreathControlTarget (m_nBreathControlTarget[nTG], nTG);
		m_PerformanceConfig.SetAftertouchRange (m_nAftertouchRange[nTG], nTG);
		m_PerformanceConfig.SetAftertouchTarget (m_nAftertouchTarget[nTG], nTG);
		
		m_PerformanceConfig.SetReverbSend (m_nReverbSend[nTG], nTG);
	}

	m_PerformanceConfig.SetCompressorEnable (!!m_nParameter[ParameterCompressorEnable]);

	m_PerformanceConfig.SetSendFX (m_SendFX->getId());
	std::vector<unsigned> pParams = m_SendFX->getParameters();
	m_PerformanceConfig.SetSendFXParams (pParams);
	pParams.clear();
	pParams.shrink_to_fit();
	m_PerformanceConfig.SetSendFXLevel (roundf(m_SendFXLevel * 100));

	m_PerformanceConfig.SetReverbEnable (!!m_nParameter[ParameterReverbEnable]);
	m_PerformanceConfig.SetReverbSize (m_nParameter[ParameterReverbSize]);
	m_PerformanceConfig.SetReverbHighDamp (m_nParameter[ParameterReverbHighDamp]);
	m_PerformanceConfig.SetReverbLowDamp (m_nParameter[ParameterReverbLowDamp]);
	m_PerformanceConfig.SetReverbLowPass (m_nParameter[ParameterReverbLowPass]);
	m_PerformanceConfig.SetReverbDiffusion (m_nParameter[ParameterReverbDiffusion]);
	m_PerformanceConfig.SetReverbLevel (m_nParameter[ParameterReverbLevel]);

	if(m_bSaveAsDeault)
	{
		m_PerformanceConfig.SetNewPerformance(0);
		
	}
	return m_PerformanceConfig.Save ();
}

void CMiniDexed::setMonoMode(uint8_t mono, uint8_t nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);
	m_bMonoMode[nTG]= mono != 0; 
	m_pTG[nTG]->setMonoMode(constrain(mono, 0, 1));
	m_pTG[nTG]->doRefreshVoice();
	m_UI.ParameterChanged ();
}

void CMiniDexed::setPitchbendRange(uint8_t range, uint8_t nTG)
{
	range = constrain (range, 0, 12);
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);
	m_nPitchBendRange[nTG] = range;
	
	m_pTG[nTG]->setPitchbendRange(range);
	m_pTG[nTG]->ControllersRefresh();
	m_UI.ParameterChanged ();
}

void CMiniDexed::setPitchbendStep(uint8_t step, uint8_t nTG)
{
	step= constrain (step, 0, 12);
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);
	m_nPitchBendStep[nTG] = step;
	
	m_pTG[nTG]->setPitchbendStep(step);
	m_pTG[nTG]->ControllersRefresh();
	m_UI.ParameterChanged ();
}

void CMiniDexed::setPortamentoMode(uint8_t mode, uint8_t nTG)
{
	mode= constrain (mode, 0, 1);

	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);
	m_nPortamentoMode[nTG] = mode;
	
	m_pTG[nTG]->setPortamentoMode(mode);
	m_pTG[nTG]->ControllersRefresh();
	m_UI.ParameterChanged ();
}

void CMiniDexed::setPortamentoGlissando(uint8_t glissando, uint8_t nTG)
{
	glissando = constrain (glissando, 0, 1);
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);
	m_nPortamentoGlissando[nTG] = glissando;
	
	m_pTG[nTG]->setPortamentoGlissando(glissando);
	m_pTG[nTG]->ControllersRefresh();
	m_UI.ParameterChanged ();
}

void CMiniDexed::setPortamentoTime(uint8_t time, uint8_t nTG)
{
	time = constrain (time, 0, 99);
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);
	m_nPortamentoTime[nTG] = time;
	
	m_pTG[nTG]->setPortamentoTime(time);
	m_pTG[nTG]->ControllersRefresh();
	m_UI.ParameterChanged ();
}

void CMiniDexed::setModWheelRange(uint8_t range, uint8_t nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);

	m_nModulationWheelRange[nTG] = range;
	m_pTG[nTG]->setMWController(range, m_pTG[nTG]->getModWheelTarget(), 0);
//	m_pTG[nTG]->setModWheelRange(constrain(range, 0, 99));  replaces with the above due to wrong constrain on dexed_synth module. 

	m_pTG[nTG]->ControllersRefresh();
	m_UI.ParameterChanged ();
}

void CMiniDexed::setModWheelTarget(uint8_t target, uint8_t nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);

	m_nModulationWheelTarget[nTG] = target;

	m_pTG[nTG]->setModWheelTarget(constrain(target, 0, 7));
	m_pTG[nTG]->ControllersRefresh();
	m_UI.ParameterChanged ();
}

void CMiniDexed::setFootControllerRange(uint8_t range, uint8_t nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);

	m_nFootControlRange[nTG]=range;
	m_pTG[nTG]->setFCController(range, m_pTG[nTG]->getFootControllerTarget(), 0);
//	m_pTG[nTG]->setFootControllerRange(constrain(range, 0, 99));  replaces with the above due to wrong constrain on dexed_synth module. 

	m_pTG[nTG]->ControllersRefresh();
	m_UI.ParameterChanged ();
}

void CMiniDexed::setFootControllerTarget(uint8_t target, uint8_t nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);

	m_nFootControlTarget[nTG] = target;

	m_pTG[nTG]->setFootControllerTarget(constrain(target, 0, 7));
	m_pTG[nTG]->ControllersRefresh();
	m_UI.ParameterChanged ();
}

void CMiniDexed::setBreathControllerRange(uint8_t range, uint8_t nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);

	m_nBreathControlRange[nTG]=range;
	m_pTG[nTG]->setBCController(range, m_pTG[nTG]->getBreathControllerTarget(), 0);
	//m_pTG[nTG]->setBreathControllerRange(constrain(range, 0, 99));

	m_pTG[nTG]->ControllersRefresh();
	m_UI.ParameterChanged ();
}

void CMiniDexed::setBreathControllerTarget(uint8_t target, uint8_t nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);

	m_nBreathControlTarget[nTG]=target;

	m_pTG[nTG]->setBreathControllerTarget(constrain(target, 0, 7));
	m_pTG[nTG]->ControllersRefresh();
	m_UI.ParameterChanged ();
}

void CMiniDexed::setAftertouchRange(uint8_t range, uint8_t nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);

	m_nAftertouchRange[nTG]=range;
	m_pTG[nTG]->setATController(range, m_pTG[nTG]->getAftertouchTarget(), 0);
//	m_pTG[nTG]->setAftertouchRange(constrain(range, 0, 99));

	m_pTG[nTG]->ControllersRefresh();
	m_UI.ParameterChanged ();
}

void CMiniDexed::setAftertouchTarget(uint8_t target, uint8_t nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);

	m_nAftertouchTarget[nTG]=target;

	m_pTG[nTG]->setAftertouchTarget(constrain(target, 0, 7));
	m_pTG[nTG]->ControllersRefresh();
	m_UI.ParameterChanged ();
}

void CMiniDexed::loadVoiceParameters(const uint8_t* data, uint8_t nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);

	uint8_t voice[161];

	memcpy(voice, data, sizeof(uint8_t)*161);

	// fix voice name
	for (uint8_t i = 0; i < 10; i++)
	{
		if (voice[151 + i] > 126) // filter characters
			voice[151 + i] = 32;
	}

	m_pTG[nTG]->loadVoiceParameters(&voice[6]);
	m_pTG[nTG]->doRefreshVoice();
	m_UI.ParameterChanged ();
}

void CMiniDexed::setVoiceDataElement(uint8_t data, uint8_t number, uint8_t nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);

	m_pTG[nTG]->setVoiceDataElement(constrain(data, 0, 155),constrain(number, 0, 99));
	//m_pTG[nTG]->doRefreshVoice();
	m_UI.ParameterChanged ();
}

int16_t CMiniDexed::checkSystemExclusive(const uint8_t* pMessage,const  uint16_t nLength, uint8_t nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);

	return(m_pTG[nTG]->checkSystemExclusive(pMessage, nLength));
}

void CMiniDexed::getSysExVoiceDump(uint8_t* dest, uint8_t nTG)
{
	uint8_t checksum = 0;
	uint8_t data[155];

	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);

	m_pTG[nTG]->getVoiceData(data);

	dest[0] = 0xF0; // SysEx start
	dest[1] = 0x43; // ID=Yamaha
	dest[2] = 0x00 | m_nMIDIChannel[nTG]; // 0x0c Sub-status 0 and MIDI channel
	dest[3] = 0x00; // Format number (0=1 voice)
	dest[4] = 0x01; // Byte count MSB
	dest[5] = 0x1B; // Byte count LSB
	for (uint8_t n = 0; n < 155; n++)
	{
		checksum -= data[n];
		dest[6 + n] = data[n];
	}
	dest[161] = checksum & 0x7f; // Checksum
	dest[162] = 0xF7; // SysEx end
}

void CMiniDexed::setMasterVolume (float32_t vol)
{
	if(vol < 0.0)
		vol = 0.0;
	else if(vol > 1.0)
		vol = 1.0;

	nMasterVolume=vol;
}

std::string CMiniDexed::GetPerformanceFileName(unsigned nID)
{
	return m_PerformanceConfig.GetPerformanceFileName(nID);
}

std::string CMiniDexed::GetPerformanceName(unsigned nID)
{
	return m_PerformanceConfig.GetPerformanceName(nID);
}

unsigned CMiniDexed::GetLastPerformance()
{
	return m_PerformanceConfig.GetLastPerformance();
}

unsigned CMiniDexed::GetPerformanceBank()
{
	return m_PerformanceConfig.GetPerformanceBank();
}

unsigned CMiniDexed::GetLastPerformanceBank()
{
	return m_PerformanceConfig.GetLastPerformanceBank();
}

unsigned CMiniDexed::GetActualPerformanceID()
{
	return m_PerformanceConfig.GetActualPerformanceID();
}

void CMiniDexed::SetActualPerformanceID(unsigned nID)
{
	m_PerformanceConfig.SetActualPerformanceID(nID);
}

unsigned CMiniDexed::GetActualPerformanceBankID()
{
	return m_PerformanceConfig.GetActualPerformanceBankID();
}

void CMiniDexed::SetActualPerformanceBankID(unsigned nBankID)
{
	m_PerformanceConfig.SetActualPerformanceBankID(nBankID);
}

bool CMiniDexed::SetNewPerformance(unsigned nID)
{
	m_bSetNewPerformance = true;
	m_nSetNewPerformanceID = nID;

	return true;
}

bool CMiniDexed::SetNewPerformanceBank(unsigned nBankID)
{
	m_bSetNewPerformanceBank = true;
	m_nSetNewPerformanceBankID = nBankID;

	return true;
}

void CMiniDexed::SetFirstPerformance(void)
{
	m_bSetFirstPerformance = true;
	return;
}

bool CMiniDexed::DoSetNewPerformance (void)
{
	m_bLoadPerformanceBusy = true;
	
	unsigned nID = m_nSetNewPerformanceID;
	m_PerformanceConfig.SetNewPerformance(nID);
	
	if (m_PerformanceConfig.Load ())
	{
		LoadPerformanceParameters();
		m_bLoadPerformanceBusy = false;
		return true;
	}
	else
	{
		SetMIDIChannel (CMIDIDevice::OmniMode, 0);
		m_bLoadPerformanceBusy = false;
		return false;
	}
}

bool CMiniDexed::DoSetNewPerformanceBank (void)
{
	m_bLoadPerformanceBankBusy = true;
	
	unsigned nBankID = m_nSetNewPerformanceBankID;
	m_PerformanceConfig.SetNewPerformanceBank(nBankID);
	
	m_bLoadPerformanceBankBusy = false;
	return true;
}

void CMiniDexed::DoSetFirstPerformance(void)
{
	unsigned nID = m_PerformanceConfig.FindFirstPerformance();
	SetNewPerformance(nID);
	m_bSetFirstPerformance = false;
	return;
}

bool CMiniDexed::SavePerformanceNewFile ()
{
	m_bSavePerformanceNewFile = m_PerformanceConfig.GetInternalFolderOk() && m_PerformanceConfig.CheckFreePerformanceSlot();
	return m_bSavePerformanceNewFile;
}

bool CMiniDexed::DoSavePerformanceNewFile (void)
{
	if (m_PerformanceConfig.CreateNewPerformanceFile())
	{
		if(SavePerformance(false))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
	
}


void CMiniDexed::LoadPerformanceParameters(void)
{
	for (unsigned nTG = 0; nTG < CConfig::ToneGenerators; nTG++)
		{
			
			BankSelect (m_PerformanceConfig.GetBankNumber (nTG), nTG);
			ProgramChange (m_PerformanceConfig.GetVoiceNumber (nTG), nTG);
			SetMIDIChannel (m_PerformanceConfig.GetMIDIChannel (nTG), nTG);
			SetVolume (m_PerformanceConfig.GetVolume (nTG), nTG);
			SetPan (m_PerformanceConfig.GetPan (nTG), nTG);
			SetMasterTune (m_PerformanceConfig.GetDetune (nTG), nTG);
			SetCutoff (m_PerformanceConfig.GetCutoff (nTG), nTG);
			SetResonance (m_PerformanceConfig.GetResonance (nTG), nTG);
			setPitchbendRange (m_PerformanceConfig.GetPitchBendRange (nTG), nTG);
			setPitchbendStep (m_PerformanceConfig.GetPitchBendStep (nTG), nTG);
			setPortamentoMode (m_PerformanceConfig.GetPortamentoMode (nTG), nTG);
			setPortamentoGlissando (m_PerformanceConfig.GetPortamentoGlissando  (nTG), nTG);
			setPortamentoTime (m_PerformanceConfig.GetPortamentoTime (nTG), nTG);

			setInsertFXType(m_PerformanceConfig.GetInsertFX (nTG), nTG);
			std::vector<unsigned> pParams = m_PerformanceConfig.GetInsertFXParams(nTG);
			m_InsertFX[nTG]->setParameters(pParams);
			pParams.clear();
			pParams.shrink_to_fit();
			
			m_nNoteLimitLow[nTG] = m_PerformanceConfig.GetNoteLimitLow (nTG);
			m_nNoteLimitHigh[nTG] = m_PerformanceConfig.GetNoteLimitHigh (nTG);
			m_nNoteShift[nTG] = m_PerformanceConfig.GetNoteShift (nTG);
			
			if(m_PerformanceConfig.VoiceDataFilled(nTG)) 
			{
			uint8_t* tVoiceData = m_PerformanceConfig.GetVoiceDataFromTxt(nTG);
			m_pTG[nTG]->loadVoiceParameters(tVoiceData); 
			}
			setMonoMode(m_PerformanceConfig.GetMonoMode(nTG) ? 1 : 0, nTG); 
			SetReverbSend (m_PerformanceConfig.GetReverbSend (nTG), nTG);
					
			setModWheelRange (m_PerformanceConfig.GetModulationWheelRange (nTG),  nTG);
			setModWheelTarget (m_PerformanceConfig.GetModulationWheelTarget (nTG),  nTG);
			setFootControllerRange (m_PerformanceConfig.GetFootControlRange (nTG),  nTG);
			setFootControllerTarget (m_PerformanceConfig.GetFootControlTarget (nTG),  nTG);
			setBreathControllerRange (m_PerformanceConfig.GetBreathControlRange (nTG),  nTG);
			setBreathControllerTarget (m_PerformanceConfig.GetBreathControlTarget (nTG),  nTG);
			setAftertouchRange (m_PerformanceConfig.GetAftertouchRange (nTG),  nTG);
			setAftertouchTarget (m_PerformanceConfig.GetAftertouchTarget (nTG),  nTG);
			
		
		}

		// Effects
		SetParameter (ParameterCompressorEnable, m_PerformanceConfig.GetCompressorEnable () ? 1 : 0);
		
		setSendFXType(m_PerformanceConfig.GetSendFX ());
		std::vector<unsigned> pParams = m_PerformanceConfig.GetSendFXParams ();
		m_SendFX->setParameters(pParams);
		pParams.clear();
		pParams.shrink_to_fit();
		SetParameter (ParameterSendFXLevel, m_PerformanceConfig.GetSendFXLevel ());

		SetParameter (ParameterReverbEnable, m_PerformanceConfig.GetReverbEnable () ? 1 : 0);
		SetParameter (ParameterReverbSize, m_PerformanceConfig.GetReverbSize ());
		SetParameter (ParameterReverbHighDamp, m_PerformanceConfig.GetReverbHighDamp ());
		SetParameter (ParameterReverbLowDamp, m_PerformanceConfig.GetReverbLowDamp ());
		SetParameter (ParameterReverbLowPass, m_PerformanceConfig.GetReverbLowPass ());
		SetParameter (ParameterReverbDiffusion, m_PerformanceConfig.GetReverbDiffusion ());
		SetParameter (ParameterReverbLevel, m_PerformanceConfig.GetReverbLevel ());
}

std::string CMiniDexed::GetNewPerformanceDefaultName(void)	
{
	return m_PerformanceConfig.GetNewPerformanceDefaultName();
}

void CMiniDexed::SetNewPerformanceName(std::string nName)
{
	m_PerformanceConfig.SetNewPerformanceName(nName);
}

bool CMiniDexed::IsValidPerformance(unsigned nID)
{
	return m_PerformanceConfig.IsValidPerformance(nID);
}

bool CMiniDexed::IsValidPerformanceBank(unsigned nBankID)
{
	return m_PerformanceConfig.IsValidPerformanceBank(nBankID);
}

void CMiniDexed::SetVoiceName (std::string VoiceName, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);
	char Name[11];
	strncpy(Name, VoiceName.c_str(),10);
	Name[10] = '\0';
	m_pTG[nTG]->getName (Name);
}

bool CMiniDexed::DeletePerformance(unsigned nID)
{
	if (m_PerformanceConfig.IsValidPerformance(nID) && m_PerformanceConfig.GetInternalFolderOk())
	{
		m_bDeletePerformance = true;
		m_nDeletePerformanceID = nID;

		return true;
	}
	else
	{
		return false;
	}
}

bool CMiniDexed::DoDeletePerformance(void)
{
	unsigned nID = m_nDeletePerformanceID;
	if(m_PerformanceConfig.DeletePerformance(nID))
	{
		if (m_PerformanceConfig.Load ())
		{
			LoadPerformanceParameters();
			return true;
		}
		else
		{
			SetMIDIChannel (CMIDIDevice::OmniMode, 0);
		}
	}
	
	return false;
}

bool CMiniDexed::GetPerformanceSelectToLoad(void)
{
	return m_pConfig->GetPerformanceSelectToLoad();
}

void CMiniDexed::setModController (unsigned controller, unsigned parameter, uint8_t value, uint8_t nTG)
{
	 uint8_t nBits;
	
	switch (controller)
	{
		case 0:
			if (parameter == 0)
			{
				setModWheelRange(value, nTG);
			}
			else
			{
				value=constrain(value, 0, 1);
				nBits=m_nModulationWheelTarget[nTG];
				value == 1 ?  nBits |= 1 << (parameter-1) : nBits &= ~(1 << (parameter-1)); 
				setModWheelTarget(nBits , nTG); 
			}
		break;
		
		case 1:
			if (parameter == 0)
			{
				setFootControllerRange(value, nTG);
			}
			else
			{
				value=constrain(value, 0, 1);
				nBits=m_nFootControlTarget[nTG];
				value == 1 ?  nBits |= 1 << (parameter-1) : nBits &= ~(1 << (parameter-1)); 
				setFootControllerTarget(nBits , nTG); 
			}
		break;	

		case 2:
			if (parameter == 0)
			{
				setBreathControllerRange(value, nTG);
			}
			else
			{
				value=constrain(value, 0, 1);
				nBits=m_nBreathControlTarget[nTG];
				value == 1 ?  nBits |= 1 << (parameter-1) : nBits &= ~(1 << (parameter-1));
				setBreathControllerTarget(nBits , nTG); 
			}
		break;			
		
		case 3:
			if (parameter == 0)
			{
				setAftertouchRange(value, nTG);
			}
			else
			{
				value=constrain(value, 0, 1);
				nBits=m_nAftertouchTarget[nTG];
				value == 1 ?  nBits |= 1 << (parameter-1) : nBits &= ~(1 << (parameter-1));
				setAftertouchTarget(nBits , nTG); 
			}
		break;	
		default:
		break;
	}
}

unsigned CMiniDexed::getModController (unsigned controller, unsigned parameter, uint8_t nTG)
{
	unsigned nBits;
	switch (controller)
	{
		case 0:
			if (parameter == 0)
			{
			    return m_nModulationWheelRange[nTG];
			}
			else
			{
	
				nBits=m_nModulationWheelTarget[nTG];
				nBits &= 1 << (parameter-1);				
				return (nBits != 0 ? 1 : 0) ; 
			}
		break;
		
		case 1:
			if (parameter == 0)
			{
				return m_nFootControlRange[nTG];
			}
			else
			{
				nBits=m_nFootControlTarget[nTG];
				nBits &= 1 << (parameter-1)	;			
				return (nBits != 0 ? 1 : 0) ; 
			}
		break;	

		case 2:
			if (parameter == 0)
			{
				return m_nBreathControlRange[nTG];
			}
			else
			{
				nBits=m_nBreathControlTarget[nTG];	
				nBits &= 1 << (parameter-1)	;			
				return (nBits != 0 ? 1 : 0) ; 
			}
		break;			
		
		case 3:
			if (parameter == 0)
			{
				return m_nAftertouchRange[nTG];
			}
			else
			{
				nBits=m_nAftertouchTarget[nTG];
				nBits &= 1 << (parameter-1)	;			
				return (nBits != 0 ? 1 : 0) ; 
			}
		break;	
		
		default:
			return 0;
		break;
	}
	
}

