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
#include <circle/pwmsoundbasedevice.h>
#include <circle/i2ssoundbasedevice.h>
#include <circle/hdmisoundbasedevice.h>
#include <circle/gpiopin.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

LOGMODULE ("minidexed");

CMiniDexed::CMiniDexed (CConfig *pConfig, CInterruptSystem *pInterrupt,
			CGPIOManager *pGPIOManager, CI2CMaster *pI2CMaster, FATFS *pFileSystem)
:
#ifdef ARM_ALLOW_MULTI_CORE
	CMultiCoreSupport (CMemorySystem::Get ()),
#endif
	m_pConfig (pConfig),
	m_UI (this, pGPIOManager, pConfig),
	m_PerformanceConfig (pFileSystem),
	m_PCKeyboard (this, pConfig),
	m_SerialMIDI (this, pInterrupt, pConfig),
	m_bUseSerial (false),
	m_pSoundDevice (0),
	m_bChannelsSwapped (pConfig->GetChannelsSwapped ()),
#ifdef ARM_ALLOW_MULTI_CORE
	m_nActiveTGsLog2 (0),
#endif
	m_GetChunkTimer ("GetChunk",
			 1000000U * pConfig->GetChunkSize ()/2 / pConfig->GetSampleRate ()),
	m_bProfileEnabled (m_pConfig->GetProfileEnabled ())
{
	assert (m_pConfig);

	for (unsigned i = 0; i < CConfig::ToneGenerators; i++)
	{
		m_nVoiceBankID[i] = 0;
		m_nProgram[i] = 0;
		m_nVolume[i] = 100;
		m_nPan[i] = 64;
		m_nMasterTune[i] = 0;
		m_nCutoff[i] = 99;
		m_nResonance[i] = 0;
		m_nMIDIChannel[i] = CMIDIDevice::Disabled;

		m_nNoteLimitLow[i] = 0;
		m_nNoteLimitHigh[i] = 127;
		m_nNoteShift[i] = 0;

		m_nReverbSend[i] = 0;
		m_uchOPMask[i] = 0b111111;	// All operators on

		m_pTG[i] = new CDexedAdapter (CConfig::MaxNotes, pConfig->GetSampleRate ());
		assert (m_pTG[i]);

		m_pTG[i]->activate ();
	}

	for (unsigned i = 0; i < CConfig::MaxUSBMIDIDevices; i++)
	{
		m_pMIDIKeyboard[i] = new CMIDIKeyboard (this, pConfig, i);
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

	// BEGIN setup tg_mixer
	tg_mixer = new AudioStereoMixer<CConfig::ToneGenerators>(pConfig->GetChunkSize()/2);
	// END setup tgmixer

	// BEGIN setup reverb
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
};

bool CMiniDexed::Initialize (void)
{
	assert (m_pConfig);
	assert (m_pSoundDevice);

	if (!m_UI.Initialize ())
	{
		return false;
	}

	m_SysExFileLoader.Load ();

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

		m_pTG[i]->setPBController (12, 1);
		m_pTG[i]->setMWController (99, 7, 0);

		tg_mixer->pan(i,mapfloat(m_nPan[i],0,127,0.0f,1.0f));
		tg_mixer->gain(i,1.0f);
		reverb_send_mixer->pan(i,mapfloat(m_nPan[i],0,127,0.0f,1.0f));
		reverb_send_mixer->gain(i,mapfloat(m_nReverbSend[i],0,99,0.0f,1.0f));
	}

	if (m_PerformanceConfig.Load ())
	{
		for (unsigned nTG = 0; nTG < CConfig::ToneGenerators; nTG++)
		{
			BankSelectLSB (m_PerformanceConfig.GetBankNumber (nTG), nTG);
			ProgramChange (m_PerformanceConfig.GetVoiceNumber (nTG), nTG);
			SetMIDIChannel (m_PerformanceConfig.GetMIDIChannel (nTG), nTG);
			SetVolume (m_PerformanceConfig.GetVolume (nTG), nTG);
			SetPan (m_PerformanceConfig.GetPan (nTG), nTG);
			SetMasterTune (m_PerformanceConfig.GetDetune (nTG), nTG);
			SetCutoff (m_PerformanceConfig.GetCutoff (nTG), nTG);
			SetResonance (m_PerformanceConfig.GetResonance (nTG), nTG);

			m_nNoteLimitLow[nTG] = m_PerformanceConfig.GetNoteLimitLow (nTG);
			m_nNoteLimitHigh[nTG] = m_PerformanceConfig.GetNoteLimitHigh (nTG);
			m_nNoteShift[nTG] = m_PerformanceConfig.GetNoteShift (nTG);

			SetReverbSend (m_PerformanceConfig.GetReverbSend (nTG), nTG);
		}

		// Effects
		SetParameter (ParameterCompressorEnable, m_PerformanceConfig.GetCompressorEnable () ? 1 : 0);
		SetParameter (ParameterReverbEnable, m_PerformanceConfig.GetReverbEnable () ? 1 : 0);
		SetParameter (ParameterReverbSize, m_PerformanceConfig.GetReverbSize ());
		SetParameter (ParameterReverbHighDamp, m_PerformanceConfig.GetReverbHighDamp ());
		SetParameter (ParameterReverbLowDamp, m_PerformanceConfig.GetReverbLowDamp ());
		SetParameter (ParameterReverbLowPass, m_PerformanceConfig.GetReverbLowPass ());
		SetParameter (ParameterReverbDiffusion, m_PerformanceConfig.GetReverbDiffusion ());
		SetParameter (ParameterReverbLevel, m_PerformanceConfig.GetReverbLevel ());
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
			}
		}
	}
}

#endif

CSysExFileLoader *CMiniDexed::GetSysExFileLoader (void)
{
	return &m_SysExFileLoader;
}

void CMiniDexed::BankSelectLSB (unsigned nBankLSB, unsigned nTG)
{
	nBankLSB=constrain((int)nBankLSB,0,127);

	assert (nTG < CConfig::ToneGenerators);
	m_nVoiceBankID[nTG] = nBankLSB;

	m_UI.ParameterChanged ();
}

void CMiniDexed::ProgramChange (unsigned nProgram, unsigned nTG)
{
	nProgram=constrain((int)nProgram,0,31);

	assert (nTG < CConfig::ToneGenerators);
	m_nProgram[nTG] = nProgram;

	uint8_t Buffer[156];
	m_SysExFileLoader.GetVoice (m_nVoiceBankID[nTG], nProgram, Buffer);

	assert (m_pTG[nTG]);
	m_pTG[nTG]->loadVoiceParameters (Buffer);

	m_UI.ParameterChanged ();
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

	case ParameterReverbEnable:
		nValue=constrain((int)nValue,0,1);
		m_ReverbSpinLock.Acquire ();
		reverb->set_bypass (!nValue);
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

	default:
		assert (0);
		break;
	}
}

int CMiniDexed::GetParameter (TParameter Parameter)
{
	assert (Parameter < ParameterUnknown);
	return m_nParameter[Parameter];
}

void CMiniDexed::SetTGParameter (TTGParameter Parameter, int nValue, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);

	switch (Parameter)
	{
	case TGParameterVoiceBank:	BankSelectLSB (nValue, nTG);	break;
	case TGParameterProgram:	ProgramChange (nValue, nTG);	break;
	case TGParameterVolume:		SetVolume (nValue, nTG);	break;
	case TGParameterPan:		SetPan (nValue, nTG);		break;
	case TGParameterMasterTune:	SetMasterTune (nValue, nTG);	break;
	case TGParameterCutoff:		SetCutoff (nValue, nTG);	break;
	case TGParameterResonance:	SetResonance (nValue, nTG);	break;

	case TGParameterMIDIChannel:
		assert (0 <= nValue && nValue <= 255);
		SetMIDIChannel ((uint8_t) nValue, nTG);
		break;

	case TGParameterReverbSend:	SetReverbSend (nValue, nTG);	break;

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
	case TGParameterProgram:	return m_nProgram[nTG];
	case TGParameterVolume:		return m_nVolume[nTG];
	case TGParameterPan:		return m_nPan[nTG];
	case TGParameterMasterTune:	return m_nMasterTune[nTG];
	case TGParameterCutoff:		return m_nCutoff[nTG];
	case TGParameterResonance:	return m_nResonance[nTG];
	case TGParameterMIDIChannel:	return m_nMIDIChannel[nTG];
	case TGParameterReverbSend:	return m_nReverbSend[nTG];

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

		// swap stereo channels if needed
		uint8_t indexL=0, indexR=1;
		if (m_bChannelsSwapped)
		{
			indexL=1;
			indexR=0;
		}
		
		// BEGIN TG mixing
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

			m_ReverbSpinLock.Acquire ();

                	reverb_send_mixer->getMix(ReverbSendBuffer[indexL], ReverbSendBuffer[indexR]);
			reverb->doReverb(ReverbSendBuffer[indexL],ReverbSendBuffer[indexR],ReverbBuffer[indexL], ReverbBuffer[indexR],nFrames);

			// scale down and add left reverb buffer by reverb level 
			arm_scale_f32(ReverbBuffer[indexL], reverb->get_level(), ReverbBuffer[indexL], nFrames);
			arm_add_f32(SampleBuffer[indexL], ReverbBuffer[indexL], SampleBuffer[indexL], nFrames);
			// scale down and add right reverb buffer by reverb level 
			arm_scale_f32(ReverbBuffer[indexR], reverb->get_level(), ReverbBuffer[indexR], nFrames);
			arm_add_f32(SampleBuffer[indexR], ReverbBuffer[indexR], SampleBuffer[indexR], nFrames);

			m_ReverbSpinLock.Release ();
		}
		// END adding reverb

		// Convert dual float array (left, right) to single int16 array (left/right)
		float32_t tmp_float[nFrames*2];
		int16_t tmp_int[nFrames*2];
		for(uint16_t i=0; i<nFrames;i++)
		{
			tmp_float[i*2]=SampleBuffer[indexL][i];
			tmp_float[(i*2)+1]=SampleBuffer[indexR][i];
		}
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

#endif

bool CMiniDexed::SavePerformance (void)
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

		m_PerformanceConfig.SetNoteLimitLow (m_nNoteLimitLow[nTG], nTG);
		m_PerformanceConfig.SetNoteLimitHigh (m_nNoteLimitHigh[nTG], nTG);
		m_PerformanceConfig.SetNoteShift (m_nNoteShift[nTG], nTG);

		m_PerformanceConfig.SetReverbSend (m_nReverbSend[nTG], nTG);
	}

	m_PerformanceConfig.SetCompressorEnable (!!m_nParameter[ParameterCompressorEnable]);
	m_PerformanceConfig.SetReverbEnable (!!m_nParameter[ParameterReverbEnable]);
	m_PerformanceConfig.SetReverbSize (m_nParameter[ParameterReverbSize]);
	m_PerformanceConfig.SetReverbHighDamp (m_nParameter[ParameterReverbHighDamp]);
	m_PerformanceConfig.SetReverbLowDamp (m_nParameter[ParameterReverbLowDamp]);
	m_PerformanceConfig.SetReverbLowPass (m_nParameter[ParameterReverbLowPass]);
	m_PerformanceConfig.SetReverbDiffusion (m_nParameter[ParameterReverbDiffusion]);
	m_PerformanceConfig.SetReverbLevel (m_nParameter[ParameterReverbLevel]);

	return m_PerformanceConfig.Save ();
}

void CMiniDexed::setMonoMode(uint8_t mono, uint8_t nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);

	m_pTG[nTG]->setMonoMode(constrain(mono, 0, 1));
	m_pTG[nTG]->doRefreshVoice();
	m_UI.ParameterChanged ();
}

void CMiniDexed::setPitchbendRange(uint8_t range, uint8_t nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);

	m_pTG[nTG]->setPitchbendRange(constrain(range, 0, 12));
	m_pTG[nTG]->ControllersRefresh();
	m_UI.ParameterChanged ();
}

void CMiniDexed::setPitchbendStep(uint8_t step, uint8_t nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);

	m_pTG[nTG]->setPitchbendStep(constrain(step, 0, 12));
	m_pTG[nTG]->ControllersRefresh();
	m_UI.ParameterChanged ();
}

void CMiniDexed::setPortamentoMode(uint8_t mode, uint8_t nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);

	m_pTG[nTG]->setPortamentoMode(constrain(mode, 0, 1));
	m_pTG[nTG]->ControllersRefresh();
	m_UI.ParameterChanged ();
}

void CMiniDexed::setPortamentoGlissando(uint8_t glissando, uint8_t nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);

	m_pTG[nTG]->setPortamentoGlissando(constrain(glissando, 0, 1));
	m_pTG[nTG]->ControllersRefresh();
	m_UI.ParameterChanged ();
}

void CMiniDexed::setPortamentoTime(uint8_t time, uint8_t nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);

	m_pTG[nTG]->setPortamentoTime(constrain(time, 0, 99));
	m_pTG[nTG]->ControllersRefresh();
	m_UI.ParameterChanged ();
}

void CMiniDexed::setModWheelRange(uint8_t range, uint8_t nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);

	m_pTG[nTG]->setModWheelRange(constrain(range, 0, 99));
	m_pTG[nTG]->ControllersRefresh();
	m_UI.ParameterChanged ();
}

void CMiniDexed::setModWheelTarget(uint8_t target, uint8_t nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);

	m_pTG[nTG]->setModWheelTarget(constrain(target, 0, 7));
	m_pTG[nTG]->ControllersRefresh();
	m_UI.ParameterChanged ();
}

void CMiniDexed::setFootControllerRange(uint8_t range, uint8_t nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);

	m_pTG[nTG]->setFootControllerRange(constrain(range, 0, 99));
	m_pTG[nTG]->ControllersRefresh();
	m_UI.ParameterChanged ();
}

void CMiniDexed::setFootControllerTarget(uint8_t target, uint8_t nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);

	m_pTG[nTG]->setFootControllerTarget(constrain(target, 0, 7));
	m_pTG[nTG]->ControllersRefresh();
	m_UI.ParameterChanged ();
}

void CMiniDexed::setBreathControllerRange(uint8_t range, uint8_t nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);

	m_pTG[nTG]->setBreathControllerRange(constrain(range, 0, 99));
	m_pTG[nTG]->ControllersRefresh();
	m_UI.ParameterChanged ();
}

void CMiniDexed::setBreathControllerTarget(uint8_t target, uint8_t nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);

	m_pTG[nTG]->setBreathControllerTarget(constrain(target, 0, 7));
	m_pTG[nTG]->ControllersRefresh();
	m_UI.ParameterChanged ();
}

void CMiniDexed::setAftertouchRange(uint8_t range, uint8_t nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);

	m_pTG[nTG]->setAftertouchRange(constrain(range, 0, 99));
	m_pTG[nTG]->ControllersRefresh();
	m_UI.ParameterChanged ();
}

void CMiniDexed::setAftertouchTarget(uint8_t target, uint8_t nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);

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
	return(m_pTG[nTG]->checkSystemExclusive(pMessage, nLength));
}
