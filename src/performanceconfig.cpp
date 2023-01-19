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
#include <cstring> 
#include <algorithm> 
#include <sstream>

CPerformanceConfig::CPerformanceConfig (FATFS *pFileSystem)
:	m_Properties ("performance.ini", pFileSystem)
{
	m_pFileSystem = pFileSystem; 
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

		PropertyName.Format ("Cutoff%u", nTG+1);
		m_nCutoff[nTG] = m_Properties.GetNumber (PropertyName, 99);

		PropertyName.Format ("Resonance%u", nTG+1);
		m_nResonance[nTG] = m_Properties.GetNumber (PropertyName, 0);

		PropertyName.Format ("NoteLimitLow%u", nTG+1);
		m_nNoteLimitLow[nTG] = m_Properties.GetNumber (PropertyName, 0);

		PropertyName.Format ("NoteLimitHigh%u", nTG+1);
		m_nNoteLimitHigh[nTG] = m_Properties.GetNumber (PropertyName, 127);

		PropertyName.Format ("NoteShift%u", nTG+1);
		m_nNoteShift[nTG] = m_Properties.GetSignedNumber (PropertyName, 0);

		PropertyName.Format ("ReverbSend%u", nTG+1);
		m_nReverbSend[nTG] = m_Properties.GetNumber (PropertyName, 50);
		
		PropertyName.Format ("PitchBendRange%u", nTG+1);
		m_nPitchBendRange[nTG] = m_Properties.GetNumber (PropertyName, 2);

		PropertyName.Format ("PitchBendStep%u", nTG+1);
		m_nPitchBendStep[nTG] = m_Properties.GetNumber (PropertyName, 0);

		PropertyName.Format ("PortamentoMode%u", nTG+1);
		m_nPortamentoMode[nTG] = m_Properties.GetNumber (PropertyName, 0);

		PropertyName.Format ("PortamentoGlissando%u", nTG+1);
		m_nPortamentoGlissando[nTG] = m_Properties.GetNumber (PropertyName, 0);

		PropertyName.Format ("PortamentoTime%u", nTG+1);
		m_nPortamentoTime[nTG] = m_Properties.GetNumber (PropertyName, 0);
		
		PropertyName.Format ("VoiceData%u", nTG+1); 
		m_nVoiceDataTxt[nTG] = m_Properties.GetString (PropertyName, "");
		
		PropertyName.Format ("MonoMode%u", nTG+1);
		m_bMonoMode[nTG] = m_Properties.GetNumber (PropertyName, 0) != 0;
				
		PropertyName.Format ("ModulationWheelRange%u", nTG+1);
		m_nModulationWheelRange[nTG] = m_Properties.GetNumber (PropertyName, 99); 
		
		PropertyName.Format ("ModulationWheelTarget%u", nTG+1);
		m_nModulationWheelTarget[nTG] = m_Properties.GetNumber (PropertyName, 1);
		
		PropertyName.Format ("FootControlRange%u", nTG+1);
		m_nFootControlRange[nTG] = m_Properties.GetNumber (PropertyName, 99); 
		
		PropertyName.Format ("FootControlTarget%u", nTG+1);
		m_nFootControlTarget[nTG] = m_Properties.GetNumber (PropertyName, 0);
		
		PropertyName.Format ("BreathControlRange%u", nTG+1);
		m_nBreathControlRange[nTG] = m_Properties.GetNumber (PropertyName, 99); 
		
		PropertyName.Format ("BreathControlTarget%u", nTG+1);
		m_nBreathControlTarget[nTG] = m_Properties.GetNumber (PropertyName, 0);
		
		PropertyName.Format ("AftertouchRange%u", nTG+1);
		m_nAftertouchRange[nTG] = m_Properties.GetNumber (PropertyName, 99); 
		
		PropertyName.Format ("AftertouchTarget%u", nTG+1);
		m_nAftertouchTarget[nTG] = m_Properties.GetNumber (PropertyName, 0);
		
		}

	m_bCompressorEnable = m_Properties.GetNumber ("CompressorEnable", 1) != 0;

	m_bReverbEnable = m_Properties.GetNumber ("ReverbEnable", 1) != 0;
	m_nReverbSize = m_Properties.GetNumber ("ReverbSize", 70);
	m_nReverbHighDamp = m_Properties.GetNumber ("ReverbHighDamp", 50);
	m_nReverbLowDamp = m_Properties.GetNumber ("ReverbLowDamp", 50);
	m_nReverbLowPass = m_Properties.GetNumber ("ReverbLowPass", 30);
	m_nReverbDiffusion = m_Properties.GetNumber ("ReverbDiffusion", 65);
	m_nReverbLevel = m_Properties.GetNumber ("ReverbLevel", 99);

#ifdef MIXING_CONSOLE_ENABLE
	this->m_bFXTubeEnable = this->m_Properties.GetNumber("FXTubeEnable", 1);
	this->m_nFXTubeOverdrive = this->m_Properties.GetNumber("FXTubeOverdrive", 10);
	this->m_bFXChorusEnable = this->m_Properties.GetNumber("FXChorusEnable", 1);
	this->m_nFXChorusRate = this->m_Properties.GetNumber("FXChorusRate", 50);
	this->m_nFXChorusDepth = this->m_Properties.GetNumber("FXChorusDepth", 50);
	this->m_bFXFlangerEnable = this->m_Properties.GetNumber("FXFlangerEnable", 1);
	this->m_nFXFlangerRate = this->m_Properties.GetNumber("FXFlangerRate", 15);
	this->m_nFXFlangerDepth = this->m_Properties.GetNumber("FXFlangerDepth", 10);
	this->m_nFXFlangerFeedback = this->m_Properties.GetNumber("FXFlangerFeedback", 20);
	this->m_bFXOrbitoneEnable = this->m_Properties.GetNumber("FXOrbitoneEnable", 1);
	this->m_nFXOrbitoneRate = this->m_Properties.GetNumber("FXOrbitoneRate", 40);
	this->m_nFXOrbitoneDepth = this->m_Properties.GetNumber("FXOrbitoneDepth", 50);
	this->m_bFXPhaserEnable = this->m_Properties.GetNumber("FXPhaserEnable", 1);
	this->m_nFXPhaserRate = this->m_Properties.GetNumber("FXPhaserRate", 5);
	this->m_nFXPhaserDepth = this->m_Properties.GetNumber("FXPhaserDepth", 99);
	this->m_nFXPhaserFeedback = this->m_Properties.GetNumber("FXPhaserFeedback", 50);
	this->m_nFXPhaserNbStages = this->m_Properties.GetNumber("FXPhaserNbStages", 12);
	this->m_bFXDelayEnable = this->m_Properties.GetNumber("FXDelayEnable", 1);
	this->m_nFXDelayLeftDelayTime = this->m_Properties.GetNumber("FXDelayLeftDelayTime", 15);
	this->m_nFXDelayRightDelayTime = this->m_Properties.GetNumber("FXDelayRightDelayTime", 22);
	this->m_nFXDelayFeedback = this->m_Properties.GetNumber("FXDelayFeedback", 35);
	this->m_bFXShimmerReverbEnable = this->m_Properties.GetNumber("FXShimmerReverbEnable", 1);
	this->m_nFXShimmerReverbInputGain = this->m_Properties.GetNumber("FXShimmerReverbInputGain", 30);
	this->m_nFXShimmerReverbTime = this->m_Properties.GetNumber("FXShimmerReverbTime", 30);
	this->m_nFXShimmerReverbDiffusion = this->m_Properties.GetNumber("FXShimmerReverbDiffusion", 30);
	this->m_nFXShimmerReverbLP = this->m_Properties.GetNumber("FXShimmerReverbLP", 99);

	for(unsigned in = 0; in < CConfig::ToneGenerators; ++in)
	{
		for(unsigned fx = 0; fx < MixerOutput::kFXCount; ++fx)
		{
			std::ostringstream oss("FXSendLevel_");
			oss << in << "_x_" << fx;
			unsigned defaultLevel = 0;
			if(fx == MixerOutput::MainOutput) defaultLevel = 50;
			else if(fx == MixerOutput::FX_PlateReverb) defaultLevel = 50;
			this->m_nFXSendLevel[in][fx] = this->m_Properties.GetNumber(oss.str().c_str(), defaultLevel);
		}
	}

	for(unsigned ret = 0; ret < (MixerOutput::kFXCount - 1); ++ret)
	{
		for(unsigned fx = 0; fx < MixerOutput::kFXCount; ++fx)
		{
			std::ostringstream oss("FXReturnLevel_");
			oss << ret << "_x_" << fx;
			unsigned defaultLevel = 0;
			if(ret == MixerOutput::FX_PlateReverb && fx == MixerOutput::MainOutput) defaultLevel = 50;
			this->m_nFXReturnLevel[ret][fx] = this->m_Properties.GetNumber(oss.str().c_str(), defaultLevel);
		}
	}
#endif

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

		PropertyName.Format ("Cutoff%u", nTG+1);
		m_Properties.SetNumber (PropertyName, m_nCutoff[nTG]);

		PropertyName.Format ("Resonance%u", nTG+1);
		m_Properties.SetNumber (PropertyName, m_nResonance[nTG]);

		PropertyName.Format ("NoteLimitLow%u", nTG+1);
		m_Properties.SetNumber (PropertyName, m_nNoteLimitLow[nTG]);

		PropertyName.Format ("NoteLimitHigh%u", nTG+1);
		m_Properties.SetNumber (PropertyName, m_nNoteLimitHigh[nTG]);

		PropertyName.Format ("NoteShift%u", nTG+1);
		m_Properties.SetSignedNumber (PropertyName, m_nNoteShift[nTG]);

		PropertyName.Format ("ReverbSend%u", nTG+1);
		m_Properties.SetNumber (PropertyName, m_nReverbSend[nTG]);
		
		PropertyName.Format ("PitchBendRange%u", nTG+1);
		m_Properties.SetNumber (PropertyName, m_nPitchBendRange[nTG]);

		PropertyName.Format ("PitchBendStep%u", nTG+1);
		m_Properties.SetNumber (PropertyName, m_nPitchBendStep[nTG]);

		PropertyName.Format ("PortamentoMode%u", nTG+1);
		m_Properties.SetNumber (PropertyName, m_nPortamentoMode[nTG]);

		PropertyName.Format ("PortamentoGlissando%u", nTG+1);
		m_Properties.SetNumber (PropertyName, m_nPortamentoGlissando[nTG]);

		PropertyName.Format ("PortamentoTime%u", nTG+1);
		m_Properties.SetNumber (PropertyName, m_nPortamentoTime[nTG]);
		
		PropertyName.Format ("VoiceData%u", nTG+1);
		char *cstr = &m_nVoiceDataTxt[nTG][0];
		m_Properties.SetString (PropertyName, cstr);
		
		PropertyName.Format ("MonoMode%u", nTG+1);
		m_Properties.SetNumber (PropertyName, m_bMonoMode[nTG] ? 1 : 0);
				
		PropertyName.Format ("ModulationWheelRange%u", nTG+1);
		m_Properties.SetNumber (PropertyName, m_nModulationWheelRange[nTG]);
	
		PropertyName.Format ("ModulationWheelTarget%u", nTG+1);
		m_Properties.SetNumber (PropertyName, m_nModulationWheelTarget[nTG]);	
			
		PropertyName.Format ("FootControlRange%u", nTG+1);
		m_Properties.SetNumber (PropertyName, m_nFootControlRange[nTG]);	
		
		PropertyName.Format ("FootControlTarget%u", nTG+1);
		m_Properties.SetNumber (PropertyName, m_nFootControlTarget[nTG]);	
		
		PropertyName.Format ("BreathControlRange%u", nTG+1);
		m_Properties.SetNumber (PropertyName, m_nBreathControlRange[nTG]);	
		
		PropertyName.Format ("BreathControlTarget%u", nTG+1);
		m_Properties.SetNumber (PropertyName, m_nBreathControlTarget[nTG]);	
		
		PropertyName.Format ("AftertouchRange%u", nTG+1);
		m_Properties.SetNumber (PropertyName, m_nAftertouchRange[nTG]);	
		
		PropertyName.Format ("AftertouchTarget%u", nTG+1);
		m_Properties.SetNumber (PropertyName, m_nAftertouchTarget[nTG]);			

		}

	m_Properties.SetNumber ("CompressorEnable", m_bCompressorEnable ? 1 : 0);

	m_Properties.SetNumber ("ReverbEnable", m_bReverbEnable ? 1 : 0);
	m_Properties.SetNumber ("ReverbSize", m_nReverbSize);
	m_Properties.SetNumber ("ReverbHighDamp", m_nReverbHighDamp);
	m_Properties.SetNumber ("ReverbLowDamp", m_nReverbLowDamp);
	m_Properties.SetNumber ("ReverbLowPass", m_nReverbLowPass);
	m_Properties.SetNumber ("ReverbDiffusion", m_nReverbDiffusion);
	m_Properties.SetNumber ("ReverbLevel", m_nReverbLevel);

#ifdef MIXING_CONSOLE_ENABLE
	this->m_Properties.SetNumber("FXTubeEnable", m_bFXTubeEnable ? 1 : 0);
	this->m_Properties.SetNumber("FXTubeOverdrive", m_nFXTubeOverdrive);
	this->m_Properties.SetNumber("FXChorusEnable", m_bFXChorusEnable ? 1 : 0);
	this->m_Properties.SetNumber("FXChorusRate", m_nFXChorusRate);
	this->m_Properties.SetNumber("FXChorusDepth", m_nFXChorusDepth);
	this->m_Properties.SetNumber("FXFlangerEnable", m_bFXFlangerEnable ? 1 : 0);
	this->m_Properties.SetNumber("FXFlangerRate", m_nFXFlangerRate);
	this->m_Properties.SetNumber("FXFlangerDepth", m_nFXFlangerDepth);
	this->m_Properties.SetNumber("FXFlangerFeedback", m_nFXFlangerFeedback);
	this->m_Properties.SetNumber("FXOrbitoneEnable", m_bFXOrbitoneEnable ? 1 : 0);
	this->m_Properties.SetNumber("FXOrbitoneRate", m_nFXOrbitoneRate);
	this->m_Properties.SetNumber("FXOrbitoneDepth", m_nFXOrbitoneDepth);
	this->m_Properties.SetNumber("FXPhaserEnable", m_bFXPhaserEnable ? 1 : 0);
	this->m_Properties.SetNumber("FXPhaserRate", m_nFXPhaserRate);
	this->m_Properties.SetNumber("FXPhaserDepth", m_nFXPhaserDepth);
	this->m_Properties.SetNumber("FXPhaserFeedback", m_nFXPhaserFeedback);
	this->m_Properties.SetNumber("FXPhaserNbStages", m_nFXPhaserNbStages);
	this->m_Properties.SetNumber("FXDelayEnable", m_bFXDelayEnable ? 1 : 0);
	this->m_Properties.SetNumber("FXDelayLeftDelayTime", m_nFXDelayLeftDelayTime);
	this->m_Properties.SetNumber("FXDelayRightDelayTime", m_nFXDelayRightDelayTime);
	this->m_Properties.SetNumber("FXDelayFeedback", m_nFXDelayFeedback);
	this->m_Properties.SetNumber("FXShimmerReverbEnable", m_bFXShimmerReverbEnable ? 1 : 0);
	this->m_Properties.SetNumber("FXShimmerReverbInputGain", m_nFXShimmerReverbInputGain);
	this->m_Properties.SetNumber("FXShimmerReverbTime", m_nFXShimmerReverbTime);
	this->m_Properties.SetNumber("FXShimmerReverbDiffusion", m_nFXShimmerReverbDiffusion);
	this->m_Properties.SetNumber("FXShimmerReverbLP", m_nFXShimmerReverbLP);

	for(unsigned in = 0; in < CConfig::ToneGenerators; ++in)
	{
		for(unsigned fx = 0; fx < MixerOutput::kFXCount; ++fx)
		{
			std::ostringstream oss("FXSendLevel_");
			oss << in << "_x_" << fx;
			this->m_Properties.SetNumber(oss.str().c_str(), this->m_nFXSendLevel[in][fx]);
		}
	}

	for(unsigned ret = 0; ret < (MixerOutput::kFXCount - 1); ++ret)
	{
		for(unsigned fx = 0; fx < MixerOutput::kFXCount; ++fx)
		{
			std::ostringstream oss("FXReturnLevel_");
			oss << ret << "_x_" << fx;
			this->m_Properties.SetNumber(oss.str().c_str(), this->m_nFXReturnLevel[ret][fx]);
		}
	}
#endif

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

unsigned CPerformanceConfig::GetCutoff (unsigned nTG) const
{
	assert (nTG < CConfig::ToneGenerators);
	return m_nCutoff[nTG];
}

unsigned CPerformanceConfig::GetResonance (unsigned nTG) const
{
	assert (nTG < CConfig::ToneGenerators);
	return m_nResonance[nTG];
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

unsigned CPerformanceConfig::GetReverbSend (unsigned nTG) const
{
	assert (nTG < CConfig::ToneGenerators);
	return m_nReverbSend[nTG];
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

void CPerformanceConfig::SetCutoff (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	m_nCutoff[nTG] = nValue;
}

void CPerformanceConfig::SetResonance (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	m_nResonance[nTG] = nValue;
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

void CPerformanceConfig::SetReverbSend (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	m_nReverbSend[nTG] = nValue;
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
// Pitch bender and portamento:
void CPerformanceConfig::SetPitchBendRange (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	m_nPitchBendRange[nTG] = nValue;
}

unsigned CPerformanceConfig::GetPitchBendRange (unsigned nTG) const
{
	assert (nTG < CConfig::ToneGenerators);
	return m_nPitchBendRange[nTG];
}


void CPerformanceConfig::SetPitchBendStep (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	m_nPitchBendStep[nTG] = nValue;
}

unsigned CPerformanceConfig::GetPitchBendStep (unsigned nTG) const
{
	assert (nTG < CConfig::ToneGenerators);
	return m_nPitchBendStep[nTG];
}


void CPerformanceConfig::SetPortamentoMode (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	m_nPortamentoMode[nTG] = nValue;
}

unsigned CPerformanceConfig::GetPortamentoMode (unsigned nTG) const
{
	assert (nTG < CConfig::ToneGenerators);
	return m_nPortamentoMode[nTG];
}


void CPerformanceConfig::SetPortamentoGlissando (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	m_nPortamentoGlissando[nTG] = nValue;
}

unsigned CPerformanceConfig::GetPortamentoGlissando (unsigned nTG) const
{
	assert (nTG < CConfig::ToneGenerators);
	return m_nPortamentoGlissando[nTG];
}


void CPerformanceConfig::SetPortamentoTime (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	m_nPortamentoTime[nTG] = nValue;
}

unsigned CPerformanceConfig::GetPortamentoTime (unsigned nTG) const
{
	assert (nTG < CConfig::ToneGenerators);
	return m_nPortamentoTime[nTG];
}

void CPerformanceConfig::SetMonoMode (bool bValue, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	m_bMonoMode[nTG] = bValue;
}

bool CPerformanceConfig::GetMonoMode (unsigned nTG) const
{
	return m_bMonoMode[nTG];
}

void CPerformanceConfig::SetModulationWheelRange (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	m_nModulationWheelRange[nTG] = nValue;
}

unsigned CPerformanceConfig::GetModulationWheelRange (unsigned nTG) const
{
	assert (nTG < CConfig::ToneGenerators);
	return m_nModulationWheelRange[nTG];
}

void CPerformanceConfig::SetModulationWheelTarget (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	m_nModulationWheelTarget[nTG] = nValue;
}

unsigned CPerformanceConfig::GetModulationWheelTarget (unsigned nTG) const
{
	assert (nTG < CConfig::ToneGenerators);
	return m_nModulationWheelTarget[nTG];
}

void CPerformanceConfig::SetFootControlRange (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	m_nFootControlRange[nTG] = nValue;
}

unsigned CPerformanceConfig::GetFootControlRange (unsigned nTG) const
{
	assert (nTG < CConfig::ToneGenerators);
	return m_nFootControlRange[nTG];
}

void CPerformanceConfig::SetFootControlTarget (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	m_nFootControlTarget[nTG] = nValue;
}

unsigned CPerformanceConfig::GetFootControlTarget (unsigned nTG) const
{
	assert (nTG < CConfig::ToneGenerators);
	return m_nFootControlTarget[nTG];
}

void CPerformanceConfig::SetBreathControlRange (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	m_nBreathControlRange[nTG] = nValue;
}

unsigned CPerformanceConfig::GetBreathControlRange (unsigned nTG) const
{
	assert (nTG < CConfig::ToneGenerators);
	return m_nBreathControlRange[nTG];
}

void CPerformanceConfig::SetBreathControlTarget (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	m_nBreathControlTarget[nTG] = nValue;
}

unsigned CPerformanceConfig::GetBreathControlTarget (unsigned nTG) const
{
	assert (nTG < CConfig::ToneGenerators);
	return m_nBreathControlTarget[nTG];
}

void CPerformanceConfig::SetAftertouchRange (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	m_nAftertouchRange[nTG] = nValue;
}

unsigned CPerformanceConfig::GetAftertouchRange (unsigned nTG) const
{
	assert (nTG < CConfig::ToneGenerators);
	return m_nAftertouchRange[nTG];
}

void CPerformanceConfig::SetAftertouchTarget (unsigned nValue, unsigned nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	m_nAftertouchTarget[nTG] = nValue;
}

unsigned CPerformanceConfig::GetAftertouchTarget (unsigned nTG) const
{
	assert (nTG < CConfig::ToneGenerators);
	return m_nAftertouchTarget[nTG];
}

void CPerformanceConfig::SetVoiceDataToTxt (const uint8_t *pData, unsigned nTG)  
{
	assert (nTG < CConfig::ToneGenerators);
	m_nVoiceDataTxt[nTG] = "";
	char nDtoH[]="0123456789ABCDEF";
	for (int i = 0; i < NUM_VOICE_PARAM; i++)
	{
	m_nVoiceDataTxt[nTG]  += nDtoH[(pData[i] & 0xF0)/16];
	m_nVoiceDataTxt[nTG]  += nDtoH[pData[i] & 0x0F]  ;
	if ( i < (NUM_VOICE_PARAM-1)  ) 
    	{
     	 m_nVoiceDataTxt[nTG] += " ";
    	}
	}
}

uint8_t *CPerformanceConfig::GetVoiceDataFromTxt (unsigned nTG) 
{
	assert (nTG < CConfig::ToneGenerators);
	static uint8_t pData[NUM_VOICE_PARAM];
	std::string nHtoD="0123456789ABCDEF";
	 
 	for (int i=0; i<NUM_VOICE_PARAM * 3; i=i+3)
	{
  	pData[i/3] = ((nHtoD.find(toupper(m_nVoiceDataTxt[nTG][i]),0) * 16 + nHtoD.find(toupper(m_nVoiceDataTxt[nTG][i+1]),0))) & 0xFF ;
	} 
  
	return pData;
}

bool CPerformanceConfig::VoiceDataFilled(unsigned nTG) 
{
	return (strcmp(m_nVoiceDataTxt[nTG].c_str(),"") != 0) ;
}

std::string CPerformanceConfig::GetPerformanceFileName(unsigned nID)
{
	return m_nPerformanceFileName[nID];
}

std::string CPerformanceConfig::GetPerformanceName(unsigned nID)
{
	if(nID == 0) // in order to assure retrocompatibility
	{
		return "Default";
	}
	else
	{
		return m_nPerformanceFileName[nID].substr(0,m_nPerformanceFileName[nID].length()-4).substr(7,14);
	}
}

unsigned CPerformanceConfig::GetLastPerformance()
{
	return nLastPerformance;
}

unsigned CPerformanceConfig::GetActualPerformanceID()
{
	return nActualPerformance;
}

void CPerformanceConfig::SetActualPerformanceID(unsigned nID)
{
	nActualPerformance = nID;
}

bool CPerformanceConfig::GetInternalFolderOk()
{
	return nInternalFolderOk;
}

bool CPerformanceConfig::CreateNewPerformanceFile(void)
{
	std::string sPerformanceName = NewPerformanceName;
	NewPerformanceName=""; 
	nActualPerformance=nLastPerformance;
	std::string nFileName;
	std::string nPath;
	std::string nIndex = "000000";
	nIndex += std::to_string(++nLastFileIndex);
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
	m_nPerformanceFileName[nLastPerformance]= nFileName;
	
	nPath = "SD:/" ;
	nPath += PERFORMANCE_DIR;
	nPath += "/";
	nFileName = nPath + nFileName;
	
	FIL File;
	FRESULT Result = f_open (&File, nFileName.c_str(), FA_WRITE | FA_CREATE_ALWAYS);
	if (Result != FR_OK)
	{
		m_nPerformanceFileName[nLastPerformance]=nullptr;
		return false;
	}

	if (f_close (&File) != FR_OK)
	{
		m_nPerformanceFileName[nLastPerformance]=nullptr;
		return false;
	}
	
	nLastPerformance++;
	new (&m_Properties) CPropertiesFatFsFile(nFileName.c_str(), m_pFileSystem);
	
	return true;
}

bool CPerformanceConfig::ListPerformances()
{
	nInternalFolderOk=false;
	nExternalFolderOk=false; // for future USB implementation
	nLastPerformance=0;
	nLastFileIndex=0;
   	m_nPerformanceFileName[nLastPerformance++]="performance.ini"; // in order to assure retrocompatibility
	
	unsigned nPIndex;
    DIR Directory;
	FILINFO FileInfo;
	FRESULT Result;
	//Check if internal "performance" directory exists
	Result = f_opendir (&Directory, "SD:/" PERFORMANCE_DIR);
	if (Result == FR_OK)
	{
		nInternalFolderOk=true;		
//		Result = f_closedir (&Directory);
	}
	else
	{
		// attenpt to create the folder
		Result = f_mkdir("SD:/" PERFORMANCE_DIR);
		nInternalFolderOk = (Result == FR_OK);
	}
	
	if (nInternalFolderOk)
	{
	Result = f_findfirst (&Directory, &FileInfo, "SD:/" PERFORMANCE_DIR, "*.ini");
		for (unsigned i = 0; Result == FR_OK && FileInfo.fname[0]; i++)
		{
			if (!(FileInfo.fattrib & (AM_HID | AM_SYS)))  
			{
				std::string FileName = FileInfo.fname;
				size_t nLen = FileName.length();
				if (   nLen > 8 && nLen <26	 && strcmp(FileName.substr(6,1).c_str(), "_")==0)
				{			
					nPIndex=stoi(FileName.substr(0,6));
					if(nPIndex > nLastFileIndex)
					{
						nLastFileIndex=nPIndex;
					}
		
					m_nPerformanceFileName[nLastPerformance++]= FileName;
				}
			}

			Result = f_findnext (&Directory, &FileInfo);
		}
		// sort by performance number-name
		if (nLastPerformance > 2)
		{
		sort (m_nPerformanceFileName+1, m_nPerformanceFileName + nLastPerformance); // default is always on first place. %%%%%%%%%%%%%%%%
		}
	}
	
	return nInternalFolderOk;
}   
    

void CPerformanceConfig::SetNewPerformance (unsigned nID)
{
		nActualPerformance=nID;
		std::string FileN = "";
		if (nID != 0) // in order to assure retrocompatibility
		{
			FileN += PERFORMANCE_DIR;
			FileN += "/";
		}
		FileN += m_nPerformanceFileName[nID];
		new (&m_Properties) CPropertiesFatFsFile(FileN.c_str(), m_pFileSystem);
		
}

std::string CPerformanceConfig::GetNewPerformanceDefaultName(void)
{
	std::string nIndex = "000000";
	nIndex += std::to_string(nLastFileIndex+1);
	nIndex = nIndex.substr(nIndex.length()-6,6);
	return "Perf" + nIndex;
}

void CPerformanceConfig::SetNewPerformanceName(std::string nName)
{
	int  i = nName.length();
	do
	{
		--i;
	}
	while (i>=0 && nName[i] == 32);
	nName=nName.substr(0,i+1)  ;
	
	NewPerformanceName = nName;
}

bool CPerformanceConfig::DeletePerformance(unsigned nID)
{
	bool bOK = false;
	if(nID == 0){return bOK;} // default (performance.ini at root directory) can't be deleted
	DIR Directory;
	FILINFO FileInfo;
	std::string FileN = "SD:/";
	FileN += PERFORMANCE_DIR;

	
	FRESULT Result = f_findfirst (&Directory, &FileInfo, FileN.c_str(), m_nPerformanceFileName[nID].c_str());
	if (Result == FR_OK && FileInfo.fname[0])
	{
		FileN += "/";
		FileN += m_nPerformanceFileName[nID];
		Result=f_unlink (FileN.c_str());
		if (Result == FR_OK)
		{
			SetNewPerformance(0);
			nActualPerformance =0;
			//nMenuSelectedPerformance=0;
			m_nPerformanceFileName[nID]="ZZZZZZ";
			sort (m_nPerformanceFileName+1, m_nPerformanceFileName + nLastPerformance); // test si va con -1 o no
			--nLastPerformance;
			m_nPerformanceFileName[nLastPerformance]=nullptr;
			bOK=true;
		}
	}
	return bOK;
}

#ifdef MIXING_CONSOLE_ENABLE

bool CPerformanceConfig::GetFXTubeEnable(void) const
{
	return this->m_bFXTubeEnable;
}

unsigned CPerformanceConfig::GetFXTubeOverdrive(void) const
{
	return this->m_nFXTubeOverdrive;
}

bool CPerformanceConfig::GetFXChorusEnable(void) const
{
	return this->m_bFXChorusEnable;
}

unsigned CPerformanceConfig::GetFXChorusRate(void) const
{
	return this->m_nFXChorusRate;
}

unsigned CPerformanceConfig::GetFXChorusDepth(void) const
{
	return this->m_nFXChorusDepth;
}

bool CPerformanceConfig::GetFXFlangerEnable(void) const
{
	return this->m_bFXFlangerEnable;
}

unsigned CPerformanceConfig::GetFXFlangerRate(void) const
{
	return this->m_nFXFlangerRate;
}

unsigned CPerformanceConfig::GetFXFlangerDepth(void) const
{
	return this->m_nFXFlangerDepth;
}

unsigned CPerformanceConfig::GetFXFlangerFeedback(void) const
{
	return this->m_nFXFlangerFeedback;
}

bool CPerformanceConfig::GetFXOrbitoneEnable(void) const
{
	return this->m_bFXOrbitoneEnable;
}

unsigned CPerformanceConfig::GetFXOrbitoneRate(void) const
{
	return this->m_nFXOrbitoneRate;
}

unsigned CPerformanceConfig::GetFXOrbitoneDepth(void) const
{
	return this->m_nFXOrbitoneDepth;
}

bool CPerformanceConfig::GetFXPhaserEnable(void) const
{
	return this->m_bFXPhaserEnable;
}

unsigned CPerformanceConfig::GetFXPhaserRate(void) const
{
	return this->m_nFXPhaserRate;
}

unsigned CPerformanceConfig::GetFXPhaserDepth(void) const
{
	return this->m_nFXPhaserDepth;
}

unsigned CPerformanceConfig::GetFXPhaserFeedback(void) const
{
	return this->m_nFXPhaserFeedback;
}

unsigned CPerformanceConfig::GetFXPhaserNbStages(void) const
{
	return this->m_nFXPhaserNbStages;
}

bool CPerformanceConfig::GetFXDelayEnable(void) const
{
	return this->m_bFXDelayEnable;
}

unsigned CPerformanceConfig::GetFXDelayLeftDelayTime(void) const
{
	return this->m_nFXDelayLeftDelayTime;
}

unsigned CPerformanceConfig::GetFXDelayRightDelayTime(void) const
{
	return this->m_nFXDelayRightDelayTime;
}

unsigned CPerformanceConfig::GetFXDelayFeedback(void) const
{
	return this->m_nFXDelayFeedback;
}

bool CPerformanceConfig::GetFXShimmerReverbEnable(void) const
{
	return this->m_bFXShimmerReverbEnable;
}

unsigned CPerformanceConfig::GetFXShimmerReverbInputGain(void) const
{
	return this->m_nFXShimmerReverbInputGain;
}

unsigned CPerformanceConfig::GetFXShimmerReverbTime(void) const
{
	return this->m_nFXShimmerReverbTime;
}

unsigned CPerformanceConfig::GetFXShimmerReverbDiffusion(void) const
{
	return this->m_nFXShimmerReverbDiffusion;
}

unsigned CPerformanceConfig::GetFXShimmerReverbLP(void) const
{
	return this->m_nFXShimmerReverbLP;
}

unsigned CPerformanceConfig::GetFXSendLevel(unsigned in, MixerOutput fx) const
{
	assert(in < CConfig::ToneGenerators);
	assert(fx < MixerOutput::kFXCount);
	return this->m_nFXSendLevel[in][fx];
}

unsigned CPerformanceConfig::GetFXReturnLevel(MixerOutput ret, MixerOutput fx) const
{
	assert(ret < (MixerOutput::kFXCount - 1));
	assert(fx < MixerOutput::kFXCount);
	return (ret == fx) ? 0 : this->m_nFXReturnLevel[ret][fx];
}

void CPerformanceConfig::SetFXTubeEnable(bool bValue)
{
	this->m_bFXTubeEnable = bValue;
}

void CPerformanceConfig::SetFXTubeOverdrive(unsigned nValue)
{
	this->m_nFXTubeOverdrive = nValue;
}

void CPerformanceConfig::SetFXChorusEnable(bool bValue)
{
	this->m_bFXChorusEnable = bValue;
}

void CPerformanceConfig::SetFXChorusRate(unsigned nValue)
{
	this->m_nFXChorusRate = nValue;
}

void CPerformanceConfig::SetFXChorusDepth(unsigned nValue)
{
	this->m_nFXChorusDepth = nValue;
}

void CPerformanceConfig::SetFXFlangerEnable(bool bValue)
{
	this->m_bFXFlangerEnable = bValue;
}

void CPerformanceConfig::SetFXFlangerRate(unsigned nValue)
{
	this->m_nFXFlangerRate = nValue;
}

void CPerformanceConfig::SetFXFlangerDepth(unsigned nValue)
{
	this->m_nFXFlangerDepth = nValue;
}

void CPerformanceConfig::SetFXFlangerFeedback(unsigned nValue)
{
	this->m_nFXFlangerFeedback = nValue;
}

void CPerformanceConfig::SetFXOrbitoneEnable(bool bValue)
{
	this->m_bFXOrbitoneEnable = bValue;
}

void CPerformanceConfig::SetFXOrbitoneRate(unsigned nValue)
{
	this->m_nFXOrbitoneRate = nValue;
}

void CPerformanceConfig::SetFXOrbitoneDepth(unsigned nValue)
{
	this->m_nFXOrbitoneDepth = nValue;
}

void CPerformanceConfig::SetFXPhaserEnable(bool bValue)
{
	this->m_bFXPhaserEnable = bValue;
}

void CPerformanceConfig::SetFXPhaserRate(unsigned nValue)
{
	this->m_nFXPhaserRate = nValue;
}

void CPerformanceConfig::SetFXPhaserDepth(unsigned nValue)
{
	this->m_nFXPhaserDepth = nValue;
}

void CPerformanceConfig::SetFXPhaserFeedback(unsigned nValue)
{
	this->m_nFXPhaserFeedback = nValue;
}

void CPerformanceConfig::SetFXPhaserNbStages(unsigned nValue)
{
	this->m_nFXPhaserNbStages = nValue;
}

void CPerformanceConfig::SetFXDelayEnable(unsigned bValue)
{
	this->m_bFXDelayEnable = bValue;
}

void CPerformanceConfig::SetFXDelayLeftDelayTime(unsigned nValue)
{
	this->m_nFXDelayLeftDelayTime = nValue;
}

void CPerformanceConfig::SetFXDelayRightDelayTime(unsigned nValue)
{
	this->m_nFXDelayRightDelayTime = nValue;
}

void CPerformanceConfig::SetFXDelayFeedback(unsigned nValue)
{
	this->m_nFXDelayFeedback = nValue;
}

void CPerformanceConfig::SetFXShimmerReverbEnable(unsigned bValue)
{
	this->m_bFXShimmerReverbEnable = bValue;
}

void CPerformanceConfig::SetFXShimmerReverbInputGain(unsigned nValue)
{
	this->m_nFXShimmerReverbInputGain = nValue;
}

void CPerformanceConfig::SetFXShimmerReverbTime(unsigned nValue)
{
	this->m_nFXShimmerReverbTime = nValue;
}

void CPerformanceConfig::SetFXShimmerReverbDiffusion(unsigned nValue)
{
	this->m_nFXShimmerReverbDiffusion = nValue;
}

void CPerformanceConfig::SetFXShimmerReverbLP(unsigned nValue)
{
	this->m_nFXShimmerReverbLP = nValue;
}

void CPerformanceConfig::SetFXSendLevel(unsigned in, MixerOutput fx, unsigned nValue)
{
	assert(in < CConfig::ToneGenerators);
	assert(fx < MixerOutput::kFXCount);
	this->m_nFXSendLevel[in][fx] = nValue;
}

void CPerformanceConfig::SetFXReturnLevel(MixerOutput ret, MixerOutput fx, unsigned nValue)
{
	assert(ret < (MixerOutput::kFXCount - 1));
	assert(fx < MixerOutput::kFXCount);
	this->m_nFXReturnLevel[ret][fx] = (ret == fx) ? 0 : nValue;
}

#endif
