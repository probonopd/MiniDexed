//
// performanceconfig.cpp
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
#include "performanceconfig.h"
#include "mididevice.h"

CPerformanceConfig::CPerformanceConfig (FATFS *pFileSystem)
:	m_Properties ("performance.ini", pFileSystem)
{
}

CPerformanceConfig::~CPerformanceConfig (void)
{
}

bool CPerformanceConfig::Load (void)
{
	if (!m_Properties.Load ())
	{
		return false;
	}

	bool bResult = false;

	for (unsigned nTG = 0; nTG < CConfig::ToneGenerators; nTG++)
	{
		CString PropertyName;

		PropertyName.Format ("BankNumber%u", nTG+1);
		m_nBankNumber[nTG] = m_Properties.GetNumber (PropertyName, 0);

		PropertyName.Format ("VoiceNumber%u", nTG+1);
		m_nVoiceNumber[nTG] = m_Properties.GetNumber (PropertyName, 1);
		if (m_nVoiceNumber[nTG] > 0)
		{
			m_nVoiceNumber[nTG]--;
		}

		PropertyName.Format ("MIDIChannel%u", nTG+1);
		unsigned nMIDIChannel = m_Properties.GetNumber (PropertyName, 255);
		if (nMIDIChannel == 0)
		{
			m_nMIDIChannel[nTG] = CMIDIDevice::Disabled;
		}
		else if (nMIDIChannel <= CMIDIDevice::Channels)
		{
			m_nMIDIChannel[nTG] = nMIDIChannel-1;
			bResult = true;
		}
		else
		{
			m_nMIDIChannel[nTG] = CMIDIDevice::OmniMode;
			bResult = true;
		}

		PropertyName.Format ("Volume%u", nTG+1);
		m_nVolume[nTG] = m_Properties.GetNumber (PropertyName, 100);

		PropertyName.Format ("Pan%u", nTG+1);
		m_nPan[nTG] = m_Properties.GetNumber (PropertyName, 64);

		PropertyName.Format ("Detune%u", nTG+1);
		m_nDetune[nTG] = m_Properties.GetSignedNumber (PropertyName, 0);

		PropertyName.Format ("NoteLimitLow%u", nTG+1);
		m_nNoteLimitLow[nTG] = m_Properties.GetNumber (PropertyName, 0);

		PropertyName.Format ("NoteLimitHigh%u", nTG+1);
		m_nNoteLimitHigh[nTG] = m_Properties.GetNumber (PropertyName, 127);

		PropertyName.Format ("NoteShift%u", nTG+1);
		m_nNoteShift[nTG] = m_Properties.GetSignedNumber (PropertyName, 0);
	}

	m_bCompressorEnable = m_Properties.GetNumber ("CompressorEnable", 1) != 0;

	m_bReverbEnable = m_Properties.GetNumber ("ReverbEnable", 1) != 0;
	m_nReverbSize = m_Properties.GetNumber ("ReverbSize", 70);
	m_nReverbHighDamp = m_Properties.GetNumber ("ReverbHighDamp", 50);
	m_nReverbLowDamp = m_Properties.GetNumber ("ReverbLowDamp", 50);
	m_nReverbLowPass = m_Properties.GetNumber ("ReverbLowPass", 30);
	m_nReverbDiffusion = m_Properties.GetNumber ("ReverbDiffusion", 65);
	m_nReverbLevel = m_Properties.GetNumber ("ReverbLevel", 80);

	return bResult;
}

bool CPerformanceConfig::Save (void)
{
	m_Properties.RemoveAll ();

	for (unsigned nTG = 0; nTG < CConfig::ToneGenerators; nTG++)
	{
		CString PropertyName;

		PropertyName.Format ("BankNumber%u", nTG+1);
		m_Properties.SetNumber (PropertyName, m_nBankNumber[nTG]);

		PropertyName.Format ("VoiceNumber%u", nTG+1);
		m_Properties.SetNumber (PropertyName, m_nVoiceNumber[nTG]+1);

		PropertyName.Format ("MIDIChannel%u", nTG+1);
		unsigned nMIDIChannel = m_nMIDIChannel[nTG];
		if (nMIDIChannel < CMIDIDevice::Channels)
		{
			nMIDIChannel++;
		}
		else if (nMIDIChannel == CMIDIDevice::OmniMode)
		{
			nMIDIChannel = 255;
		}
		else
		{
			nMIDIChannel = 0;
		}
		m_Properties.SetNumber (PropertyName, nMIDIChannel);

		PropertyName.Format ("Volume%u", nTG+1);
		m_Properties.SetNumber (PropertyName, m_nVolume[nTG]);

		PropertyName.Format ("Pan%u", nTG+1);
		m_Properties.SetNumber (PropertyName, m_nPan[nTG]);

		PropertyName.Format ("Detune%u", nTG+1);
		m_Properties.SetSignedNumber (PropertyName, m_nDetune[nTG]);

		PropertyName.Format ("NoteLimitLow%u", nTG+1);
		m_Properties.SetNumber (PropertyName, m_nNoteLimitLow[nTG]);

		PropertyName.Format ("NoteLimitHigh%u", nTG+1);
		m_Properties.SetNumber (PropertyName, m_nNoteLimitHigh[nTG]);

		PropertyName.Format ("NoteShift%u", nTG+1);
		m_Properties.SetSignedNumber (PropertyName, m_nNoteShift[nTG]);
	}

	m_Properties.SetNumber ("CompressorEnable", m_bCompressorEnable ? 1 : 0);

	m_Properties.SetNumber ("ReverbEnable", m_bReverbEnable ? 1 : 0);
	m_Properties.SetNumber ("ReverbSize", m_nReverbSize);
	m_Properties.SetNumber ("ReverbHighDamp", m_nReverbHighDamp);
	m_Properties.SetNumber ("ReverbLowDamp", m_nReverbLowDamp);
	m_Properties.SetNumber ("ReverbLowPass", m_nReverbLowPass);
	m_Properties.SetNumber ("ReverbDiffusion", m_nReverbDiffusion);
	m_Properties.SetNumber ("ReverbLevel", m_nReverbLevel);

	return m_Properties.Save ();
}

unsigned CPerformanceConfig::GetBankNumber (unsigned nTG) const
{
	assert (nTG < CConfig::ToneGenerators);
	return m_nBankNumber[nTG];
}

unsigned CPerformanceConfig::GetVoiceNumber (unsigned nTG) const
{
	assert (nTG < CConfig::ToneGenerators);
	return m_nVoiceNumber[nTG];
}

unsigned CPerformanceConfig::GetMIDIChannel (unsigned nTG) const
{
	assert (nTG < CConfig::ToneGenerators);
	return m_nMIDIChannel[nTG];
}

unsigned CPerformanceConfig::GetVolume (unsigned nTG) const
{
	assert (nTG < CConfig::ToneGenerators);
	return m_nVolume[nTG];
}

unsigned CPerformanceConfig::GetPan (unsigned nTG) const
{
	assert (nTG < CConfig::ToneGenerators);
	return m_nPan[nTG];
}

int CPerformanceConfig::GetDetune (unsigned nTG) const
{
	assert (nTG < CConfig::ToneGenerators);
	return m_nDetune[nTG];
}

unsigned CPerformanceConfig::GetNoteLimitLow (unsigned nTG) const
{
	assert (nTG < CConfig::ToneGenerators);
	return m_nNoteLimitLow[nTG];
}

unsigned CPerformanceConfig::GetNoteLimitHigh (unsigned nTG) const
{
	assert (nTG < CConfig::ToneGenerators);
	return m_nNoteLimitHigh[nTG];
}

int CPerformanceConfig::GetNoteShift (unsigned nTG) const
{
	assert (nTG < CConfig::ToneGenerators);
	return m_nNoteShift[nTG];
}

void CPerformanceConfig::SetBankNumber (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	m_nBankNumber[nTG] = nValue;
}

void CPerformanceConfig::SetVoiceNumber (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	m_nVoiceNumber[nTG] = nValue;
}

void CPerformanceConfig::SetMIDIChannel (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	m_nMIDIChannel[nTG] = nValue;
}

void CPerformanceConfig::SetVolume (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	m_nVolume[nTG] = nValue;
}

void CPerformanceConfig::SetPan (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	m_nPan[nTG] = nValue;
}

void CPerformanceConfig::SetDetune (int nValue, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	m_nDetune[nTG] = nValue;
}

void CPerformanceConfig::SetNoteLimitLow (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	m_nNoteLimitLow[nTG] = nValue;
}

void CPerformanceConfig::SetNoteLimitHigh (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	m_nNoteLimitHigh[nTG] = nValue;
}

void CPerformanceConfig::SetNoteShift (int nValue, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	m_nNoteShift[nTG] = nValue;
}

bool CPerformanceConfig::GetCompressorEnable (void) const
{
	return m_bCompressorEnable;
}

bool CPerformanceConfig::GetReverbEnable (void) const
{
	return m_bReverbEnable;
}

unsigned CPerformanceConfig::GetReverbSize (void) const
{
	return m_nReverbSize;
}

unsigned CPerformanceConfig::GetReverbHighDamp (void) const
{
	return m_nReverbHighDamp;
}

unsigned CPerformanceConfig::GetReverbLowDamp (void) const
{
	return m_nReverbLowDamp;
}

unsigned CPerformanceConfig::GetReverbLowPass (void) const
{
	return m_nReverbLowPass;
}

unsigned CPerformanceConfig::GetReverbDiffusion (void) const
{
	return m_nReverbDiffusion;
}

unsigned CPerformanceConfig::GetReverbLevel (void) const
{
	return m_nReverbLevel;
}

void CPerformanceConfig::SetCompressorEnable (bool bValue)
{
	m_bCompressorEnable = bValue;
}

void CPerformanceConfig::SetReverbEnable (bool bValue)
{
	m_bReverbEnable = bValue;
}

void CPerformanceConfig::SetReverbSize (unsigned nValue)
{
	m_nReverbSize = nValue;
}

void CPerformanceConfig::SetReverbHighDamp (unsigned nValue)
{
	m_nReverbHighDamp = nValue;
}

void CPerformanceConfig::SetReverbLowDamp (unsigned nValue)
{
	m_nReverbLowDamp = nValue;
}

void CPerformanceConfig::SetReverbLowPass (unsigned nValue)
{
	m_nReverbLowPass = nValue;
}

void CPerformanceConfig::SetReverbDiffusion (unsigned nValue)
{
	m_nReverbDiffusion = nValue;
}

void CPerformanceConfig::SetReverbLevel (unsigned nValue)
{
	m_nReverbLevel = nValue;
}
