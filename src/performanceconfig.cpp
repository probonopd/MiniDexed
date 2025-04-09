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
#include <circle/logger.h>
#include "performanceconfig.h"
#include "mididevice.h"
#include <cstring> 
#include <algorithm>

LOGMODULE ("Performance");

//#define VERBOSE_DEBUG

#define PERFORMANCE_DIR "performance" 
#define DEFAULT_PERFORMANCE_FILENAME "performance.ini"
#define DEFAULT_PERFORMANCE_NAME "Default"
#define DEFAULT_PERFORMANCE_BANK_NAME "Default"

CPerformanceConfig::CPerformanceConfig (FATFS *pFileSystem)
{
	m_pFileSystem = pFileSystem; 
}

CPerformanceConfig::~CPerformanceConfig (void)
{
}

bool CPerformanceConfig::Init (unsigned nToneGenerators)
{
	// Different versions of Pi allow different TG configurations.
	// On loading, performances will load up to the number of
	// supported/active TGs.
	//
	// On saving, the active/supported number of TGs is used.
	//
	// This means that if an 8TG performance is loaded into
	// a 16 TG system and then saved, the saved performance
	// will include all 16 TG configurations.
	//
	m_nToneGenerators = nToneGenerators;

	// Check intermal performance directory exists
	DIR Directory;
	FRESULT Result;
	//Check if internal "performance" directory exists
	Result = f_opendir (&Directory, "SD:/" PERFORMANCE_DIR);
	if (Result == FR_OK)
	{
		m_bPerformanceDirectoryExists=true;		
		Result = f_closedir (&Directory);
	}
	else
	{
		m_bPerformanceDirectoryExists = false;
	}
	
	for (unsigned i=0; i<NUM_PERFORMANCES; i++) {
		m_pPerfBank[i] = nullptr;
	}
	
	// List banks if present
	ListPerformanceBanks();

#ifdef VERBOSE_DEBUG
#warning "PerformanceConfig in verbose debug printing mode"
	LOGNOTE("Testing loading of banks");
	for (unsigned i=0; i<NUM_PERFORMANCE_BANKS; i++)
	{
		if (!m_PerformanceBankName[i].empty())
		{
			SetNewPerformanceBank(i);
			SetNewPerformance(0);
		}
	}
#endif
	// Set to default initial bank
	SetNewPerformanceBank(0);
	SetNewPerformance(0);

	LOGNOTE ("Loaded Default Performance Bank - Last Performance: %d", m_nLastPerformance + 1); // Show "user facing" index

	return true;
}

bool CPerformanceConfig::Load (void)
{
	assert (m_nActualPerformance < NUM_PERFORMANCES);
	if (m_pPerfBank[m_nActualPerformance] != nullptr)
	{
		// Already loaded - nothing more to do
		return true;
	}

	std::string FileN = GetPerformanceFullFilePath(m_nActualPerformance);
	CPropertiesFatFsFile Properties (FileN.c_str(), m_pFileSystem);

	if (!Properties.Load ())
	{
		return false;
	}
	
	m_pPerfBank[m_nActualPerformance] = new TPerf;
	assert(m_pPerfBank[m_nActualPerformance]);
	TPerf *pPB = m_pPerfBank[m_nActualPerformance];

	bool bResult = false;

	for (unsigned nTG = 0; nTG < CConfig::AllToneGenerators; nTG++)
	{
		CString PropertyName;

		PropertyName.Format ("BankNumber%u", nTG+1);
		pPB->nBankNumber[nTG] = Properties.GetNumber (PropertyName, 0);

		PropertyName.Format ("VoiceNumber%u", nTG+1);
		pPB->nVoiceNumber[nTG] = Properties.GetNumber (PropertyName, 1);
		if (pPB->nVoiceNumber[nTG] > 0)
		{
			pPB->nVoiceNumber[nTG]--;
		}

		PropertyName.Format ("MIDIChannel%u", nTG+1);
		unsigned nMIDIChannel = Properties.GetNumber (PropertyName, 0);
		if (nMIDIChannel == 0)
		{
			pPB->nMIDIChannel[nTG] = CMIDIDevice::Disabled;
		}
		else if (nMIDIChannel <= CMIDIDevice::Channels)
		{
			pPB->nMIDIChannel[nTG] = nMIDIChannel-1;
			bResult = true;
		}
		else
		{
			pPB->nMIDIChannel[nTG] = CMIDIDevice::OmniMode;
			bResult = true;
		}

		PropertyName.Format ("Volume%u", nTG+1);
		pPB->nVolume[nTG] = Properties.GetNumber (PropertyName, 100);

		PropertyName.Format ("Pan%u", nTG+1);
		pPB->nPan[nTG] = Properties.GetNumber (PropertyName, 64);

		PropertyName.Format ("Detune%u", nTG+1);
		pPB->nDetune[nTG] = Properties.GetSignedNumber (PropertyName, 0);

		PropertyName.Format ("Cutoff%u", nTG+1);
		pPB->nCutoff[nTG] = Properties.GetNumber (PropertyName, 99);

		PropertyName.Format ("Resonance%u", nTG+1);
		pPB->nResonance[nTG] = Properties.GetNumber (PropertyName, 0);

		PropertyName.Format ("NoteLimitLow%u", nTG+1);
		pPB->nNoteLimitLow[nTG] = Properties.GetNumber (PropertyName, 0);

		PropertyName.Format ("NoteLimitHigh%u", nTG+1);
		pPB->nNoteLimitHigh[nTG] = Properties.GetNumber (PropertyName, 127);

		PropertyName.Format ("NoteShift%u", nTG+1);
		pPB->nNoteShift[nTG] = Properties.GetSignedNumber (PropertyName, 0);

		PropertyName.Format ("ReverbSend%u", nTG+1);
		pPB->nReverbSend[nTG] = Properties.GetNumber (PropertyName, 50);
		
		PropertyName.Format ("PitchBendRange%u", nTG+1);
		pPB->nPitchBendRange[nTG] = Properties.GetNumber (PropertyName, 2);

		PropertyName.Format ("PitchBendStep%u", nTG+1);
		pPB->nPitchBendStep[nTG] = Properties.GetNumber (PropertyName, 0);

		PropertyName.Format ("PortamentoMode%u", nTG+1);
		pPB->nPortamentoMode[nTG] = Properties.GetNumber (PropertyName, 0);

		PropertyName.Format ("PortamentoGlissando%u", nTG+1);
		pPB->nPortamentoGlissando[nTG] = Properties.GetNumber (PropertyName, 0);

		PropertyName.Format ("PortamentoTime%u", nTG+1);
		pPB->nPortamentoTime[nTG] = Properties.GetNumber (PropertyName, 0);
		
		PropertyName.Format ("VoiceData%u", nTG+1); 
		pPB->nVoiceDataTxt[nTG] = Properties.GetString (PropertyName, "");
		
		PropertyName.Format ("MonoMode%u", nTG+1);
		pPB->bMonoMode[nTG] = Properties.GetNumber (PropertyName, 0) != 0;
				
		PropertyName.Format ("ModulationWheelRange%u", nTG+1);
		pPB->nModulationWheelRange[nTG] = Properties.GetNumber (PropertyName, 99); 
		
		PropertyName.Format ("ModulationWheelTarget%u", nTG+1);
		pPB->nModulationWheelTarget[nTG] = Properties.GetNumber (PropertyName, 1);
		
		PropertyName.Format ("FootControlRange%u", nTG+1);
		pPB->nFootControlRange[nTG] = Properties.GetNumber (PropertyName, 99); 
		
		PropertyName.Format ("FootControlTarget%u", nTG+1);
		pPB->nFootControlTarget[nTG] = Properties.GetNumber (PropertyName, 0);
		
		PropertyName.Format ("BreathControlRange%u", nTG+1);
		pPB->nBreathControlRange[nTG] = Properties.GetNumber (PropertyName, 99); 
		
		PropertyName.Format ("BreathControlTarget%u", nTG+1);
		pPB->nBreathControlTarget[nTG] = Properties.GetNumber (PropertyName, 0);
		
		PropertyName.Format ("AftertouchRange%u", nTG+1);
		pPB->nAftertouchRange[nTG] = Properties.GetNumber (PropertyName, 99); 
		
		PropertyName.Format ("AftertouchTarget%u", nTG+1);
		pPB->nAftertouchTarget[nTG] = Properties.GetNumber (PropertyName, 0);
		
		}

	pPB->bCompressorEnable = Properties.GetNumber ("CompressorEnable", 1) != 0;

	pPB->bReverbEnable = Properties.GetNumber ("ReverbEnable", 1) != 0;
	pPB->nReverbSize = Properties.GetNumber ("ReverbSize", 70);
	pPB->nReverbHighDamp = Properties.GetNumber ("ReverbHighDamp", 50);
	pPB->nReverbLowDamp = Properties.GetNumber ("ReverbLowDamp", 50);
	pPB->nReverbLowPass = Properties.GetNumber ("ReverbLowPass", 30);
	pPB->nReverbDiffusion = Properties.GetNumber ("ReverbDiffusion", 65);
	pPB->nReverbLevel = Properties.GetNumber ("ReverbLevel", 99);

	return bResult;
}

bool CPerformanceConfig::Save (void)
{
	std::string FileN = GetPerformanceFullFilePath(m_nActualPerformance);
	CPropertiesFatFsFile Properties (FileN.c_str(), m_pFileSystem);

	TPerf *pPB = m_pPerfBank[m_nActualPerformance];
	assert (pPB);

	Properties.RemoveAll ();

	for (unsigned nTG = 0; nTG < m_nToneGenerators; nTG++)
	{
		CString PropertyName;

		PropertyName.Format ("BankNumber%u", nTG+1);
		Properties.SetNumber (PropertyName, pPB->nBankNumber[nTG]);

		PropertyName.Format ("VoiceNumber%u", nTG+1);
		Properties.SetNumber (PropertyName, pPB->nVoiceNumber[nTG]+1);

		PropertyName.Format ("MIDIChannel%u", nTG+1);
		unsigned nMIDIChannel = pPB->nMIDIChannel[nTG];
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
		Properties.SetNumber (PropertyName, nMIDIChannel);

		PropertyName.Format ("Volume%u", nTG+1);
		Properties.SetNumber (PropertyName, pPB->nVolume[nTG]);

		PropertyName.Format ("Pan%u", nTG+1);
		Properties.SetNumber (PropertyName, pPB->nPan[nTG]);

		PropertyName.Format ("Detune%u", nTG+1);
		Properties.SetSignedNumber (PropertyName, pPB->nDetune[nTG]);

		PropertyName.Format ("Cutoff%u", nTG+1);
		Properties.SetNumber (PropertyName, pPB->nCutoff[nTG]);

		PropertyName.Format ("Resonance%u", nTG+1);
		Properties.SetNumber (PropertyName, pPB->nResonance[nTG]);

		PropertyName.Format ("NoteLimitLow%u", nTG+1);
		Properties.SetNumber (PropertyName, pPB->nNoteLimitLow[nTG]);

		PropertyName.Format ("NoteLimitHigh%u", nTG+1);
		Properties.SetNumber (PropertyName, pPB->nNoteLimitHigh[nTG]);

		PropertyName.Format ("NoteShift%u", nTG+1);
		Properties.SetSignedNumber (PropertyName, pPB->nNoteShift[nTG]);

		PropertyName.Format ("ReverbSend%u", nTG+1);
		Properties.SetNumber (PropertyName, pPB->nReverbSend[nTG]);
		
		PropertyName.Format ("PitchBendRange%u", nTG+1);
		Properties.SetNumber (PropertyName, pPB->nPitchBendRange[nTG]);

		PropertyName.Format ("PitchBendStep%u", nTG+1);
		Properties.SetNumber (PropertyName, pPB->nPitchBendStep[nTG]);

		PropertyName.Format ("PortamentoMode%u", nTG+1);
		Properties.SetNumber (PropertyName, pPB->nPortamentoMode[nTG]);

		PropertyName.Format ("PortamentoGlissando%u", nTG+1);
		Properties.SetNumber (PropertyName, pPB->nPortamentoGlissando[nTG]);

		PropertyName.Format ("PortamentoTime%u", nTG+1);
		Properties.SetNumber (PropertyName, pPB->nPortamentoTime[nTG]);
		
		PropertyName.Format ("VoiceData%u", nTG+1);
		char *cstr = &pPB->nVoiceDataTxt[nTG][0];
		Properties.SetString (PropertyName, cstr);
		
		PropertyName.Format ("MonoMode%u", nTG+1);
		Properties.SetNumber (PropertyName, pPB->bMonoMode[nTG] ? 1 : 0);
				
		PropertyName.Format ("ModulationWheelRange%u", nTG+1);
		Properties.SetNumber (PropertyName, pPB->nModulationWheelRange[nTG]);
	
		PropertyName.Format ("ModulationWheelTarget%u", nTG+1);
		Properties.SetNumber (PropertyName, pPB->nModulationWheelTarget[nTG]);	
			
		PropertyName.Format ("FootControlRange%u", nTG+1);
		Properties.SetNumber (PropertyName, pPB->nFootControlRange[nTG]);	
		
		PropertyName.Format ("FootControlTarget%u", nTG+1);
		Properties.SetNumber (PropertyName, pPB->nFootControlTarget[nTG]);	
		
		PropertyName.Format ("BreathControlRange%u", nTG+1);
		Properties.SetNumber (PropertyName, pPB->nBreathControlRange[nTG]);	
		
		PropertyName.Format ("BreathControlTarget%u", nTG+1);
		Properties.SetNumber (PropertyName, pPB->nBreathControlTarget[nTG]);	
		
		PropertyName.Format ("AftertouchRange%u", nTG+1);
		Properties.SetNumber (PropertyName, pPB->nAftertouchRange[nTG]);	
		
		PropertyName.Format ("AftertouchTarget%u", nTG+1);
		Properties.SetNumber (PropertyName, pPB->nAftertouchTarget[nTG]);			

		}

	Properties.SetNumber ("CompressorEnable", pPB->bCompressorEnable ? 1 : 0);

	Properties.SetNumber ("ReverbEnable", pPB->bReverbEnable ? 1 : 0);
	Properties.SetNumber ("ReverbSize", pPB->nReverbSize);
	Properties.SetNumber ("ReverbHighDamp", pPB->nReverbHighDamp);
	Properties.SetNumber ("ReverbLowDamp", pPB->nReverbLowDamp);
	Properties.SetNumber ("ReverbLowPass", pPB->nReverbLowPass);
	Properties.SetNumber ("ReverbDiffusion", pPB->nReverbDiffusion);
	Properties.SetNumber ("ReverbLevel", pPB->nReverbLevel);

	return Properties.Save ();
}

unsigned CPerformanceConfig::GetBankNumber (unsigned nTG) const
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nBankNumber[nTG];
}

unsigned CPerformanceConfig::GetVoiceNumber (unsigned nTG) const
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nVoiceNumber[nTG];
}

unsigned CPerformanceConfig::GetMIDIChannel (unsigned nTG) const
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nMIDIChannel[nTG];
}

unsigned CPerformanceConfig::GetVolume (unsigned nTG) const
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nVolume[nTG];
}

unsigned CPerformanceConfig::GetPan (unsigned nTG) const
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nPan[nTG];
}

int CPerformanceConfig::GetDetune (unsigned nTG) const
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nDetune[nTG];
}

unsigned CPerformanceConfig::GetCutoff (unsigned nTG) const
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nCutoff[nTG];
}

unsigned CPerformanceConfig::GetResonance (unsigned nTG) const
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nResonance[nTG];
}

unsigned CPerformanceConfig::GetNoteLimitLow (unsigned nTG) const
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nNoteLimitLow[nTG];
}

unsigned CPerformanceConfig::GetNoteLimitHigh (unsigned nTG) const
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nNoteLimitHigh[nTG];
}

int CPerformanceConfig::GetNoteShift (unsigned nTG) const
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nNoteShift[nTG];
}

unsigned CPerformanceConfig::GetReverbSend (unsigned nTG) const
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nReverbSend[nTG];
}

void CPerformanceConfig::SetBankNumber (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nBankNumber[nTG] = nValue;
}

void CPerformanceConfig::SetVoiceNumber (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nVoiceNumber[nTG] = nValue;
}

void CPerformanceConfig::SetMIDIChannel (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nMIDIChannel[nTG] = nValue;
}

void CPerformanceConfig::SetVolume (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nVolume[nTG] = nValue;
}

void CPerformanceConfig::SetPan (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nPan[nTG] = nValue;
}

void CPerformanceConfig::SetDetune (int nValue, unsigned nTG)
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nDetune[nTG] = nValue;
}

void CPerformanceConfig::SetCutoff (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nCutoff[nTG] = nValue;
}

void CPerformanceConfig::SetResonance (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nResonance[nTG] = nValue;
}

void CPerformanceConfig::SetNoteLimitLow (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nNoteLimitLow[nTG] = nValue;
}

void CPerformanceConfig::SetNoteLimitHigh (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nNoteLimitHigh[nTG] = nValue;
}

void CPerformanceConfig::SetNoteShift (int nValue, unsigned nTG)
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nNoteShift[nTG] = nValue;
}

void CPerformanceConfig::SetReverbSend (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nReverbSend[nTG] = nValue;
}

bool CPerformanceConfig::GetCompressorEnable (void) const
{
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->bCompressorEnable;
}

bool CPerformanceConfig::GetReverbEnable (void) const
{
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->bReverbEnable;
}

unsigned CPerformanceConfig::GetReverbSize (void) const
{
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nReverbSize;
}

unsigned CPerformanceConfig::GetReverbHighDamp (void) const
{
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nReverbHighDamp;
}

unsigned CPerformanceConfig::GetReverbLowDamp (void) const
{
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nReverbLowDamp;
}

unsigned CPerformanceConfig::GetReverbLowPass (void) const
{
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nReverbLowPass;
}

unsigned CPerformanceConfig::GetReverbDiffusion (void) const
{
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nReverbDiffusion;
}

unsigned CPerformanceConfig::GetReverbLevel (void) const
{
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nReverbLevel;
}

void CPerformanceConfig::SetCompressorEnable (bool bValue)
{
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->bCompressorEnable = bValue;
}

void CPerformanceConfig::SetReverbEnable (bool bValue)
{
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->bReverbEnable = bValue;
}

void CPerformanceConfig::SetReverbSize (unsigned nValue)
{
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nReverbSize = nValue;
}

void CPerformanceConfig::SetReverbHighDamp (unsigned nValue)
{
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nReverbHighDamp = nValue;
}

void CPerformanceConfig::SetReverbLowDamp (unsigned nValue)
{
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nReverbLowDamp = nValue;
}

void CPerformanceConfig::SetReverbLowPass (unsigned nValue)
{
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nReverbLowPass = nValue;
}

void CPerformanceConfig::SetReverbDiffusion (unsigned nValue)
{
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nReverbDiffusion = nValue;
}

void CPerformanceConfig::SetReverbLevel (unsigned nValue)
{
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nReverbLevel = nValue;
}
// Pitch bender and portamento:
void CPerformanceConfig::SetPitchBendRange (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nPitchBendRange[nTG] = nValue;
}

unsigned CPerformanceConfig::GetPitchBendRange (unsigned nTG) const
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nPitchBendRange[nTG];
}


void CPerformanceConfig::SetPitchBendStep (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nPitchBendStep[nTG] = nValue;
}

unsigned CPerformanceConfig::GetPitchBendStep (unsigned nTG) const
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nPitchBendStep[nTG];
}


void CPerformanceConfig::SetPortamentoMode (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nPortamentoMode[nTG] = nValue;
}

unsigned CPerformanceConfig::GetPortamentoMode (unsigned nTG) const
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nPortamentoMode[nTG];
}


void CPerformanceConfig::SetPortamentoGlissando (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nPortamentoGlissando[nTG] = nValue;
}

unsigned CPerformanceConfig::GetPortamentoGlissando (unsigned nTG) const
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nPortamentoGlissando[nTG];
}


void CPerformanceConfig::SetPortamentoTime (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nPortamentoTime[nTG] = nValue;
}

unsigned CPerformanceConfig::GetPortamentoTime (unsigned nTG) const
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nPortamentoTime[nTG];
}

void CPerformanceConfig::SetMonoMode (bool bValue, unsigned nTG)
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->bMonoMode[nTG] = bValue;
}

bool CPerformanceConfig::GetMonoMode (unsigned nTG) const
{
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->bMonoMode[nTG];
}

void CPerformanceConfig::SetModulationWheelRange (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nModulationWheelRange[nTG] = nValue;
}

unsigned CPerformanceConfig::GetModulationWheelRange (unsigned nTG) const
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nModulationWheelRange[nTG];
}

void CPerformanceConfig::SetModulationWheelTarget (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nModulationWheelTarget[nTG] = nValue;
}

unsigned CPerformanceConfig::GetModulationWheelTarget (unsigned nTG) const
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nModulationWheelTarget[nTG];
}

void CPerformanceConfig::SetFootControlRange (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nFootControlRange[nTG] = nValue;
}

unsigned CPerformanceConfig::GetFootControlRange (unsigned nTG) const
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nFootControlRange[nTG];
}

void CPerformanceConfig::SetFootControlTarget (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nFootControlTarget[nTG] = nValue;
}

unsigned CPerformanceConfig::GetFootControlTarget (unsigned nTG) const
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nFootControlTarget[nTG];
}

void CPerformanceConfig::SetBreathControlRange (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nBreathControlRange[nTG] = nValue;
}

unsigned CPerformanceConfig::GetBreathControlRange (unsigned nTG) const
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nBreathControlRange[nTG];
}

void CPerformanceConfig::SetBreathControlTarget (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nBreathControlTarget[nTG] = nValue;
}

unsigned CPerformanceConfig::GetBreathControlTarget (unsigned nTG) const
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nBreathControlTarget[nTG];
}

void CPerformanceConfig::SetAftertouchRange (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nAftertouchRange[nTG] = nValue;
}

unsigned CPerformanceConfig::GetAftertouchRange (unsigned nTG) const
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nAftertouchRange[nTG];
}

void CPerformanceConfig::SetAftertouchTarget (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nAftertouchTarget[nTG] = nValue;
}

unsigned CPerformanceConfig::GetAftertouchTarget (unsigned nTG) const
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	return m_pPerfBank[m_nActualPerformance]->nAftertouchTarget[nTG];
}

void CPerformanceConfig::SetVoiceDataToTxt (const uint8_t *pData, unsigned nTG)  
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	m_pPerfBank[m_nActualPerformance]->nVoiceDataTxt[nTG] = "";
	char nDtoH[]="0123456789ABCDEF";
	for (int i = 0; i < NUM_VOICE_PARAM; i++)
	{
	m_pPerfBank[m_nActualPerformance]->nVoiceDataTxt[nTG]  += nDtoH[(pData[i] & 0xF0)/16];
	m_pPerfBank[m_nActualPerformance]->nVoiceDataTxt[nTG]  += nDtoH[pData[i] & 0x0F]  ;
	if ( i < (NUM_VOICE_PARAM-1)  ) 
    	{
     	 m_pPerfBank[m_nActualPerformance]->nVoiceDataTxt[nTG] += " ";
    	}
	}
}

uint8_t *CPerformanceConfig::GetVoiceDataFromTxt (unsigned nTG) 
{
	assert (nTG < CConfig::AllToneGenerators);
	assert (m_pPerfBank[m_nActualPerformance]);
	static uint8_t pData[NUM_VOICE_PARAM];
	std::string nHtoD="0123456789ABCDEF";
	 
 	for (int i=0; i<NUM_VOICE_PARAM * 3; i=i+3)
	{
  	pData[i/3] = ((nHtoD.find(toupper(m_pPerfBank[m_nActualPerformance]->nVoiceDataTxt[nTG][i]),0) * 16 + nHtoD.find(toupper(m_pPerfBank[m_nActualPerformance]->nVoiceDataTxt[nTG][i+1]),0))) & 0xFF ;
	} 
  
	return pData;
}

bool CPerformanceConfig::VoiceDataFilled(unsigned nTG) 
{
	assert (m_pPerfBank[m_nActualPerformance]);
	return (strcmp(m_pPerfBank[m_nActualPerformance]->nVoiceDataTxt[nTG].c_str(),"") != 0) ;
}

std::string CPerformanceConfig::GetPerformanceFileName(unsigned nID)
{
	assert (nID < NUM_PERFORMANCES);
	std::string FileN = "";
	if ((m_nPerformanceBank==0) && (nID == 0)) // in order to assure retrocompatibility
	{
		FileN += DEFAULT_PERFORMANCE_FILENAME;
	}
	else
	{
		// Build up from the index, "_", performance name, and ".ini"
		// NB: Index on disk = index in memory + 1
		std::string nIndex = "000000";
		nIndex += std::to_string(nID+1);
		nIndex = nIndex.substr(nIndex.length()-6,6);
		FileN += nIndex;
		FileN += "_";
		FileN += m_PerformanceFileName[nID];
		FileN += ".ini";
	}
	return FileN;
}

std::string CPerformanceConfig::GetPerformanceFullFilePath(unsigned nID)
{
	assert (nID < NUM_PERFORMANCES);
	std::string FileN = "SD:/";
	if ((m_nPerformanceBank == 0) && (nID == 0))
	{
		// Special case for the legacy Bank 1/Default performance
		FileN += DEFAULT_PERFORMANCE_FILENAME;
	}
	else
	{
		if (m_bPerformanceDirectoryExists)
		{
			FileN += PERFORMANCE_DIR;
			FileN += AddPerformanceBankDirName(m_nPerformanceBank);
			FileN += "/";
			FileN += GetPerformanceFileName(nID);
		}
	}
	return FileN;
}

std::string CPerformanceConfig::GetPerformanceName(unsigned nID)
{
	assert (nID < NUM_PERFORMANCES);
	if ((m_nPerformanceBank==0) && (nID == 0)) // in order to assure retrocompatibility
	{
		return DEFAULT_PERFORMANCE_NAME;
	}
	else
	{
		return m_PerformanceFileName[nID];
	}
}

unsigned CPerformanceConfig::GetLastPerformance()
{
	return m_nLastPerformance;
}

unsigned CPerformanceConfig::GetLastPerformanceBank()
{
	return m_nLastPerformanceBank;
}

unsigned CPerformanceConfig::GetActualPerformanceID()
{
	return m_nActualPerformance;
}

void CPerformanceConfig::SetActualPerformanceID(unsigned nID)
{
	assert (nID < NUM_PERFORMANCES);
	m_nActualPerformance = nID;
}

unsigned CPerformanceConfig::GetActualPerformanceBankID()
{
	return m_nActualPerformanceBank;
}

void CPerformanceConfig::SetActualPerformanceBankID(unsigned nBankID)
{
	assert (nBankID < NUM_PERFORMANCE_BANKS);
	m_nActualPerformanceBank = nBankID;
}

bool CPerformanceConfig::GetInternalFolderOk()
{
	return m_bPerformanceDirectoryExists;
}

bool CPerformanceConfig::IsValidPerformance(unsigned nID)
{
	if (nID < NUM_PERFORMANCES)
	{
		if (!m_PerformanceFileName[nID].empty())
		{
			return true;
		}
	}
	
	return false;
}


bool CPerformanceConfig::CheckFreePerformanceSlot(void)
{
	if (m_nLastPerformance < NUM_PERFORMANCES-1)
	{
		// There is a free slot...
		return true;
	}
	else
	{
		return false;
	}
}

bool CPerformanceConfig::CreateNewPerformanceFile(void)
{
	if (!m_bPerformanceDirectoryExists)
	{
		// Nothing can be done if there is no performance directory
		LOGNOTE("Performance directory does not exist");
		return false;
	}
	if (m_nLastPerformance >= NUM_PERFORMANCES) {
		// No space left for new performances
		LOGWARN ("No space left for new performance");
		return false;
	}
	
	// Note: New performances are created at the end of the currently selected bank.
	//       Sould we default to a new bank just for user-performances?
	//
	//       There is the possibility of MIDI changing the Bank Number and the user
	//       not spotting the bank has changed...
	//
	//       Another option would be to find empty slots in the current bank
	//       rather than always add at the end.
	//
	//       Sorting this out is left for a future update.

	std::string sPerformanceName = NewPerformanceName;
	NewPerformanceName=""; 
	unsigned nNewPerformance = m_nLastPerformance + 1;
	std::string nFileName;
	std::string nPath;
	std::string nIndex = "000000";
	nIndex += std::to_string(nNewPerformance+1);  // Index on disk = index in memory+1
	nIndex = nIndex.substr(nIndex.length()-6,6);
	

	nFileName = nIndex;
	nFileName += "_";
	if (strcmp(sPerformanceName.c_str(),"") == 0)
	{
		nFileName += "Perf";
		nFileName += nIndex;
	}		
	else
	{
		nFileName +=sPerformanceName.substr(0,14);
	}
	nFileName += ".ini";
	m_PerformanceFileName[nNewPerformance]= sPerformanceName;
	
	nPath = "SD:/" ;
	nPath += PERFORMANCE_DIR;
	nPath += AddPerformanceBankDirName(m_nPerformanceBank);
	nPath += "/";
	nFileName = nPath + nFileName;
	
	FIL File;
	FRESULT Result = f_open (&File, nFileName.c_str(), FA_WRITE | FA_CREATE_ALWAYS);
	if (Result != FR_OK)
	{
		m_PerformanceFileName[nNewPerformance]=nullptr;
		return false;
	}

	if (f_close (&File) != FR_OK)
	{
		m_PerformanceFileName[nNewPerformance]=nullptr;
		return false;
	}
	
	m_nLastPerformance = nNewPerformance;
	m_nActualPerformance = nNewPerformance;
	
	// This has no actual data associated with it until the caller
	// Sets the parameters and then saves them.
	m_pPerfBank[m_nActualPerformance] = new TPerf;	
	
	return true;
}

bool CPerformanceConfig::ListPerformances()
{
	// Clear any existing lists of performances
	for (unsigned i=0; i<NUM_PERFORMANCES; i++)
	{
		m_PerformanceFileName[i].clear();
		m_pPerfBank[i] = nullptr;
	}
	m_nLastPerformance=0;
	if (m_nPerformanceBank == 0)
	{
		// The first bank is the default performance directory
	   	m_PerformanceFileName[0]=DEFAULT_PERFORMANCE_NAME; // in order to assure retrocompatibility
	}
	
	if (m_bPerformanceDirectoryExists)
	{
		DIR Directory;
		FILINFO FileInfo;
		FRESULT Result;
		std::string PerfDir = "SD:/" PERFORMANCE_DIR + AddPerformanceBankDirName(m_nPerformanceBank);
#ifdef VERBOSE_DEBUG
		LOGNOTE("Listing Performances from %s", PerfDir.c_str());
#endif
		Result = f_opendir (&Directory, PerfDir.c_str());
		if (Result != FR_OK)
		{
			return false;
		}
		unsigned nPIndex;

		Result = f_findfirst (&Directory, &FileInfo, PerfDir.c_str(), "*.ini");
		for (unsigned i = 0; Result == FR_OK && FileInfo.fname[0]; i++)
		{
			if (!(FileInfo.fattrib & (AM_HID | AM_SYS)))  
			{
				std::string OriFileName = FileInfo.fname;
				size_t nLen = OriFileName.length();
				if (   nLen > 8 && nLen <26	 && strcmp(OriFileName.substr(6,1).c_str(), "_")==0)
				{
					// Note: m_nLastPerformance - refers to the number (index) of the last performance in memory,
					//       which includes a default performance.
					//
					//       Filenames on the disk start from 1 to match what the user might see in MIDI.
					//       So this means that actually file 000001_ will correspond to index position [0].
					//       For the default bank though, ID 1 is the default performance, so will already exist.
					//          m_PerformanceFileName[0] = default performance (file 000001)
					//          m_PerformanceFileName[1] = first available on-disk performance (file 000002)
					//
					// Note2: filenames assume 6 digits, underscore, name, finally ".ini"
					//        i.e.   123456_Performance Name.ini
					//
					nPIndex=stoi(OriFileName.substr(0,6));
					if ((nPIndex < 1) || (nPIndex >= (NUM_PERFORMANCES+1)))
					{
						// Index is out of range - skip to next file
						LOGNOTE ("Performance number out of range: %s (%d to %d)", FileInfo.fname, 1, NUM_PERFORMANCES);
					}
					else
					{
						// Convert from "user facing" 1..indexed number to internal 0..indexed
						nPIndex = nPIndex-1;
						if (m_PerformanceFileName[nPIndex].empty())
						{
							if(nPIndex > m_nLastPerformance)
							{
								m_nLastPerformance=nPIndex;
							}

							std::string FileName = OriFileName.substr(0,OriFileName.length()-4).substr(7,14);

							m_PerformanceFileName[nPIndex] = FileName;
#ifdef VERBOSE_DEBUG
							LOGNOTE ("Loading performance %s (%d, %s)", OriFileName.c_str(), nPIndex, FileName.c_str());
#endif
						}
						else
						{
							LOGNOTE ("Duplicate performance %s", OriFileName.c_str());
						}
					}
				}
			}

			Result = f_findnext (&Directory, &FileInfo);
		}
		f_closedir (&Directory);
	}
	
	return true;
}

void CPerformanceConfig::SetNewPerformance (unsigned nID)
{
	assert (nID < NUM_PERFORMANCES);
	m_nActualPerformance=nID;
	std::string FileN = GetPerformanceFullFilePath(nID);
#ifdef VERBOSE_DEBUG
	LOGNOTE("Selecting Performance: %d (%s)", nID+1, FileN.c_str());
#endif
}

unsigned CPerformanceConfig::FindFirstPerformance (void)
{
	for (int nID=0; nID < NUM_PERFORMANCES; nID++)
	{
		if (IsValidPerformance(nID))
		{
			return nID;
		}
	}

	return 0; // Even though 0 is a valid performance, not much else to do
}

std::string CPerformanceConfig::GetNewPerformanceDefaultName(void)
{
	std::string nIndex = "000000";
	nIndex += std::to_string(m_nLastPerformance+1+1); // Convert from internal 0.. index to a file-based 1.. index to show the user
	nIndex = nIndex.substr(nIndex.length()-6,6);
	return "Perf" + nIndex;
}

void CPerformanceConfig::SetNewPerformanceName(const std::string &Name)
{
	NewPerformanceName = Name.substr(0, Name.find_last_not_of(' ') + 1);
}

bool CPerformanceConfig::DeletePerformance(unsigned nID)
{
	if (!m_bPerformanceDirectoryExists)
	{
		// Nothing can be done if there is no performance directory
		LOGNOTE("Performance directory does not exist");
		return false;
	}
	bool bOK = false;
	if((m_nPerformanceBank == 0) && (nID == 0)){return bOK;} // default (performance.ini at root directory) can't be deleted
	DIR Directory;
	FILINFO FileInfo;
	std::string FileN = "SD:/";
	FileN += PERFORMANCE_DIR;
	FileN += AddPerformanceBankDirName(m_nPerformanceBank);

	// Remove from cache
	m_pPerfBank[nID] = nullptr;
	
	FRESULT Result = f_findfirst (&Directory, &FileInfo, FileN.c_str(), GetPerformanceFileName(nID).c_str());
	if (Result == FR_OK && FileInfo.fname[0])
	{
		FileN += "/";
		FileN += GetPerformanceFileName(nID);
		Result=f_unlink (FileN.c_str());
		if (Result == FR_OK)
		{
			SetNewPerformance(0);
			m_nActualPerformance =0;
			//nMenuSelectedPerformance=0;
			m_PerformanceFileName[nID].clear();
			// If this was the last performance in the bank...
			if (nID == m_nLastPerformance)
			{
				do
				{
					// Find the new last performance
					m_nLastPerformance--;
				} while (!IsValidPerformance(m_nLastPerformance) && (m_nLastPerformance > 0));
			}
			bOK=true;
		}
		else
		{
			LOGNOTE ("Failed to delete %s", FileN.c_str());
		}
	}
	return bOK;
}

bool CPerformanceConfig::ListPerformanceBanks()
{
	m_nPerformanceBank = 0;
	m_nLastPerformance = 0;
	m_nLastPerformanceBank = 0;

	// Open performance directory
	DIR Directory;
	FILINFO FileInfo;
	FRESULT Result;
	Result = f_opendir (&Directory, "SD:/" PERFORMANCE_DIR);
	if (Result != FR_OK)
	{
		// No performance directory, so no performance banks.
		// So nothing else to do here
		LOGNOTE ("No performance banks detected");
		m_bPerformanceDirectoryExists = false;
		return false;
	}

	unsigned nNumBanks = 0;
	
	// Add in the default performance directory as the first bank
	m_PerformanceBankName[0] = DEFAULT_PERFORMANCE_BANK_NAME;
	nNumBanks = 1;
	m_nLastPerformanceBank = 0;

	// List directories with names in format 01_Perf Bank Name
	Result = f_findfirst (&Directory, &FileInfo, "SD:/" PERFORMANCE_DIR, "*");
	for (unsigned i = 0; Result == FR_OK && FileInfo.fname[0]; i++)
	{
		// Check to see if it is a directory
		if ((FileInfo.fattrib & AM_DIR) != 0)
		{
			// Looking for Performance banks of the form: 01_Perf Bank Name
			// So positions 0,1,2 = decimal digit
			//    position  3   = "_"
			//    positions 4.. = actual name
			//
			std::string OriFileName = FileInfo.fname;
			size_t nLen = OriFileName.length();
			if (   nLen > 4 && nLen <26	 && strcmp(OriFileName.substr(3,1).c_str(), "_")==0)
			{
				unsigned nBankIndex = stoi(OriFileName.substr(0,3));
				// Recall user index numbered 002..NUM_PERFORMANCE_BANKS
				// NB: Bank 001 is reserved for the default performance directory
				if ((nBankIndex > 0) && (nBankIndex <= NUM_PERFORMANCE_BANKS))
				{
					// Convert from "user facing" 1..indexed number to internal 0..indexed
					nBankIndex = nBankIndex-1;
					if (m_PerformanceBankName[nBankIndex].empty())
					{
						std::string BankName = OriFileName.substr(4,nLen);

						m_PerformanceBankName[nBankIndex] = BankName;
#ifdef VERBOSE_DEBUG
						LOGNOTE ("Found performance bank %s (%d, %s)", OriFileName.c_str(), nBankIndex, BankName.c_str());
#endif
						nNumBanks++;
						if (nBankIndex > m_nLastPerformanceBank)
						{
							m_nLastPerformanceBank = nBankIndex;
						}
					}
					else
					{
						LOGNOTE ("Duplicate Performance Bank: %s", FileInfo.fname);
						if (nBankIndex==0)
						{
							LOGNOTE ("(Bank 001 is the default performance directory)");
						}
					}
				}
				else
				{
					LOGNOTE ("Performance Bank number out of range: %s (%d to %d)", FileInfo.fname, 1, NUM_PERFORMANCE_BANKS);
				}
			}
			else
			{
#ifdef VERBOSE_DEBUG
				LOGNOTE ("Skipping: %s", FileInfo.fname);
#endif
			}
		}
		
		Result = f_findnext (&Directory, &FileInfo);
	}
	
	if (nNumBanks > 0)
	{
		LOGNOTE ("Number of Performance Banks: %d (last = %d)", nNumBanks, m_nLastPerformanceBank+1);
	}
	
	f_closedir (&Directory);
	return true;
}

void CPerformanceConfig::SetNewPerformanceBank(unsigned nBankID)
{
	assert (nBankID < NUM_PERFORMANCE_BANKS);
	if (IsValidPerformanceBank(nBankID))
	{
#ifdef VERBOSE_DEBUG
		LOGNOTE("Selecting Performance Bank: %d", nBankID+1);
#endif
		m_nPerformanceBank = nBankID;
		m_nActualPerformanceBank = nBankID;
		ListPerformances();
	}
	else
	{
#ifdef VERBOSE_DEBUG
		LOGNOTE("Not selecting invalid Performance Bank: %d", nBankID+1);
#endif
	}
}

unsigned CPerformanceConfig::GetPerformanceBank(void)
{
	return m_nPerformanceBank;
}

std::string CPerformanceConfig::GetPerformanceBankName(unsigned nBankID)
{
	assert (nBankID < NUM_PERFORMANCE_BANKS);
	if (IsValidPerformanceBank(nBankID))
	{
		return m_PerformanceBankName[nBankID];
	}
	else
	{
		return DEFAULT_PERFORMANCE_BANK_NAME;
	}
}

std::string CPerformanceConfig::AddPerformanceBankDirName(unsigned nBankID)
{
	assert (nBankID < NUM_PERFORMANCE_BANKS);
	if (IsValidPerformanceBank(nBankID))
	{
		// Performance Banks directories in format "001_Bank Name"
		std::string Index;
		if (nBankID == 0)
		{
			// Legacy: Bank 1 is the default performance directory
			return "";
		}

		if (nBankID < 9)
		{
			Index = "00" + std::to_string(nBankID+1);
		}
		else if (nBankID < 99)
		{
			Index = "0" + std::to_string(nBankID+1);
		}
		else
		{
			Index = std::to_string(nBankID+1);
		}

		return "/" + Index + "_" + m_PerformanceBankName[nBankID];
	}
	else
	{
		return "";
	}
}

bool CPerformanceConfig::IsValidPerformanceBank(unsigned nBankID)
{
	if (nBankID >= NUM_PERFORMANCE_BANKS) {
		return false;
	}
	if (m_PerformanceBankName[nBankID].empty())
	{
		return false;
	}
	return true;
}
