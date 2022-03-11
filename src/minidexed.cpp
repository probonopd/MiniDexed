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
			CGPIOManager *pGPIOManager, CI2CMaster *pI2CMaster)
:	CDexedAdapter (CConfig::MaxNotes, pConfig->GetSampleRate ()),
#ifdef ARM_ALLOW_MULTI_CORE
	CMultiCoreSupport (CMemorySystem::Get ()),
#endif
	m_pConfig (pConfig),
	m_UI (this, pGPIOManager, pConfig),
	m_PCKeyboard (this),
	m_SerialMIDI (this, pInterrupt, pConfig),
	m_bUseSerial (false),
	m_pSoundDevice (0),
	m_GetChunkTimer ("GetChunk",
			 1000000U * pConfig->GetChunkSize ()/2 / pConfig->GetSampleRate ()),
	m_bProfileEnabled (m_pConfig->GetProfileEnabled ())
{
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
	}
	else
	{
		LOGNOTE ("PWM mode");

		m_pSoundDevice = new CPWMSoundBaseDevice (pInterrupt, pConfig->GetSampleRate (),
							  pConfig->GetChunkSize ());
	}
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

	activate ();

	ProgramChange (0);
	setTranspose (24);

	// setup and start the sound device
	if (!m_pSoundDevice->AllocateQueueFrames (m_pConfig->GetChunkSize ()/2))
	{
		LOGERR ("Cannot allocate sound queue");

		return false;
	}

	m_pSoundDevice->SetWriteFormat (SoundFormatSigned16, 1);	// 16-bit Mono

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
	if (nCore == 1)
	{
		while (1)
		{
			ProcessSound ();
		}
	}
}

#endif

CSysExFileLoader *CMiniDexed::GetSysExFileLoader (void)
{
	return &m_SysExFileLoader;
}

void CMiniDexed::BankSelectLSB (unsigned nBankLSB)
{
	if (nBankLSB > 127)
	{
		return;
	}

	m_SysExFileLoader.SelectVoiceBank (nBankLSB);

	m_UI.BankSelected (nBankLSB);
}

void CMiniDexed::ProgramChange (unsigned nProgram)
{
	if (nProgram > 31)
	{
		return;
	}

	uint8_t Buffer[156];
	m_SysExFileLoader.GetVoice (nProgram, Buffer);
	loadVoiceParameters (Buffer);

	m_UI.ProgramChanged (nProgram);
}

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
		getSamples (nFrames, SampleBuffer);

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
