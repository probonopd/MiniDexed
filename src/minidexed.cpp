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

LOGMODULE ("minidexed");

CMiniDexed::CMiniDexed (	
	CConfig *pConfig, 
	CInterruptSystem *pInterrupt,
	CGPIOManager *pGPIOManager, 
	CI2CMaster *pI2CMaster, 
	FATFS *pFileSystem
) :
#ifdef ARM_ALLOW_MULTI_CORE
	CMultiCoreSupport (CMemorySystem::Get ()),
#endif
	m_pConfig (pConfig),
	m_UI (this, pGPIOManager, pI2CMaster, pConfig),
	m_PerformanceConfig (pFileSystem),
	m_PCKeyboard (this, pConfig, &m_UI),
	m_SerialMIDI (this, pInterrupt, pConfig, &m_UI),
	m_bUseSerial (false),
	m_pSoundDevice (0),
	m_bChannelsSwapped (pConfig->GetChannelsSwapped ()),
#ifdef ARM_ALLOW_MULTI_CORE
	m_nActiveTGsLog2 (0),
#endif
							 
							 
	m_GetChunkTimer ("GetChunk", 1000000U * pConfig->GetChunkSize ()/2 / pConfig->GetSampleRate ()),
	m_bProfileEnabled (m_pConfig->GetProfileEnabled ()),
	m_bSavePerformance (false),
	m_bSavePerformanceNewFile (false),
	m_bSetNewPerformance (false),
	m_bDeletePerformance (false),
	m_bLoadPerformanceBusy(false)
{
	assert (m_pConfig);

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
		
								
								
							
							 
							   
							   
							
						   
  
		m_nModulationWheelRange[i] = 99;
		m_nModulationWheelTarget[i] = 7;
		m_nFootControlRange[i] = 99;
		m_nFootControlTarget[i] = 0;	
		m_nBreathControlRange[i] = 99;	
		m_nBreathControlTarget[i] = 0;	
		m_nAftertouchRange[i] = 99;	
		m_nAftertouchTarget[i] = 0;

#if defined(MIXING_CONSOLE_ENABLE)
		memset(this->m_nTGSendLevel[i], 0, MixerOutput::kFXCount * sizeof(unsigned));
#elif defined(PLATE_REVERB_ENABLE)
		m_nReverbSend[i] = 0;
#endif
		m_uchOPMask[i] = 0b111111;	// All operators on

		m_pTG[i] = new CDexedAdapter (CConfig::MaxNotes, pConfig->GetSampleRate ());
		assert (m_pTG[i]);

		m_pTG[i]->activate ();
	}

#if defined(MIXING_CONSOLE_ENABLE)
	for(size_t i = MixerOutput::FX_Tube; i < (MixerOutput::kFXCount - 1); ++i)
	{
		memset(this->m_nFXSendLevel[i], 0, MixerOutput::kFXCount * sizeof(unsigned));
	}

	this->m_nTGSendLevel[0][MixerOutput::MainOutput] = 99;
	this->m_nTGSendLevel[0][MixerOutput::FX_PlateReverb] = 99;
	this->m_nFXSendLevel[MixerOutput::FX_PlateReverb][MixerOutput::MainOutput] = 99;
#endif

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
		LOGNOTE ("HDMI mode");

		m_pSoundDevice = new CHDMISoundBaseDevice (pInterrupt, pConfig->GetSampleRate (),
							   pConfig->GetChunkSize ());

		// The channels are swapped by default in the HDMI sound driver.
		// TODO: Remove this line, when this has been fixed in the driver.
		m_bChannelsSwapped = !m_bChannelsSwapped;
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

	this->setMasterVolume(1.0);

#if defined(MIXING_CONSOLE_ENABLE)
	this->mixing_console_ = new Mixer(static_cast<float32_t>(pConfig->GetSampleRate()), CConfig::MaxChunkSize, this->m_bChannelsSwapped);
	for (uint8_t i = 0; i < CConfig::ToneGenerators; i++)
	{
		memset(this->m_OutputLevel[i], 0, CConfig::MaxChunkSize * sizeof(float32_t));
		this->mixing_console_->setInputSampleBuffer(i, this->m_OutputLevel[i]);
	}

	// Tube parameters
	this->SetParameter(TParameter::ParameterFXTubeEnable, 1);
	this->SetParameter(TParameter::ParameterFXTubeOverdrive, 25);

	// Chorus parameters
	this->SetParameter(TParameter::ParameterFXChorusEnable, 1);
	this->SetParameter(TParameter::ParameterFXChorusRate, 50);
	this->SetParameter(TParameter::ParameterFXChorusDepth, 50);
	
	// Flanger parameters
	this->SetParameter(TParameter::ParameterFXFlangerEnable, 1);
	this->SetParameter(TParameter::ParameterFXFlangerRate, 3);
	this->SetParameter(TParameter::ParameterFXFlangerDepth, 75);
	this->SetParameter(TParameter::ParameterFXFlangerFeedback, 50);

	// Orbitone parameters
	this->SetParameter(TParameter::ParameterFXOrbitoneEnable, 1);
	this->SetParameter(TParameter::ParameterFXOrbitoneRate, 40);
	this->SetParameter(TParameter::ParameterFXOrbitoneDepth, 50);

	// Phaser parameters
	this->SetParameter(TParameter::ParameterFXPhaserEnable, 1);
	this->SetParameter(TParameter::ParameterFXPhaserRate, 5);
	this->SetParameter(TParameter::ParameterFXPhaserDepth, 99);
	this->SetParameter(TParameter::ParameterFXPhaserFeedback, 50);
	this->SetParameter(TParameter::ParameterFXPhaserNbStages, 12);

	// Delay parameters
	this->SetParameter(TParameter::ParameterFXDelayEnable, 1);
	this->SetParameter(TParameter::ParameterFXDelayLeftDelayTime, 15);
	this->SetParameter(TParameter::ParameterFXDelayRightDelayTime, 22);
	this->SetParameter(TParameter::ParameterFXDelayFeedback, 35);

	// AudioEffectPlateReverb parameters
	this->SetParameter(TParameter::ParameterReverbEnable, 1);
	this->SetParameter(TParameter::ParameterReverbSize, 70);
	this->SetParameter(TParameter::ParameterReverbHighDamp, 50);
	this->SetParameter(TParameter::ParameterReverbLowDamp, 50);
	this->SetParameter(TParameter::ParameterReverbLowPass, 30);
	this->SetParameter(TParameter::ParameterReverbDiffusion, 65);
	this->SetParameter(TParameter::ParameterReverbLevel, 99);

	// Reverberator parameters
	this->SetParameter(TParameter::ParameterFXReverberatorEnable, 1);
	this->SetParameter(TParameter::ParameterFXReverberatorInputGain, 99);
	this->SetParameter(TParameter::ParameterFXReverberatorTime, 80);
	this->SetParameter(TParameter::ParameterFXReverberatorDiffusion, 80);
	this->SetParameter(TParameter::ParameterFXReverberatorLP, 70);

	// Bypass
	this->SetParameter(TParameter::ParameterFXBypass, 0);

#elif defined(PLATE_REVERB_ENABLE)

	// BEGIN setup tg_mixer
	tg_mixer = new AudioStereoMixer<CConfig::ToneGenerators>(pConfig->GetChunkSize()/2);
	// END setup tgmixer

	// BEGIN setup reverb
	reverb_send_mixer = new AudioStereoMixer<CConfig::ToneGenerators>(pConfig->GetChunkSize()/2);
	reverb = new AudioEffectPlateReverb(pConfig->GetSampleRate());
	SetParameter (TParameter::ParameterReverbEnable, 1);
	SetParameter (TParameter::ParameterReverbSize, 70);
	SetParameter (TParameter::ParameterReverbHighDamp, 50);
	SetParameter (TParameter::ParameterReverbLowDamp, 50);
	SetParameter (TParameter::ParameterReverbLowPass, 30);
	SetParameter (TParameter::ParameterReverbDiffusion, 65);
	SetParameter (TParameter::ParameterReverbLevel, 99);
	// END setup reverb

#endif

	this->SetParameter (TParameter::ParameterCompressorEnable, 1);
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
		
#if defined(MIXING_CONSOLE_ENABLE)
		this->mixing_console_->reset();
		this->mixing_console_->setPan(i, this->m_nPan[i] / 127.0f);

		this->mixing_console_->setSendLevel(i, MixerOutput::FX_PlateReverb, this->m_nTGSendLevel[i][MixerOutput::FX_PlateReverb] / 99.0f);
		this->mixing_console_->setSendLevel(i, MixerOutput::MainOutput, this->m_nTGSendLevel[i][MixerOutput::MainOutput] / 99.0f);
		this->mixing_console_->setFXSendLevel(MixerOutput::FX_PlateReverb, MixerOutput::MainOutput, this->m_nFXSendLevel[MixerOutput::FX_PlateReverb][MixerOutput::FX_PlateReverb] / 99.0f);

#elif defined(PLATE_REVERB_ENABLE)

		tg_mixer->pan(i,mapfloat(m_nPan[i],0,127,0.0f,1.0f));
		tg_mixer->gain(i,1.0f);
		reverb_send_mixer->pan(i,mapfloat(m_nPan[i],0,127,0.0f,1.0f));
		reverb_send_mixer->gain(i,mapfloat(m_nReverbSend[i],0,99,0.0f,1.0f));
#endif
	}

	if (m_PerformanceConfig.Load ())
	{
		LoadPerformanceParameters(); 
	}
	else
	{
		SetMIDIChannel (CMIDIDevice::OmniMode, 0);
	}

	// load performances file list, and attempt to create the performance folder
	if (!m_PerformanceConfig.ListPerformances()) 
	{
		LOGERR ("Cannot create internal Performance folder, new performances can't be created");
	}
	
	// setup and start the sound device
	if (!m_pSoundDevice->AllocateQueueFrames (m_pConfig->GetChunkSize ()))
	{
		LOGERR ("Cannot allocate sound queue");

		return false;
	}

#if defined(ARM_ALLOW_MULTI_CORE)
																		
	 
	m_pSoundDevice->SetWriteFormat (SoundFormatSigned16, 2);	// 16-bit Stereo
#else
	m_pSoundDevice->SetWriteFormat (SoundFormatSigned16, 1);	// 16-bit Mono
#endif

	m_nQueueSizeFrames = m_pSoundDevice->GetQueueSizeFrames ();

	m_pSoundDevice->Start ();

#if defined(ARM_ALLOW_MULTI_CORE)
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
	
	if (m_bSetNewPerformance && !m_bLoadPerformanceBusy)
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
				m_pTG[nTG]->getSamples (m_OutputLevel[nTG],m_nFramesToProcess);

#if defined(MIXING_CONSOLE_ENABLE)
				this->mixing_console_->preProcessInputSampleBuffer(nTG, this->m_nFramesToProcess);
#endif
			}
		}
	}
}

#endif

CSysExFileLoader *CMiniDexed::GetSysExFileLoader (void)
{
	return &m_SysExFileLoader;
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

void CMiniDexed::SetVolume (unsigned nVolume, unsigned nTG)
{
	nVolume=constrain((int)nVolume,0,127);

	assert (nTG < CConfig::ToneGenerators);
	m_nVolume[nTG] = nVolume;

	assert (m_pTG[nTG]);
	m_pTG[nTG]->setGain (nVolume / 127.0f);

#if defined(MIXING_CONSOLE_ENABLE)
	this->mixing_console_->setChannelLevel(nTG, nVolume == 0 ? 0.0f : 1.0f);
#endif

	m_UI.ParameterChanged ();
}

void CMiniDexed::SetPan (unsigned nPan, unsigned nTG)
{
	nPan=constrain((int)nPan,0,127);

	assert (nTG < CConfig::ToneGenerators);
	m_nPan[nTG] = nPan;

#if defined(MIXING_CONSOLE_ENABLE)
	this->mixing_console_->setPan(nTG, nPan / 127.0f);

#elif defined(PLATE_REVERB_ENABLE)

	tg_mixer->pan(nTG,mapfloat(nPan,0,127,0.0f,1.0f));
	reverb_send_mixer->pan(nTG,mapfloat(nPan,0,127,0.0f,1.0f));
#endif

	m_UI.ParameterChanged ();
}

#if defined(MIXING_CONSOLE_ENABLE)

unsigned CMiniDexed::getMixingConsoleSendLevel(unsigned nTG, MixerOutput fx) const
{
	assert (nTG < CConfig::ToneGenerators);
	return this->m_nTGSendLevel[nTG][fx];
}

void CMiniDexed::setMixingConsoleSendLevel(unsigned nTG, MixerOutput fx, unsigned nFXSend)
{
	assert (nTG < CConfig::ToneGenerators);
	nFXSend = constrain((int)nFXSend, 0, 99);

	this->m_nTGSendLevel[nTG][fx] = nFXSend;
	this->mixing_console_->setSendLevel(nTG, fx, nFXSend / 99.0f);

	this->m_UI.ParameterChanged();
}

void CMiniDexed::setMixingConsoleFXSendLevel(MixerOutput fromFX, MixerOutput toFX, unsigned nFXSend)
{
	assert(fromFX < (MixerOutput::kFXCount - 1));
	assert(toFX < MixerOutput::kFXCount);
	if(fromFX != toFX)
	{
		nFXSend = constrain((int)nFXSend, 0, 99);
		this->m_nFXSendLevel[fromFX][toFX] = nFXSend;
		this->mixing_console_->setFXSendLevel(fromFX, toFX, nFXSend / 99.0f);
	}
}

#elif defined(PLATE_REVERB_ENABLE)

void CMiniDexed::SetReverbSend (unsigned nReverbSend, unsigned nTG)
{
	nReverbSend=constrain((int)nReverbSend,0,99);

	assert (nTG < CConfig::ToneGenerators);
	m_nReverbSend[nTG] = nReverbSend;

	reverb_send_mixer->gain(nTG,mapfloat(nReverbSend,0,99,0.0f,1.0f));
	
	m_UI.ParameterChanged ();
}

#endif

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

void CMiniDexed::keyup (int16_t pitch, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);

	pitch = ApplyNoteLimits (pitch, nTG);
	if (pitch >= 0)
	{
		m_pTG[nTG]->keyup (pitch);
	}
}

void CMiniDexed::keydown (int16_t pitch, uint8_t velocity, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);

	pitch = ApplyNoteLimits (pitch, nTG);
	if (pitch >= 0)
	{
		m_pTG[nTG]->keydown (pitch, velocity);
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
				 

	assert(Parameter < TParameter::ParameterUnknown);
	
	m_nParameter[Parameter] = nValue;

	switch (Parameter)
	{
	case TParameter::ParameterCompressorEnable:
		for (unsigned nTG = 0; nTG < CConfig::ToneGenerators; ++nTG)
		{
			assert(m_pTG[nTG]);
			m_pTG[nTG]->setCompressor (!!nValue);
		}
		break;

#if defined(MIXING_CONSOLE_ENABLE)

	// Tube parameters
	case TParameter::ParameterFXTubeEnable: 
		this->m_FXSpinLock.Acquire();
		this->mixing_console_->getTube()->setMute(!!!nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXTubeOverdrive: 
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->mixing_console_->getTube()->setOverdrive(nValue / 99.0f);
		this->m_FXSpinLock.Release();
		break;
	
	// Chorus parameters
	case TParameter::ParameterFXChorusEnable: 
		this->m_FXSpinLock.Acquire();
		this->mixing_console_->getChorus()->setMute(!!!nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXChorusRate: 
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->mixing_console_->getChorus()->setRate(nValue / 99.0f);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXChorusDepth: 
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->mixing_console_->getChorus()->setDepth(nValue / 9.9f);
		this->m_FXSpinLock.Release();
		break;
	
	// Flanger parameters
	case TParameter::ParameterFXFlangerEnable: 
		this->m_FXSpinLock.Acquire();
		this->mixing_console_->getFlanger()->setMute(!!!nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXFlangerRate: 
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->mixing_console_->getFlanger()->setRate(nValue / 99.0f);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXFlangerDepth: 
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->mixing_console_->getFlanger()->setDepth(nValue / 99.0f);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXFlangerFeedback: 
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->mixing_console_->getFlanger()->setFeedback(mapfloat(nValue, 0, 99, 0.0f, 0.97f));
		this->m_FXSpinLock.Release();
		break;
	
	// Orbitone parameters
	case TParameter::ParameterFXOrbitoneEnable: 
		this->m_FXSpinLock.Acquire();
		this->mixing_console_->getOrbitone()->setMute(!!!nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXOrbitoneRate: 
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->mixing_console_->getOrbitone()->setRate(nValue / 99.0f);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXOrbitoneDepth: 
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->mixing_console_->getOrbitone()->setDepth(nValue / 99.0f);
		this->m_FXSpinLock.Release();
		break;
	
	// Phaser parameters
	case TParameter::ParameterFXPhaserEnable: 
		this->m_FXSpinLock.Acquire();
		this->mixing_console_->getPhaser()->setMute(!!!nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXPhaserRate: 
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->mixing_console_->getPhaser()->setRate(nValue / 99.0f);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXPhaserDepth: 
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->mixing_console_->getPhaser()->setDepth(nValue / 99.0f);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXPhaserFeedback: 
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->mixing_console_->getPhaser()->setFeedback(mapfloat(nValue, 0, 99, 0.0f, 0.97f));
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXPhaserNbStages: 
		nValue = constrain((int)nValue, 2, MAX_NB_PHASES);
		this->m_FXSpinLock.Acquire();
		this->mixing_console_->getPhaser()->setNbStages(nValue);
		this->m_FXSpinLock.Release();
		break;

	// Delay parameters
	case TParameter::ParameterFXDelayEnable: 
		this->m_FXSpinLock.Acquire();
		this->mixing_console_->getDelay()->setMute(!!!nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXDelayLeftDelayTime: 
		nValue = constrain((int)nValue, -99, 99);
		this->m_FXSpinLock.Acquire();
		this->mixing_console_->getDelay()->setLeftDelayTime(nValue / 99.0f);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXDelayRightDelayTime: 
		nValue = constrain((int)nValue, -99, 99);
		this->m_FXSpinLock.Acquire();
		this->mixing_console_->getDelay()->setRightDelayTime(nValue / 99.0f);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXDelayFeedback: 
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->mixing_console_->getDelay()->setFeedback(nValue / 99.0f);
		this->m_FXSpinLock.Release();
		break;

	// AudioEffectPlateReverb parameters
	case TParameter::ParameterReverbEnable:
		this->m_FXSpinLock.Acquire ();
		this->mixing_console_->getPlateReverb()->set_bypass (!!!nValue);
		this->m_FXSpinLock.Release ();
		break;
	case TParameter::ParameterReverbSize:
		nValue=constrain((int)nValue,0,99);
		this->m_FXSpinLock.Acquire ();
		this->mixing_console_->getPlateReverb()->size (nValue / 99.0f);
		this->m_FXSpinLock.Release ();
		break;
	case TParameter::ParameterReverbHighDamp:
		nValue=constrain((int)nValue,0,99);
		this->m_FXSpinLock.Acquire ();
		this->mixing_console_->getPlateReverb()->hidamp (nValue / 99.0f);
		this->m_FXSpinLock.Release ();
		break;
	case TParameter::ParameterReverbLowDamp:
		nValue=constrain((int)nValue,0,99);
		this->m_FXSpinLock.Acquire ();
		this->mixing_console_->getPlateReverb()->lodamp (nValue / 99.0f);
		this->m_FXSpinLock.Release ();
		break;
	case TParameter::ParameterReverbLowPass:
		nValue=constrain((int)nValue,0,99);
		this->m_FXSpinLock.Acquire ();
		this->mixing_console_->getPlateReverb()->lowpass (nValue / 99.0f);
		this->m_FXSpinLock.Release ();
		break;
	case TParameter::ParameterReverbDiffusion:
		nValue=constrain((int)nValue,0,99);
		this->m_FXSpinLock.Acquire ();
		this->mixing_console_->getPlateReverb()->diffusion (nValue / 99.0f);
		this->m_FXSpinLock.Release ();
		break;
	case TParameter::ParameterReverbLevel:
		nValue=constrain((int)nValue,0,99);
		this->m_FXSpinLock.Acquire ();
		this->mixing_console_->getPlateReverb()->level (nValue / 99.0f);
		this->m_FXSpinLock.Release ();
		break;

	// Reverberator parameters
	case TParameter::ParameterFXReverberatorEnable: 
		this->m_FXSpinLock.Acquire();
		this->mixing_console_->getReverberator()->setMute(!!!nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXReverberatorInputGain: 
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->mixing_console_->getReverberator()->setInputGain(nValue / 99.0f);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXReverberatorTime: 
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->mixing_console_->getReverberator()->setTime(nValue / 99.0f);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXReverberatorDiffusion: 
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->mixing_console_->getReverberator()->setDiffusion(nValue / 99.0f);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXReverberatorLP: 
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->mixing_console_->getReverberator()->setLP(nValue / 99.0f);
		this->m_FXSpinLock.Release();
		break;

	// Tube Send parameters
	case TParameter::ParameterFXTube_ChorusSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Tube, MixerOutput::FX_Chorus, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXTube_FlangerSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Tube, MixerOutput::FX_Flanger, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXTube_OrbitoneSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Tube, MixerOutput::FX_Orbitone, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXTube_PhaserSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Tube, MixerOutput::FX_Phaser, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXTube_DelaySend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Tube, MixerOutput::FX_Delay, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXTube_PlateReverbSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Tube, MixerOutput::FX_PlateReverb, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXTube_ReverberatorSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Tube, MixerOutput::FX_Reverberator, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXTube_MainOutput:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Tube, MixerOutput::MainOutput, nValue);
		this->m_FXSpinLock.Release();
		break;

	// Chorus Send parameters
	case TParameter::ParameterFXChorus_TubeSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Chorus, MixerOutput::FX_Tube, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXChorus_FlangerSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Chorus, MixerOutput::FX_Flanger, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXChorus_OrbitoneSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Chorus, MixerOutput::FX_Orbitone, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXChorus_PhaserSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Chorus, MixerOutput::FX_Phaser, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXChorus_DelaySend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Chorus, MixerOutput::FX_Delay, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXChorus_PlateReverbSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Chorus, MixerOutput::FX_PlateReverb, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXChorus_ReverberatorSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Chorus, MixerOutput::FX_Reverberator, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXChorus_MainOutput:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Chorus, MixerOutput::MainOutput, nValue);
		this->m_FXSpinLock.Release();
		break;

	// Flanger Send parameters
	case TParameter::ParameterFXFlanger_TubeSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Flanger, MixerOutput::FX_Tube, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXFlanger_ChorusSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Flanger, MixerOutput::FX_Chorus, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXFlanger_OrbitoneSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Flanger, MixerOutput::FX_Orbitone, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXFlanger_PhaserSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Flanger, MixerOutput::FX_Phaser, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXFlanger_DelaySend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Flanger, MixerOutput::FX_Delay, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXFlanger_PlateReverbSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Flanger, MixerOutput::FX_PlateReverb, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXFlanger_ReverberatorSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Flanger, MixerOutput::FX_Reverberator, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXFlanger_MainOutput:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Flanger, MixerOutput::MainOutput, nValue);
		this->m_FXSpinLock.Release();
		break;

	// Orbitone Send parameters
	case TParameter::ParameterFXOrbitone_TubeSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Orbitone, MixerOutput::FX_Tube, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXOrbitone_ChorusSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Orbitone, MixerOutput::FX_Chorus, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXOrbitone_FlangerSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Orbitone, MixerOutput::FX_Flanger, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXOrbitone_PhaserSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Orbitone, MixerOutput::FX_Phaser, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXOrbitone_DelaySend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Orbitone, MixerOutput::FX_Delay, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXOrbitone_PlateReverbSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Orbitone, MixerOutput::FX_PlateReverb, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXOrbitone_ReverberatorSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Orbitone, MixerOutput::FX_Reverberator, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXOrbitone_MainOutput:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Orbitone, MixerOutput::MainOutput, nValue);
		this->m_FXSpinLock.Release();
		break;

	// Phaser Send parameters
	case TParameter::ParameterFXPhaser_TubeSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Phaser, MixerOutput::FX_Tube, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXPhaser_ChorusSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Phaser, MixerOutput::FX_Chorus, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXPhaser_FlangerSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Phaser, MixerOutput::FX_Flanger, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXPhaser_OrbitoneSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Phaser, MixerOutput::FX_Orbitone, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXPhaser_DelaySend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Phaser, MixerOutput::FX_Delay, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXPhaser_PlateReverbSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Phaser, MixerOutput::FX_PlateReverb, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXPhaser_ReverberatorSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Phaser, MixerOutput::FX_Reverberator, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXPhaser_MainOutput:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Phaser, MixerOutput::MainOutput, nValue);
		this->m_FXSpinLock.Release();
		break;

	// Delay Send parameters
	case TParameter::ParameterFXDelay_TubeSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Delay, MixerOutput::FX_Tube, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXDelay_ChorusSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Delay, MixerOutput::FX_Chorus, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXDelay_FlangerSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Delay, MixerOutput::FX_Flanger, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXDelay_OrbitoneSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Delay, MixerOutput::FX_Orbitone, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXDelay_PhaserSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Delay, MixerOutput::FX_Phaser, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXDelay_PlateReverbSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Delay, MixerOutput::FX_PlateReverb, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXDelay_ReverberatorSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Delay, MixerOutput::FX_Reverberator, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXDelay_MainOutput:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Delay, MixerOutput::MainOutput, nValue);
		this->m_FXSpinLock.Release();
		break;

	// Reverb Send parameters
	case TParameter::ParameterFXPlateReverb_TubeSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_PlateReverb, MixerOutput::FX_Tube, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXPlateReverb_ChorusSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_PlateReverb, MixerOutput::FX_Chorus, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXPlateReverb_FlangerSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_PlateReverb, MixerOutput::FX_Flanger, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXPlateReverb_OrbitoneSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_PlateReverb, MixerOutput::FX_Orbitone, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXPlateReverb_PhaserSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_PlateReverb, MixerOutput::FX_Phaser, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXPlateReverb_DelaySend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_PlateReverb, MixerOutput::FX_Delay, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXPlateReverb_ReverberatorSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_PlateReverb, MixerOutput::FX_Reverberator, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXPlateReverb_MainOutput:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_PlateReverb, MixerOutput::MainOutput, nValue);
		this->m_FXSpinLock.Release();
		break;

	// Reverberator Send parameters
	case TParameter::ParameterFXReverberator_TubeSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Reverberator, MixerOutput::FX_Tube, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXReverberator_ChorusSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Reverberator, MixerOutput::FX_Chorus, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXReverberator_FlangerSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Reverberator, MixerOutput::FX_Flanger, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXReverberator_OrbitoneSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Reverberator, MixerOutput::FX_Orbitone, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXReverberator_PhaserSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Reverberator, MixerOutput::FX_Phaser, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXReverberator_DelaySend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Reverberator, MixerOutput::FX_Delay, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXReverberator_PlateReverbSend:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Reverberator, MixerOutput::FX_PlateReverb, nValue);
		this->m_FXSpinLock.Release();
		break;
	case TParameter::ParameterFXReverberator_MainOutput:
		nValue = constrain((int)nValue, 0, 99);
		this->m_FXSpinLock.Acquire();
		this->setMixingConsoleFXSendLevel(MixerOutput::FX_Reverberator, MixerOutput::MainOutput, nValue);
		this->m_FXSpinLock.Release();
		break;

	case TParameter::ParameterFXBypass:
		this->m_FXSpinLock.Acquire();
		this->mixing_console_->bypass(!!nValue);
		this->m_FXSpinLock.Release();
		break;

#elif defined(PLATE_REVERB_ENABLE)

	case TParameter::ParameterReverbEnable:
		nValue=constrain((int)nValue,0,1);
		m_FXSpinLock.Acquire ();
		reverb->set_bypass (!nValue);
		m_FXSpinLock.Release ();
		break;

	case TParameter::ParameterReverbSize:
		nValue=constrain((int)nValue,0,99);
		m_FXSpinLock.Acquire ();
		reverb->size (nValue / 99.0f);
		m_FXSpinLock.Release ();
		break;

	case TParameter::ParameterReverbHighDamp:
		nValue=constrain((int)nValue,0,99);
		m_FXSpinLock.Acquire ();
		reverb->hidamp (nValue / 99.0f);
		m_FXSpinLock.Release ();
		break;

	case TParameter::ParameterReverbLowDamp:
		nValue=constrain((int)nValue,0,99);
		m_FXSpinLock.Acquire ();
		reverb->lodamp (nValue / 99.0f);
		m_FXSpinLock.Release ();
		break;

	case TParameter::ParameterReverbLowPass:
		nValue=constrain((int)nValue,0,99);
		m_FXSpinLock.Acquire ();
		reverb->lowpass (nValue / 99.0f);
		m_FXSpinLock.Release ();
		break;

	case TParameter::ParameterReverbDiffusion:
		nValue=constrain((int)nValue,0,99);
		m_FXSpinLock.Acquire ();
		reverb->diffusion (nValue / 99.0f);
		m_FXSpinLock.Release ();
		break;

	case TParameter::ParameterReverbLevel:
		nValue=constrain((int)nValue,0,99);
		m_FXSpinLock.Acquire ();
		reverb->level (nValue / 99.0f);
		m_FXSpinLock.Release ();
		break;

#endif

	default:
		assert (0);
		break;
	}
}

int CMiniDexed::GetParameter (TParameter Parameter)
{
	assert(Parameter < TParameter::ParameterUnknown);

	return m_nParameter[Parameter];
}

void CMiniDexed::SetTGParameter (TTGParameter Parameter, int nValue, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);

	switch (Parameter)
	{
	case TTGParameter::TGParameterVoiceBank:			this->BankSelect (nValue, nTG);	break;
	case TTGParameter::TGParameterVoiceBankMSB:			this->BankSelectMSB (nValue, nTG); break;
	case TTGParameter::TGParameterVoiceBankLSB:			this->BankSelectLSB (nValue, nTG); break;
	case TTGParameter::TGParameterProgram:				this->ProgramChange (nValue, nTG); break;
	case TTGParameter::TGParameterVolume:				this->SetVolume (nValue, nTG); break;
	case TTGParameter::TGParameterPan:					this->SetPan (nValue, nTG); break;
	case TTGParameter::TGParameterMasterTune:			this->SetMasterTune (nValue, nTG); break;
	case TTGParameter::TGParameterCutoff:				this->SetCutoff (nValue, nTG); break;
	case TTGParameter::TGParameterResonance:			this->SetResonance (nValue, nTG); break;
	case TTGParameter::TGParameterPitchBendRange:		this->setPitchbendRange (nValue, nTG); break;
	case TTGParameter::TGParameterPitchBendStep:		this->setPitchbendStep (nValue, nTG); break;
	case TTGParameter::TGParameterPortamentoMode:		this->setPortamentoMode (nValue, nTG); break;
	case TTGParameter::TGParameterPortamentoGlissando:	this->setPortamentoGlissando (nValue, nTG); break;
	case TTGParameter::TGParameterPortamentoTime:		this->setPortamentoTime (nValue, nTG); break;
	case TTGParameter::TGParameterMonoMode:				this->setMonoMode (nValue , nTG); break; 
	
	case TTGParameter::TGParameterMWRange:				this->setModController(0, 0, nValue, nTG); break;
	case TTGParameter::TGParameterMWPitch:				this->setModController(0, 1, nValue, nTG); break;
	case TTGParameter::TGParameterMWAmplitude:			this->setModController(0, 2, nValue, nTG); break;
	case TTGParameter::TGParameterMWEGBias:				this->setModController(0, 3, nValue, nTG); break;
	
	case TTGParameter::TGParameterFCRange:				this->setModController(1, 0, nValue, nTG); break;
	case TTGParameter::TGParameterFCPitch:				this->setModController(1, 1, nValue, nTG); break;
	case TTGParameter::TGParameterFCAmplitude:			this->setModController(1, 2, nValue, nTG); break;
	case TTGParameter::TGParameterFCEGBias:				this->setModController(1, 3, nValue, nTG); break;
	
	case TTGParameter::TGParameterBCRange:				this->setModController(2, 0, nValue, nTG); break;
	case TTGParameter::TGParameterBCPitch:				this->setModController(2, 1, nValue, nTG); break;
	case TTGParameter::TGParameterBCAmplitude:			this->setModController(2, 2, nValue, nTG); break;
	case TTGParameter::TGParameterBCEGBias:				this->setModController(2, 3, nValue, nTG); break;
	
	case TTGParameter::TGParameterATRange:				this->setModController(3, 0, nValue, nTG); break;
	case TTGParameter::TGParameterATPitch:				this->setModController(3, 1, nValue, nTG); break;
	case TTGParameter::TGParameterATAmplitude:			this->setModController(3, 2, nValue, nTG); break;
	case TTGParameter::TGParameterATEGBias:				this->setModController(3, 3, nValue, nTG); break;
	
	case TTGParameter::TGParameterMIDIChannel:
		assert (0 <= nValue && nValue <= 255);
		SetMIDIChannel ((uint8_t) nValue, nTG);
		break;

#if defined(MIXING_CONSOLE_ENABLE)
	case TTGParameter::TGParameterMixingSendFXTube:			this->setMixingConsoleSendLevel(nTG, MixerOutput::FX_Tube, 			nValue); break;
	case TTGParameter::TGParameterMixingSendFXChorus:		this->setMixingConsoleSendLevel(nTG, MixerOutput::FX_Chorus, 		nValue); break;
	case TTGParameter::TGParameterMixingSendFXFlanger:		this->setMixingConsoleSendLevel(nTG, MixerOutput::FX_Flanger, 		nValue); break;
	case TTGParameter::TGParameterMixingSendFXOrbitone:		this->setMixingConsoleSendLevel(nTG, MixerOutput::FX_Orbitone, 		nValue); break;
	case TTGParameter::TGParameterMixingSendFXPhaser:		this->setMixingConsoleSendLevel(nTG, MixerOutput::FX_Phaser, 		nValue); break;
	case TTGParameter::TGParameterMixingSendFXDelay:		this->setMixingConsoleSendLevel(nTG, MixerOutput::FX_Delay, 		nValue); break;
	case TTGParameter::TGParameterMixingSendFXPlateReverb:	this->setMixingConsoleSendLevel(nTG, MixerOutput::FX_PlateReverb, 	nValue); break;
	case TTGParameter::TGParameterMixingSendFXReverberator:	this->setMixingConsoleSendLevel(nTG, MixerOutput::FX_Reverberator, 	nValue); break;
	case TTGParameter::TGParameterMixingSendFXMainOutput:	this->setMixingConsoleSendLevel(nTG, MixerOutput::MainOutput, 		nValue); break;
#elif defined(PLATE_REVERB_ENABLE)
	case TTGParameter::TGParameterReverbSend:				this->SetReverbSend (nValue, nTG); break;
#endif // MIXING_CONSOLE_ENABLE

	default:
		assert (0);
		break;
	}

	this->m_UI.ParameterChanged();
}

int CMiniDexed::GetTGParameter (TTGParameter Parameter, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);

	switch (Parameter)
	{
	case TTGParameter::TGParameterVoiceBank:				return m_nVoiceBankID[nTG];
	case TTGParameter::TGParameterVoiceBankMSB:				return m_nVoiceBankID[nTG] >> 7;
	case TTGParameter::TGParameterVoiceBankLSB:				return m_nVoiceBankID[nTG] & 0x7F;
	case TTGParameter::TGParameterProgram:					return m_nProgram[nTG];
	case TTGParameter::TGParameterVolume:					return m_nVolume[nTG];
	case TTGParameter::TGParameterPan:						return m_nPan[nTG];
	case TTGParameter::TGParameterMasterTune:				return m_nMasterTune[nTG];
	case TTGParameter::TGParameterCutoff:					return m_nCutoff[nTG];
	case TTGParameter::TGParameterResonance:				return m_nResonance[nTG];
	case TTGParameter::TGParameterMIDIChannel:				return m_nMIDIChannel[nTG];
#if defined(MIXING_CONSOLE_ENABLE)
	case TTGParameter::TGParameterMixingSendFXTube:			return this->getMixingConsoleSendLevel(nTG, MixerOutput::FX_Tube);
	case TTGParameter::TGParameterMixingSendFXChorus:		return this->getMixingConsoleSendLevel(nTG, MixerOutput::FX_Chorus);
	case TTGParameter::TGParameterMixingSendFXFlanger:		return this->getMixingConsoleSendLevel(nTG, MixerOutput::FX_Flanger);
	case TTGParameter::TGParameterMixingSendFXOrbitone:		return this->getMixingConsoleSendLevel(nTG, MixerOutput::FX_Orbitone);
	case TTGParameter::TGParameterMixingSendFXPhaser:		return this->getMixingConsoleSendLevel(nTG, MixerOutput::FX_Phaser);
	case TTGParameter::TGParameterMixingSendFXDelay:		return this->getMixingConsoleSendLevel(nTG, MixerOutput::FX_Delay);
	case TTGParameter::TGParameterMixingSendFXPlateReverb:	return this->getMixingConsoleSendLevel(nTG, MixerOutput::FX_PlateReverb);
	case TTGParameter::TGParameterMixingSendFXReverberator:	return this->getMixingConsoleSendLevel(nTG, MixerOutput::FX_Reverberator);
	case TTGParameter::TGParameterMixingSendFXMainOutput:	return this->getMixingConsoleSendLevel(nTG, MixerOutput::MainOutput);
#elif defined(PLATE_REVERB_ENABLE)
	case TTGParameter::TGParameterReverbSend:				return m_nReverbSend[nTG];
#endif
	case TTGParameter::TGParameterPitchBendRange:			return m_nPitchBendRange[nTG];
	case TTGParameter::TGParameterPitchBendStep:			return m_nPitchBendStep[nTG];
	case TTGParameter::TGParameterPortamentoMode:			return m_nPortamentoMode[nTG];
	case TTGParameter::TGParameterPortamentoGlissando:		return m_nPortamentoGlissando[nTG];
	case TTGParameter::TGParameterPortamentoTime:			return m_nPortamentoTime[nTG];
	case TTGParameter::TGParameterMonoMode:					return m_bMonoMode[nTG] ? 1 : 0; 
	
	case TTGParameter::TGParameterMWRange:					return getModController(0, 0, nTG);
	case TTGParameter::TGParameterMWPitch:					return getModController(0, 1, nTG);
	case TTGParameter::TGParameterMWAmplitude:				return getModController(0, 2, nTG); 
	case TTGParameter::TGParameterMWEGBias:					return getModController(0, 3, nTG); 
	
	case TTGParameter::TGParameterFCRange:					return getModController(1, 0,  nTG); 
	case TTGParameter::TGParameterFCPitch:					return getModController(1, 1,  nTG); 
	case TTGParameter::TGParameterFCAmplitude:				return getModController(1, 2,  nTG); 
	case TTGParameter::TGParameterFCEGBias:					return getModController(1, 3,  nTG); 
	
	case TTGParameter::TGParameterBCRange:					return getModController(2, 0,  nTG); 
	case TTGParameter::TGParameterBCPitch:					return getModController(2, 1,  nTG); 
	case TTGParameter::TGParameterBCAmplitude:				return getModController(2, 2,  nTG); 
	case TTGParameter::TGParameterBCEGBias:					return getModController(2, 3,  nTG); 
	
	case TTGParameter::TGParameterATRange:					return getModController(3, 0,  nTG); 
	case TTGParameter::TGParameterATPitch:					return getModController(3, 1,  nTG); 
	case TTGParameter::TGParameterATAmplitude:				return getModController(3, 2,  nTG); 
	case TTGParameter::TGParameterATEGBias:					return getModController(3, 3,  nTG); 
		
	default:
		assert (0);
		return 0;
	}
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

		float32_t SampleBuffer[nFrames];
		m_pTG[0]->getSamples (SampleBuffer, nFrames);

		// Convert single float array (mono) to int16 array
		int16_t tmp_int[nFrames];
		arm_float_to_q15(SampleBuffer,tmp_int,nFrames);

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
			m_pTG[i]->getSamples (m_OutputLevel[i], nFrames);

#if defined(MIXING_CONSOLE_ENABLE)
			this->mixing_console_->preProcessInputSampleBuffer(i, nFrames);
#endif
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
		int16_t tmp_int[nFrames * 2];

#if defined(MIXING_CONSOLE_ENABLE)
		// BEGIN mixing
		if(this->nMasterVolume > 0.0f)
		{
			// temp buffering and channel indexing
			float32_t interlacedSampleBuffer[nFrames << 1];

			this->m_FXSpinLock.Acquire();
			this->mixing_console_->process(interlacedSampleBuffer);
			this->m_FXSpinLock.Release();

			if(this->nMasterVolume < 1.0f)
			{
				arm_scale_f32(interlacedSampleBuffer, this->nMasterVolume, interlacedSampleBuffer, nFrames << 1);
			}
			
			// Convert float array (left, right) to single int16 array (left/right)
			arm_float_to_q15(interlacedSampleBuffer, tmp_int, nFrames << 1);
		}
		else // this->nMasterVolume == 0.0f
		{
			arm_fill_q15(0, tmp_int, nFrames << 1);
		}

#elif defined(PLATE_REVERB_ENABLE)

		uint8_t indexL=0, indexR=1;
		
		// BEGIN TG mixing
		float32_t tmp_float[nFrames*2];

		if(nMasterVolume > 0.0f)
		{
			for (uint8_t i = 0; i < CConfig::ToneGenerators; i++)
			{
				tg_mixer->doAddMix(i,m_OutputLevel[i]);
				reverb_send_mixer->doAddMix(i,m_OutputLevel[i]);
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
	
				m_FXSpinLock.Acquire ();
	
				reverb_send_mixer->getMix(ReverbSendBuffer[indexL], ReverbSendBuffer[indexR]);
				reverb->doReverb(ReverbSendBuffer[indexL],ReverbSendBuffer[indexR],ReverbBuffer[indexL], ReverbBuffer[indexR],nFrames);
	
				// scale down and add left reverb buffer by reverb level 
				arm_scale_f32(ReverbBuffer[indexL], reverb->get_level(), ReverbBuffer[indexL], nFrames);
				arm_add_f32(SampleBuffer[indexL], ReverbBuffer[indexL], SampleBuffer[indexL], nFrames);
				// scale down and add right reverb buffer by reverb level 
				arm_scale_f32(ReverbBuffer[indexR], reverb->get_level(), ReverbBuffer[indexR], nFrames);
				arm_add_f32(SampleBuffer[indexR], ReverbBuffer[indexR], SampleBuffer[indexR], nFrames);
	
				m_FXSpinLock.Release ();
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
#endif

		if(this->m_pSoundDevice->Write(tmp_int, sizeof(tmp_int)) != (int)sizeof(tmp_int))
		{
			LOGERR ("Sound data dropped");
		}

		if(this->m_bProfileEnabled)
		{
			this->m_GetChunkTimer.Stop ();
		}
	}
}

#endif

bool CMiniDexed::SavePerformance (bool bSaveAsDeault)
{
	m_bSavePerformance = true;
	m_bSaveAsDeault=bSaveAsDeault;

	return true;
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

#if defined(MIXING_CONSOLE_ENABLE)
		for(size_t fx = 0; fx < MixerOutput::kFXCount; ++fx)
		{
			this->m_PerformanceConfig.SetTGSendLevel(nTG, static_cast<MixerOutput>(fx), this->m_nTGSendLevel[nTG][fx]);
		}
#endif

#if defined(PLATE_REVERB_ENABLE)
		m_PerformanceConfig.SetReverbSend (m_nReverbSend[nTG], nTG);
#endif
	}

	m_PerformanceConfig.SetCompressorEnable (!!m_nParameter[TParameter::ParameterCompressorEnable]);
#if defined(MIXING_CONSOLE_ENABLE) || defined(PLATE_REVERB_ENABLE) 
	m_PerformanceConfig.SetReverbEnable (!!m_nParameter[TParameter::ParameterReverbEnable]);
	m_PerformanceConfig.SetReverbSize (m_nParameter[TParameter::ParameterReverbSize]);
	m_PerformanceConfig.SetReverbHighDamp (m_nParameter[TParameter::ParameterReverbHighDamp]);
	m_PerformanceConfig.SetReverbLowDamp (m_nParameter[TParameter::ParameterReverbLowDamp]);
	m_PerformanceConfig.SetReverbLowPass (m_nParameter[TParameter::ParameterReverbLowPass]);
	m_PerformanceConfig.SetReverbDiffusion (m_nParameter[TParameter::ParameterReverbDiffusion]);
	m_PerformanceConfig.SetReverbLevel (m_nParameter[TParameter::ParameterReverbLevel]);
#endif

#ifdef MIXING_CONSOLE_ENABLE
	this->m_PerformanceConfig.SetFXTubeEnable(!!this->m_nParameter[TParameter::ParameterFXTubeEnable]);
	this->m_PerformanceConfig.SetFXTubeOverdrive(this->m_nParameter[TParameter::ParameterFXTubeOverdrive]);

	this->m_PerformanceConfig.SetFXChorusEnable(!!this->m_nParameter[TParameter::ParameterFXChorusEnable]);
	this->m_PerformanceConfig.SetFXChorusRate(this->m_nParameter[TParameter::ParameterFXChorusRate]);
	this->m_PerformanceConfig.SetFXChorusDepth(this->m_nParameter[TParameter::ParameterFXChorusDepth]);

	this->m_PerformanceConfig.SetFXFlangerEnable(!!this->m_nParameter[TParameter::ParameterFXFlangerEnable]);
	this->m_PerformanceConfig.SetFXFlangerRate(this->m_nParameter[TParameter::ParameterFXFlangerRate]);
	this->m_PerformanceConfig.SetFXFlangerDepth(this->m_nParameter[TParameter::ParameterFXFlangerDepth]);
	this->m_PerformanceConfig.SetFXFlangerFeedback(this->m_nParameter[TParameter::ParameterFXFlangerFeedback]);

	this->m_PerformanceConfig.SetFXOrbitoneEnable(!!this->m_nParameter[TParameter::ParameterFXOrbitoneEnable]);
	this->m_PerformanceConfig.SetFXOrbitoneRate(this->m_nParameter[TParameter::ParameterFXOrbitoneRate]);
	this->m_PerformanceConfig.SetFXOrbitoneDepth(this->m_nParameter[TParameter::ParameterFXOrbitoneDepth]);

	this->m_PerformanceConfig.SetFXPhaserEnable(!!this->m_nParameter[TParameter::ParameterFXPhaserEnable]);
	this->m_PerformanceConfig.SetFXPhaserRate(this->m_nParameter[TParameter::ParameterFXPhaserRate]);
	this->m_PerformanceConfig.SetFXPhaserDepth(this->m_nParameter[TParameter::ParameterFXPhaserDepth]);
	this->m_PerformanceConfig.SetFXPhaserFeedback(this->m_nParameter[TParameter::ParameterFXPhaserFeedback]);
	this->m_PerformanceConfig.SetFXPhaserNbStages(this->m_nParameter[TParameter::ParameterFXPhaserNbStages]);

	this->m_PerformanceConfig.SetFXDelayEnable(!!this->m_nParameter[TParameter::ParameterFXDelayEnable]);
	this->m_PerformanceConfig.SetFXDelayLeftDelayTime(this->m_nParameter[TParameter::ParameterFXDelayLeftDelayTime]);
	this->m_PerformanceConfig.SetFXDelayRightDelayTime(this->m_nParameter[TParameter::ParameterFXDelayRightDelayTime]);
	this->m_PerformanceConfig.SetFXDelayFeedback(this->m_nParameter[TParameter::ParameterFXDelayFeedback]);

	this->m_PerformanceConfig.SetFXReverberatorEnable(!!this->m_nParameter[TParameter::ParameterFXReverberatorEnable]);
	this->m_PerformanceConfig.SetFXReverberatorInputGain(this->m_nParameter[TParameter::ParameterFXReverberatorInputGain]);
	this->m_PerformanceConfig.SetFXReverberatorTime(this->m_nParameter[TParameter::ParameterFXReverberatorTime]);
	this->m_PerformanceConfig.SetFXReverberatorDiffusion(this->m_nParameter[TParameter::ParameterFXReverberatorDiffusion]);
	this->m_PerformanceConfig.SetFXReverberatorLP(this->m_nParameter[TParameter::ParameterFXReverberatorLP]);

	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Tube, MixerOutput::FX_Chorus, this->m_nParameter[TParameter::ParameterFXTube_ChorusSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Tube, MixerOutput::FX_Flanger, this->m_nParameter[TParameter::ParameterFXTube_FlangerSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Tube, MixerOutput::FX_Orbitone, this->m_nParameter[TParameter::ParameterFXTube_OrbitoneSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Tube, MixerOutput::FX_Phaser, this->m_nParameter[TParameter::ParameterFXTube_PhaserSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Tube, MixerOutput::FX_Delay, this->m_nParameter[TParameter::ParameterFXTube_DelaySend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Tube, MixerOutput::FX_PlateReverb, this->m_nParameter[TParameter::ParameterFXTube_PlateReverbSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Tube, MixerOutput::FX_Reverberator, this->m_nParameter[TParameter::ParameterFXTube_ReverberatorSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Tube, MixerOutput::MainOutput, this->m_nParameter[TParameter::ParameterFXTube_MainOutput]);

	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Chorus, MixerOutput::FX_Tube, this->m_nParameter[TParameter::ParameterFXChorus_TubeSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Chorus, MixerOutput::FX_Flanger, this->m_nParameter[TParameter::ParameterFXChorus_FlangerSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Chorus, MixerOutput::FX_Orbitone, this->m_nParameter[TParameter::ParameterFXChorus_OrbitoneSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Chorus, MixerOutput::FX_Phaser, this->m_nParameter[TParameter::ParameterFXChorus_PhaserSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Chorus, MixerOutput::FX_Delay, this->m_nParameter[TParameter::ParameterFXChorus_DelaySend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Chorus, MixerOutput::FX_PlateReverb, this->m_nParameter[TParameter::ParameterFXChorus_PlateReverbSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Chorus, MixerOutput::FX_Reverberator, this->m_nParameter[TParameter::ParameterFXChorus_ReverberatorSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Chorus, MixerOutput::MainOutput, this->m_nParameter[TParameter::ParameterFXChorus_MainOutput]);

	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Flanger, MixerOutput::FX_Tube, this->m_nParameter[TParameter::ParameterFXFlanger_TubeSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Flanger, MixerOutput::FX_Chorus, this->m_nParameter[TParameter::ParameterFXFlanger_ChorusSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Flanger, MixerOutput::FX_Orbitone, this->m_nParameter[TParameter::ParameterFXFlanger_OrbitoneSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Flanger, MixerOutput::FX_Phaser, this->m_nParameter[TParameter::ParameterFXFlanger_PhaserSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Flanger, MixerOutput::FX_Delay, this->m_nParameter[TParameter::ParameterFXFlanger_DelaySend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Flanger, MixerOutput::FX_PlateReverb, this->m_nParameter[TParameter::ParameterFXFlanger_PlateReverbSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Flanger, MixerOutput::FX_Reverberator, this->m_nParameter[TParameter::ParameterFXFlanger_ReverberatorSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Flanger, MixerOutput::MainOutput, this->m_nParameter[TParameter::ParameterFXFlanger_MainOutput]);

	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Orbitone, MixerOutput::FX_Tube, this->m_nParameter[TParameter::ParameterFXOrbitone_TubeSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Orbitone, MixerOutput::FX_Chorus, this->m_nParameter[TParameter::ParameterFXOrbitone_ChorusSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Orbitone, MixerOutput::FX_Flanger, this->m_nParameter[TParameter::ParameterFXOrbitone_FlangerSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Orbitone, MixerOutput::FX_Phaser, this->m_nParameter[TParameter::ParameterFXOrbitone_PhaserSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Orbitone, MixerOutput::FX_Delay, this->m_nParameter[TParameter::ParameterFXOrbitone_DelaySend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Orbitone, MixerOutput::FX_PlateReverb, this->m_nParameter[TParameter::ParameterFXOrbitone_PlateReverbSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Orbitone, MixerOutput::FX_Reverberator, this->m_nParameter[TParameter::ParameterFXOrbitone_ReverberatorSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Orbitone, MixerOutput::MainOutput, this->m_nParameter[TParameter::ParameterFXOrbitone_MainOutput]);

	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Phaser, MixerOutput::FX_Tube, this->m_nParameter[TParameter::ParameterFXPhaser_TubeSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Phaser, MixerOutput::FX_Chorus, this->m_nParameter[TParameter::ParameterFXPhaser_ChorusSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Phaser, MixerOutput::FX_Flanger, this->m_nParameter[TParameter::ParameterFXPhaser_FlangerSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Phaser, MixerOutput::FX_Orbitone, this->m_nParameter[TParameter::ParameterFXPhaser_OrbitoneSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Phaser, MixerOutput::FX_Delay, this->m_nParameter[TParameter::ParameterFXPhaser_DelaySend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Phaser, MixerOutput::FX_PlateReverb, this->m_nParameter[TParameter::ParameterFXPhaser_PlateReverbSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Phaser, MixerOutput::FX_Reverberator, this->m_nParameter[TParameter::ParameterFXPhaser_ReverberatorSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Phaser, MixerOutput::MainOutput, this->m_nParameter[TParameter::ParameterFXPhaser_MainOutput]);

	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Delay, MixerOutput::FX_Tube, this->m_nParameter[TParameter::ParameterFXDelay_TubeSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Delay, MixerOutput::FX_Chorus, this->m_nParameter[TParameter::ParameterFXDelay_ChorusSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Delay, MixerOutput::FX_Flanger, this->m_nParameter[TParameter::ParameterFXDelay_FlangerSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Delay, MixerOutput::FX_Orbitone, this->m_nParameter[TParameter::ParameterFXDelay_OrbitoneSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Delay, MixerOutput::FX_Phaser, this->m_nParameter[TParameter::ParameterFXDelay_PhaserSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Delay, MixerOutput::FX_PlateReverb, this->m_nParameter[TParameter::ParameterFXDelay_PlateReverbSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Delay, MixerOutput::FX_Reverberator, this->m_nParameter[TParameter::ParameterFXDelay_ReverberatorSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Delay, MixerOutput::MainOutput, this->m_nParameter[TParameter::ParameterFXDelay_MainOutput]);

	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_PlateReverb, MixerOutput::FX_Tube, this->m_nParameter[TParameter::ParameterFXPlateReverb_TubeSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_PlateReverb, MixerOutput::FX_Chorus, this->m_nParameter[TParameter::ParameterFXPlateReverb_ChorusSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_PlateReverb, MixerOutput::FX_Flanger, this->m_nParameter[TParameter::ParameterFXPlateReverb_FlangerSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_PlateReverb, MixerOutput::FX_Orbitone, this->m_nParameter[TParameter::ParameterFXPlateReverb_OrbitoneSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_PlateReverb, MixerOutput::FX_Phaser, this->m_nParameter[TParameter::ParameterFXPlateReverb_PhaserSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_PlateReverb, MixerOutput::FX_Delay, this->m_nParameter[TParameter::ParameterFXPlateReverb_DelaySend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_PlateReverb, MixerOutput::FX_Reverberator, this->m_nParameter[TParameter::ParameterFXPlateReverb_ReverberatorSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_PlateReverb, MixerOutput::MainOutput, this->m_nParameter[TParameter::ParameterFXPlateReverb_MainOutput]);

	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Reverberator, MixerOutput::FX_Tube, this->m_nParameter[TParameter::ParameterFXReverberator_TubeSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Reverberator, MixerOutput::FX_Chorus, this->m_nParameter[TParameter::ParameterFXReverberator_ChorusSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Reverberator, MixerOutput::FX_Flanger, this->m_nParameter[TParameter::ParameterFXReverberator_FlangerSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Reverberator, MixerOutput::FX_Orbitone, this->m_nParameter[TParameter::ParameterFXReverberator_OrbitoneSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Reverberator, MixerOutput::FX_Phaser, this->m_nParameter[TParameter::ParameterFXReverberator_PhaserSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Reverberator, MixerOutput::FX_Delay, this->m_nParameter[TParameter::ParameterFXReverberator_DelaySend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Reverberator, MixerOutput::FX_PlateReverb, this->m_nParameter[TParameter::ParameterFXReverberator_PlateReverbSend]);
	this->m_PerformanceConfig.SetFXSendLevel(MixerOutput::FX_Reverberator, MixerOutput::MainOutput, this->m_nParameter[TParameter::ParameterFXReverberator_MainOutput]);

	this->m_PerformanceConfig.SetFXBypass(this->mixing_console_->bypass());

#endif

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
	this->nMasterVolume = constrain(vol, 0.0f, 1.0f);
			
				   
			

				   
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



unsigned CMiniDexed::GetActualPerformanceID()
{
	return m_PerformanceConfig.GetActualPerformanceID();
}

void CMiniDexed::SetActualPerformanceID(unsigned nID)
{
	m_PerformanceConfig.SetActualPerformanceID(nID);
}

bool CMiniDexed::SetNewPerformance(unsigned nID)
{
	m_bSetNewPerformance = true;
	m_nSetNewPerformanceID = nID;

	return true;
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

		m_nNoteLimitLow[nTG] = m_PerformanceConfig.GetNoteLimitLow (nTG);
		m_nNoteLimitHigh[nTG] = m_PerformanceConfig.GetNoteLimitHigh (nTG);
		m_nNoteShift[nTG] = m_PerformanceConfig.GetNoteShift (nTG);
   
												
	
																	  
												
	
																   
																
	 
																			  
																				
																				
																				  
																					
																					  
																		   
																			 
   
		
		if(m_PerformanceConfig.VoiceDataFilled(nTG)) 
		{
		uint8_t* tVoiceData = m_PerformanceConfig.GetVoiceDataFromTxt(nTG);
		m_pTG[nTG]->loadVoiceParameters(tVoiceData); 
		}
		setMonoMode(m_PerformanceConfig.GetMonoMode(nTG) ? 1 : 0, nTG); 
			
		this->SetParameter(TParameter::ParameterCompressorEnable, this->m_PerformanceConfig.GetCompressorEnable());
#if defined(MIXING_CONSOLE_ENABLE)
		for(size_t fx = 0; fx < MixerOutput::kFXCount; ++fx)
		{
			this->setMixingConsoleSendLevel(nTG, static_cast<MixerOutput>(fx), this->m_PerformanceConfig.GetTGSendLevel(nTG, static_cast<MixerOutput>(fx)));
		}
#elif defined(PLATE_REVERB_ENABLE)
		SetReverbSend (m_PerformanceConfig.GetReverbSend (nTG), nTG);
#endif

		setModWheelRange (m_PerformanceConfig.GetModulationWheelRange (nTG),  nTG);
		setModWheelTarget (m_PerformanceConfig.GetModulationWheelTarget (nTG),  nTG);
		setFootControllerRange (m_PerformanceConfig.GetFootControlRange (nTG),  nTG);
		setFootControllerTarget (m_PerformanceConfig.GetFootControlTarget (nTG),  nTG);
		setBreathControllerRange (m_PerformanceConfig.GetBreathControlRange (nTG),  nTG);
		setBreathControllerTarget (m_PerformanceConfig.GetBreathControlTarget (nTG),  nTG);
		setAftertouchRange (m_PerformanceConfig.GetAftertouchRange (nTG),  nTG);
		setAftertouchTarget (m_PerformanceConfig.GetAftertouchTarget (nTG),  nTG);
	}

#ifdef MIXING_CONSOLE_ENABLE
	this->SetParameter(TParameter::ParameterFXTubeEnable, this->m_PerformanceConfig.GetFXTubeEnable());
	this->SetParameter(TParameter::ParameterFXTubeOverdrive, this->m_PerformanceConfig.GetFXTubeOverdrive());

	this->SetParameter(TParameter::ParameterFXChorusEnable, this->m_PerformanceConfig.GetFXChorusEnable());
	this->SetParameter(TParameter::ParameterFXChorusRate, this->m_PerformanceConfig.GetFXChorusRate());
	this->SetParameter(TParameter::ParameterFXChorusDepth, this->m_PerformanceConfig.GetFXChorusDepth());

	this->SetParameter(TParameter::ParameterFXFlangerEnable, this->m_PerformanceConfig.GetFXFlangerEnable());
	this->SetParameter(TParameter::ParameterFXFlangerRate, this->m_PerformanceConfig.GetFXFlangerRate());
	this->SetParameter(TParameter::ParameterFXFlangerDepth, this->m_PerformanceConfig.GetFXFlangerDepth());
	this->SetParameter(TParameter::ParameterFXFlangerFeedback, this->m_PerformanceConfig.GetFXFlangerFeedback());

	this->SetParameter(TParameter::ParameterFXOrbitoneEnable, this->m_PerformanceConfig.GetFXOrbitoneEnable());
	this->SetParameter(TParameter::ParameterFXOrbitoneRate, this->m_PerformanceConfig.GetFXOrbitoneRate());
	this->SetParameter(TParameter::ParameterFXOrbitoneDepth, this->m_PerformanceConfig.GetFXOrbitoneDepth());

	this->SetParameter(TParameter::ParameterFXPhaserEnable, this->m_PerformanceConfig.GetFXPhaserEnable());
	this->SetParameter(TParameter::ParameterFXPhaserRate, this->m_PerformanceConfig.GetFXPhaserRate());
	this->SetParameter(TParameter::ParameterFXPhaserDepth, this->m_PerformanceConfig.GetFXPhaserDepth());
	this->SetParameter(TParameter::ParameterFXPhaserFeedback, this->m_PerformanceConfig.GetFXPhaserFeedback());
	this->SetParameter(TParameter::ParameterFXPhaserNbStages, this->m_PerformanceConfig.GetFXPhaserNbStages());

	this->SetParameter(TParameter::ParameterFXDelayEnable, this->m_PerformanceConfig.GetFXDelayEnable());
	this->SetParameter(TParameter::ParameterFXDelayLeftDelayTime, this->m_PerformanceConfig.GetFXDelayLeftDelayTime());
	this->SetParameter(TParameter::ParameterFXDelayRightDelayTime, this->m_PerformanceConfig.GetFXDelayRightDelayTime());
	this->SetParameter(TParameter::ParameterFXDelayFeedback, this->m_PerformanceConfig.GetFXDelayFeedback());

	this->SetParameter(TParameter::ParameterFXReverberatorEnable, this->m_PerformanceConfig.GetFXReverberatorEnable());
	this->SetParameter(TParameter::ParameterFXReverberatorInputGain, this->m_PerformanceConfig.GetFXReverberatorInputGain());
	this->SetParameter(TParameter::ParameterFXReverberatorTime, this->m_PerformanceConfig.GetFXReverberatorTime());
	this->SetParameter(TParameter::ParameterFXReverberatorDiffusion, this->m_PerformanceConfig.GetFXReverberatorDiffusion());
	this->SetParameter(TParameter::ParameterFXReverberatorLP, this->m_PerformanceConfig.GetFXReverberatorLP());

	this->SetParameter(TParameter::ParameterFXTube_ChorusSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Tube, MixerOutput::FX_Chorus));
	this->SetParameter(TParameter::ParameterFXTube_FlangerSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Tube, MixerOutput::FX_Flanger));
	this->SetParameter(TParameter::ParameterFXTube_OrbitoneSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Tube, MixerOutput::FX_Orbitone));
	this->SetParameter(TParameter::ParameterFXTube_PhaserSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Tube, MixerOutput::FX_Phaser));
	this->SetParameter(TParameter::ParameterFXTube_DelaySend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Tube, MixerOutput::FX_Delay));
	this->SetParameter(TParameter::ParameterFXTube_PlateReverbSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Tube, MixerOutput::FX_PlateReverb));
	this->SetParameter(TParameter::ParameterFXTube_ReverberatorSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Tube, MixerOutput::FX_Reverberator));
	this->SetParameter(TParameter::ParameterFXTube_MainOutput, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Tube, MixerOutput::MainOutput));

	this->SetParameter(TParameter::ParameterFXChorus_TubeSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Chorus, MixerOutput::FX_Tube));
	this->SetParameter(TParameter::ParameterFXChorus_FlangerSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Chorus, MixerOutput::FX_Flanger));
	this->SetParameter(TParameter::ParameterFXChorus_OrbitoneSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Chorus, MixerOutput::FX_Orbitone));
	this->SetParameter(TParameter::ParameterFXChorus_PhaserSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Chorus, MixerOutput::FX_Phaser));
	this->SetParameter(TParameter::ParameterFXChorus_DelaySend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Chorus, MixerOutput::FX_Delay));
	this->SetParameter(TParameter::ParameterFXChorus_PlateReverbSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Chorus, MixerOutput::FX_PlateReverb));
	this->SetParameter(TParameter::ParameterFXChorus_ReverberatorSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Chorus, MixerOutput::FX_Reverberator));
	this->SetParameter(TParameter::ParameterFXChorus_MainOutput, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Chorus, MixerOutput::MainOutput));

	this->SetParameter(TParameter::ParameterFXFlanger_TubeSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Flanger, MixerOutput::FX_Tube));
	this->SetParameter(TParameter::ParameterFXFlanger_ChorusSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Flanger, MixerOutput::FX_Chorus));
	this->SetParameter(TParameter::ParameterFXFlanger_OrbitoneSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Flanger, MixerOutput::FX_Orbitone));
	this->SetParameter(TParameter::ParameterFXFlanger_PhaserSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Flanger, MixerOutput::FX_Phaser));
	this->SetParameter(TParameter::ParameterFXFlanger_DelaySend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Flanger, MixerOutput::FX_Delay));
	this->SetParameter(TParameter::ParameterFXFlanger_PlateReverbSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Flanger, MixerOutput::FX_PlateReverb));
	this->SetParameter(TParameter::ParameterFXFlanger_ReverberatorSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Flanger, MixerOutput::FX_Reverberator));
	this->SetParameter(TParameter::ParameterFXFlanger_MainOutput, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Flanger, MixerOutput::MainOutput));

	this->SetParameter(TParameter::ParameterFXOrbitone_TubeSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Orbitone, MixerOutput::FX_Tube));
	this->SetParameter(TParameter::ParameterFXOrbitone_ChorusSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Orbitone, MixerOutput::FX_Chorus));
	this->SetParameter(TParameter::ParameterFXOrbitone_FlangerSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Orbitone, MixerOutput::FX_Flanger));
	this->SetParameter(TParameter::ParameterFXOrbitone_PhaserSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Orbitone, MixerOutput::FX_Phaser));
	this->SetParameter(TParameter::ParameterFXOrbitone_DelaySend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Orbitone, MixerOutput::FX_Delay));
	this->SetParameter(TParameter::ParameterFXOrbitone_PlateReverbSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Orbitone, MixerOutput::FX_PlateReverb));
	this->SetParameter(TParameter::ParameterFXOrbitone_ReverberatorSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Orbitone, MixerOutput::FX_Reverberator));
	this->SetParameter(TParameter::ParameterFXOrbitone_MainOutput, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Orbitone, MixerOutput::MainOutput));

	this->SetParameter(TParameter::ParameterFXPhaser_TubeSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Phaser, MixerOutput::FX_Tube));
	this->SetParameter(TParameter::ParameterFXPhaser_ChorusSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Phaser, MixerOutput::FX_Chorus));
	this->SetParameter(TParameter::ParameterFXPhaser_FlangerSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Phaser, MixerOutput::FX_Flanger));
	this->SetParameter(TParameter::ParameterFXPhaser_OrbitoneSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Phaser, MixerOutput::FX_Orbitone));
	this->SetParameter(TParameter::ParameterFXPhaser_DelaySend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Phaser, MixerOutput::FX_Delay));
	this->SetParameter(TParameter::ParameterFXPhaser_PlateReverbSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Phaser, MixerOutput::FX_PlateReverb));
	this->SetParameter(TParameter::ParameterFXPhaser_ReverberatorSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Phaser, MixerOutput::FX_Reverberator));
	this->SetParameter(TParameter::ParameterFXPhaser_MainOutput, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Phaser, MixerOutput::MainOutput));

	this->SetParameter(TParameter::ParameterFXDelay_TubeSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Delay, MixerOutput::FX_Tube));
	this->SetParameter(TParameter::ParameterFXDelay_ChorusSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Delay, MixerOutput::FX_Chorus));
	this->SetParameter(TParameter::ParameterFXDelay_FlangerSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Delay, MixerOutput::FX_Flanger));
	this->SetParameter(TParameter::ParameterFXDelay_OrbitoneSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Delay, MixerOutput::FX_Orbitone));
	this->SetParameter(TParameter::ParameterFXDelay_PhaserSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Delay, MixerOutput::FX_Phaser));
	this->SetParameter(TParameter::ParameterFXDelay_PlateReverbSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Delay, MixerOutput::FX_PlateReverb));
	this->SetParameter(TParameter::ParameterFXDelay_ReverberatorSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Delay, MixerOutput::FX_Reverberator));
	this->SetParameter(TParameter::ParameterFXDelay_MainOutput, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Delay, MixerOutput::MainOutput));

	this->SetParameter(TParameter::ParameterFXPlateReverb_TubeSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_PlateReverb, MixerOutput::FX_Tube));
	this->SetParameter(TParameter::ParameterFXPlateReverb_ChorusSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_PlateReverb, MixerOutput::FX_Chorus));
	this->SetParameter(TParameter::ParameterFXPlateReverb_FlangerSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_PlateReverb, MixerOutput::FX_Flanger));
	this->SetParameter(TParameter::ParameterFXPlateReverb_OrbitoneSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_PlateReverb, MixerOutput::FX_Orbitone));
	this->SetParameter(TParameter::ParameterFXPlateReverb_PhaserSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_PlateReverb, MixerOutput::FX_Phaser));
	this->SetParameter(TParameter::ParameterFXPlateReverb_DelaySend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_PlateReverb, MixerOutput::FX_Delay));
	this->SetParameter(TParameter::ParameterFXPlateReverb_ReverberatorSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_PlateReverb, MixerOutput::FX_Reverberator));
	this->SetParameter(TParameter::ParameterFXPlateReverb_MainOutput, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_PlateReverb, MixerOutput::MainOutput));

	this->SetParameter(TParameter::ParameterFXReverberator_TubeSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Reverberator, MixerOutput::FX_Tube));
	this->SetParameter(TParameter::ParameterFXReverberator_ChorusSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Reverberator, MixerOutput::FX_Chorus));
	this->SetParameter(TParameter::ParameterFXReverberator_FlangerSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Reverberator, MixerOutput::FX_Flanger));
	this->SetParameter(TParameter::ParameterFXReverberator_OrbitoneSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Reverberator, MixerOutput::FX_Orbitone));
	this->SetParameter(TParameter::ParameterFXReverberator_PhaserSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Reverberator, MixerOutput::FX_Phaser));
	this->SetParameter(TParameter::ParameterFXReverberator_DelaySend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Reverberator, MixerOutput::FX_Delay));
	this->SetParameter(TParameter::ParameterFXReverberator_PlateReverbSend, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Reverberator, MixerOutput::FX_PlateReverb));
	this->SetParameter(TParameter::ParameterFXReverberator_MainOutput, this->m_PerformanceConfig.GetFXSendLevel(MixerOutput::FX_Reverberator, MixerOutput::MainOutput));

	this->mixing_console_->bypass(this->m_PerformanceConfig.IsFXBypass());
#endif
}

std::string CMiniDexed::GetNewPerformanceDefaultName(void)	
{
	return m_PerformanceConfig.GetNewPerformanceDefaultName();
}

void CMiniDexed::SetNewPerformanceName(std::string nName)
{
	m_PerformanceConfig.SetNewPerformanceName(nName);
}

void CMiniDexed::SetVoiceName (std::string VoiceName, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);
	char Name[10];
	strncpy(Name, VoiceName.c_str(),10);
	m_pTG[nTG]->getName (Name);
}

bool CMiniDexed::DeletePerformance(unsigned nID)
{
	m_bDeletePerformance = true;
	m_nDeletePerformanceID = nID;

	return true;
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
