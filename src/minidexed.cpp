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
		m_nPan[i] = 64;

		m_nNoteLimitLow[i] = 0;
		m_nNoteLimitHigh[i] = 127;
		m_nNoteShift[i] = 0;

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

	// BEGIN setup reverb
	reverb = new AudioEffectPlateReverb(pConfig->GetSampleRate());
	reverb->size(0.7);
	reverb->hidamp(0.5);
	reverb->lodamp(0.5);
	reverb->lowpass(0.3);
	reverb->diffusion(0.2);
	reverb->send(0.8);
	// END setup reverb
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

			m_nNoteLimitLow[nTG] = m_PerformanceConfig.GetNoteLimitLow (nTG);
			m_nNoteLimitHigh[nTG] = m_PerformanceConfig.GetNoteLimitHigh (nTG);
			m_nNoteShift[nTG] = m_PerformanceConfig.GetNoteShift (nTG);
		}
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
				m_pTG[nTG]->getSamples (m_nFramesToProcess, m_OutputLevel[nTG]);
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
	if (nBankLSB > 127)
	{
		return;
	}

	assert (nTG < CConfig::ToneGenerators);
	m_nVoiceBankID[nTG] = nBankLSB;

	m_UI.BankSelected (nBankLSB, nTG);
}

void CMiniDexed::ProgramChange (unsigned nProgram, unsigned nTG)
{
	if (nProgram > 31)
	{
		return;
	}

	assert (nTG < CConfig::ToneGenerators);
	uint8_t Buffer[156];
	m_SysExFileLoader.GetVoice (m_nVoiceBankID[nTG], nProgram, Buffer);

	assert (m_pTG[nTG]);
	m_pTG[nTG]->loadVoiceParameters (Buffer);

	m_UI.ProgramChanged (nProgram, nTG);
}

void CMiniDexed::SetVolume (unsigned nVolume, unsigned nTG)
{
	if (nVolume > 127)
	{
		return;
	}

	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);
	m_pTG[nTG]->setGain (nVolume / 127.0);

	m_UI.VolumeChanged (nVolume, nTG);
}

void CMiniDexed::SetPan (unsigned nPan, unsigned nTG)
{
	if (nPan > 127)
	{
		return;
	}

	assert (nTG < CConfig::ToneGenerators);
	m_nPan[nTG] = nPan;

	m_UI.PanChanged (nPan, nTG);
}

void CMiniDexed::SetMasterTune (int nMasterTune, unsigned nTG)
{
	if (!(-99 <= nMasterTune && nMasterTune <= 99))
	{
		return;
	}

	assert (nTG < CConfig::ToneGenerators);
	assert (m_pTG[nTG]);
	m_pTG[nTG]->setMasterTune ((int8_t) nMasterTune);

	m_UI.MasterTuneChanged (nMasterTune, nTG);
}

void CMiniDexed::SetMIDIChannel (uint8_t uchChannel, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);

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
		if (m_PCKeyboard.GetChannel (nTG) != CMIDIDevice::Disabled)
		{
			nActiveTGs++;
		}
	}

	assert (nActiveTGs <= 8);
	static const unsigned Log2[] = {0, 0, 1, 2, 2, 3, 3, 3, 3};
	m_nActiveTGsLog2 = Log2[nActiveTGs];
#endif

	m_UI.MIDIChannelChanged (uchChannel, nTG);
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

		int16_t SampleBuffer[nFrames];
		m_pTG[0]->getSamples (nFrames, SampleBuffer);

		if (   m_pSoundDevice->Write (SampleBuffer, sizeof SampleBuffer)
		    != (int) sizeof SampleBuffer)
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
			m_pTG[i]->getSamples (nFrames, m_OutputLevel[i]);
		}

		// wait for cores 2 and 3 to complete their work
		for (unsigned nCore = 2; nCore < CORES; nCore++)
		{
			while (m_CoreStatus[nCore] != CoreStatusIdle)
			{
				// just wait
			}
		}

		// now mix the output of all TGs
		int16_t SampleBuffer[nFrames][2];

		assert (CConfig::ToneGenerators == 8);
		for (unsigned i = 0; i < nFrames; i++)
		{
			int32_t nLeft =   m_OutputLevel[0][i] * (127-m_nPan[0])
					+ m_OutputLevel[1][i] * (127-m_nPan[1])
					+ m_OutputLevel[2][i] * (127-m_nPan[2])
					+ m_OutputLevel[3][i] * (127-m_nPan[3])
					+ m_OutputLevel[4][i] * (127-m_nPan[4])
					+ m_OutputLevel[5][i] * (127-m_nPan[5])
					+ m_OutputLevel[6][i] * (127-m_nPan[6])
					+ m_OutputLevel[7][i] * (127-m_nPan[7]);
			nLeft >>= m_nActiveTGsLog2 + 7;

			int32_t nRight =   m_OutputLevel[0][i] * m_nPan[0]
					 + m_OutputLevel[1][i] * m_nPan[1]
					 + m_OutputLevel[2][i] * m_nPan[2]
					 + m_OutputLevel[3][i] * m_nPan[3]
					 + m_OutputLevel[4][i] * m_nPan[4]
					 + m_OutputLevel[5][i] * m_nPan[5]
					 + m_OutputLevel[6][i] * m_nPan[6]
					 + m_OutputLevel[7][i] * m_nPan[7];
			nRight >>= m_nActiveTGsLog2 + 7;

			if (!m_bChannelsSwapped)
			{
				SampleBuffer[i][0] = (int16_t) nLeft;
				SampleBuffer[i][1] = (int16_t) nRight;
			}
			else
			{
				SampleBuffer[i][0] = (int16_t) nRight;
				SampleBuffer[i][1] = (int16_t) nLeft;
			}
		}

		// BEGIN adding reverb
		reverb->doReverb(nFrames,SampleBuffer);
		// END adding reverb

		if (m_pSoundDevice->Write (SampleBuffer, sizeof SampleBuffer) != (int) sizeof SampleBuffer)
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
