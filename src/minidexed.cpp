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

LOGMODULE ("minidexed");

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

  activate();

  ProgramChange (0);
  setTranspose (24);

  return true;
}

void CMiniDexed::Process(boolean bPlugAndPlayUpdated)
{
	if (m_pConfig->GetProfileEnabled ())
	{
		m_GetChunkTimer.Dump ();
	}

	m_UI.Process ();

	m_MIDIKeyboard.Process (bPlugAndPlayUpdated);

	m_PCKeyboard.Process (bPlugAndPlayUpdated);

	if (m_bUseSerial)
	{
		m_SerialMIDI.Process ();
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

void CMiniDexed::ProgramChange (unsigned program)
{
	if (program > 31)
	{
		return;
	}

	uint8_t Buffer[156];
	m_SysExFileLoader.GetVoice (program, Buffer);
	loadVoiceParameters (Buffer);

	m_UI.ProgramChanged (program);
}

bool CMiniDexedPWM::Initialize (void)
{
  if (!CMiniDexed::Initialize())
  {
    return false;
  }

  return Start ();
}

unsigned CMiniDexedPWM::GetChunk(u32 *pBuffer, unsigned nChunkSize)
{
  m_GetChunkTimer.Start();

  unsigned nResult = nChunkSize;

  int16_t int16_buf[nChunkSize/2];

  getSamples(nChunkSize/2, int16_buf);

  for (unsigned i = 0; nChunkSize > 0; nChunkSize -= 2)		// fill the whole buffer
  {
    s32 nSample = int16_buf[i++];
    nSample += 32768;
    nSample *= GetRangeMax()/2;
    nSample /= 32768;

    *pBuffer++ = nSample;		// 2 stereo channels
    *pBuffer++ = nSample;
  }

  m_GetChunkTimer.Stop();

  return(nResult);
};

bool CMiniDexedI2S::Initialize (void)
{
  if (!CMiniDexed::Initialize())
  {
    return false;
  }

  return Start ();
}

unsigned CMiniDexedI2S::GetChunk(u32 *pBuffer, unsigned nChunkSize)
{
  m_GetChunkTimer.Start();

  unsigned nResult = nChunkSize;

  int16_t int16_buf[nChunkSize/2];

  getSamples(nChunkSize/2, int16_buf);

  for (unsigned i = 0; nChunkSize > 0; nChunkSize -= 2)		// fill the whole buffer
  {
    s32 nSample = int16_buf[i++];
    nSample <<= 8;

    *pBuffer++ = nSample;		// 2 stereo channels
    *pBuffer++ = nSample;
  }

  m_GetChunkTimer.Stop();

  return(nResult);
};

bool CMiniDexedHDMI::Initialize (void)
{
  if (!CMiniDexed::Initialize())
  {
    return false;
  }

  return Start ();
}

unsigned CMiniDexedHDMI::GetChunk(u32 *pBuffer, unsigned nChunkSize)
{
  m_GetChunkTimer.Start();

  unsigned nResult = nChunkSize;

  int16_t int16_buf[nChunkSize/2];
  unsigned nFrame = 0;

  getSamples(nChunkSize/2, int16_buf);

  for (unsigned i = 0; nChunkSize > 0; nChunkSize -= 2)		// fill the whole buffer
  {
    s32 nSample = int16_buf[i++];
    nSample <<= 8;

    nSample = ConvertIEC958Sample (nSample, nFrame);

    if (++nFrame == IEC958_FRAMES_PER_BLOCK)
      nFrame = 0;

    *pBuffer++ = nSample;		// 2 stereo channels
    *pBuffer++ = nSample;
  }

  m_GetChunkTimer.Stop();

  return(nResult);
};
