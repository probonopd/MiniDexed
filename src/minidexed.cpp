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
#include <stdio.h>
#include <assert.h>

LOGMODULE ("minidexed");

CMiniDexed::CMiniDexed (CConfig *pConfig, CInterruptSystem *pInterrupt)
:	CDexedAdapter (CConfig::MaxNotes, pConfig->GetSampleRate ()),
	m_pConfig (pConfig),
	m_UI (this, pConfig),
	m_MIDIKeyboard (this, pConfig),
	m_PCKeyboard (this),
	m_SerialMIDI (this, pInterrupt, pConfig),
	m_bUseSerial (false),
	m_GetChunkTimer ("GetChunk",
			 1000000U * pConfig->GetChunkSize ()/2 / pConfig->GetSampleRate ()),
	m_bProfileEnabled (m_pConfig->GetProfileEnabled ())
{
};

bool CMiniDexed::Initialize (void)
{
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

	return true;
}

void CMiniDexed::Process (bool bPlugAndPlayUpdated)
{
	m_MIDIKeyboard.Process (bPlugAndPlayUpdated);

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

void CMiniDexed::BankSelectLSB (unsigned nBankLSB)
{
	if (nBankLSB > 127)
	{
		return;
	}

	// MIDI numbering starts with 0, user interface with 1
	printf ("Select voice bank %u\n", nBankLSB+1);

	m_SysExFileLoader.SelectVoiceBank (nBankLSB);
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

//// PWM //////////////////////////////////////////////////////////////////////

CMiniDexedPWM::CMiniDexedPWM (CConfig *pConfig, CInterruptSystem *pInterrupt)
:	CMiniDexed (pConfig, pInterrupt),
	CPWMSoundBaseDevice (pInterrupt, pConfig->GetSampleRate (),
			     pConfig->GetChunkSize ())
{
}

bool CMiniDexedPWM::Initialize (void)
{
	if (!CMiniDexed::Initialize ())
	{
		return false;
	}

	return Start ();
}

unsigned CMiniDexedPWM::GetChunk (u32 *pBuffer, unsigned nChunkSize)
{
	if (m_bProfileEnabled)
	{
		m_GetChunkTimer.Start ();
	}

	unsigned nResult = nChunkSize;

	int16_t SampleBuffer[nChunkSize/2];
	getSamples (nChunkSize/2, SampleBuffer);

	for (unsigned i = 0; nChunkSize > 0; nChunkSize -= 2)	// fill the whole buffer
	{
		s32 nSample = SampleBuffer[i++];
		nSample += 32768;
		nSample *= GetRangeMax()/2;
		nSample /= 32768;

		*pBuffer++ = nSample;		// 2 stereo channels
		*pBuffer++ = nSample;
	}

	if (m_bProfileEnabled)
	{
		m_GetChunkTimer.Stop ();
	}

	return nResult;
};

//// I2S //////////////////////////////////////////////////////////////////////

CMiniDexedI2S::CMiniDexedI2S (CConfig *pConfig, CInterruptSystem *pInterrupt,
			      CI2CMaster *pI2CMaster)
:	CMiniDexed (pConfig, pInterrupt),
	CI2SSoundBaseDevice (pInterrupt, pConfig->GetSampleRate (),
			     pConfig->GetChunkSize (), false, pI2CMaster,
			     pConfig->GetDACI2CAddress ())
{
}

bool CMiniDexedI2S::Initialize (void)
{
	if (!CMiniDexed::Initialize ())
	{
		return false;
	}

	return Start ();
}

unsigned CMiniDexedI2S::GetChunk (u32 *pBuffer, unsigned nChunkSize)
{
	if (m_bProfileEnabled)
	{
		m_GetChunkTimer.Start ();
	}

	unsigned nResult = nChunkSize;

	int16_t SampleBuffer[nChunkSize/2];
	getSamples (nChunkSize/2, SampleBuffer);

	for (unsigned i = 0; nChunkSize > 0; nChunkSize -= 2)	// fill the whole buffer
	{
		s32 nSample = SampleBuffer[i++];
		nSample <<= 8;

		*pBuffer++ = nSample;		// 2 stereo channels
		*pBuffer++ = nSample;
	}

	if (m_bProfileEnabled)
	{
		m_GetChunkTimer.Stop ();
	}

	return nResult;
};

//// HDMI /////////////////////////////////////////////////////////////////////

CMiniDexedHDMI::CMiniDexedHDMI (CConfig *pConfig, CInterruptSystem *pInterrupt)
:	CMiniDexed (pConfig, pInterrupt),
	CHDMISoundBaseDevice (pInterrupt, pConfig->GetSampleRate (),
			      pConfig->GetChunkSize ())
{
}

bool CMiniDexedHDMI::Initialize (void)
{
	if (!CMiniDexed::Initialize ())
	{
		return false;
	}

	return Start ();
}

unsigned CMiniDexedHDMI::GetChunk(u32 *pBuffer, unsigned nChunkSize)
{
	if (m_bProfileEnabled)
	{
		m_GetChunkTimer.Start ();
	}

	unsigned nResult = nChunkSize;

	int16_t SampleBuffer[nChunkSize/2];
	getSamples (nChunkSize/2, SampleBuffer);

	unsigned nFrame = 0;
	for (unsigned i = 0; nChunkSize > 0; nChunkSize -= 2)		// fill the whole buffer
	{
		s32 nSample = SampleBuffer[i++];
		nSample <<= 8;

		nSample = ConvertIEC958Sample (nSample, nFrame);
		if (++nFrame == IEC958_FRAMES_PER_BLOCK)
		{
			nFrame = 0;
		}

		*pBuffer++ = nSample;		// 2 stereo channels
		*pBuffer++ = nSample;
	}

	if (m_bProfileEnabled)
	{
		m_GetChunkTimer.Stop();
	}

	return nResult;
};
