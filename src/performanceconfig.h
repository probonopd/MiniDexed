//
// performanceconfig.h
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
#ifndef _performanceconfig_h
#define _performanceconfig_h

#include "config.h"
#include "mixing_console_constants.h"
#include <fatfs/ff.h>
#include <Properties/propertiesfatfsfile.h>
#define NUM_VOICE_PARAM 156
#define PERFORMANCE_DIR "performance" 
#define NUM_PERFORMANCES 256

class CPerformanceConfig	// Performance configuration
{
public:
	CPerformanceConfig (FATFS *pFileSystem);
	~CPerformanceConfig (void);

	bool Load (void);

	bool Save (void);

	// TG#
	unsigned GetBankNumber (unsigned nTG) const;		// 0 .. 127
	unsigned GetVoiceNumber (unsigned nTG) const;		// 0 .. 31
	unsigned GetMIDIChannel (unsigned nTG) const;		// 0 .. 15, omni, off
	unsigned GetVolume (unsigned nTG) const;		// 0 .. 127
	unsigned GetPan (unsigned nTG) const;			// 0 .. 127
	int GetDetune (unsigned nTG) const;			// -99 .. 99
	unsigned GetCutoff (unsigned nTG) const;		// 0 .. 99
	unsigned GetResonance (unsigned nTG) const;		// 0 .. 99
	unsigned GetNoteLimitLow (unsigned nTG) const;		// 0 .. 127
	unsigned GetNoteLimitHigh (unsigned nTG) const;		// 0 .. 127
	int GetNoteShift (unsigned nTG) const;			// -24 .. 24
#if defined(PLATE_REVERB_ENABLE)
	unsigned GetReverbSend (unsigned nTG) const;		// 0 .. 127
#endif
	unsigned GetPitchBendRange (unsigned nTG) const;		// 0 .. 12
	unsigned GetPitchBendStep (unsigned nTG) const;		// 0 .. 12
	unsigned GetPortamentoMode (unsigned nTG) const;		// 0 .. 1
	unsigned GetPortamentoGlissando (unsigned nTG) const;		// 0 .. 1
	unsigned GetPortamentoTime (unsigned nTG) const;		// 0 .. 99
	bool GetMonoMode (unsigned nTG) const; 				// 0 .. 1
	
	unsigned GetModulationWheelRange (unsigned nTG) const; // 0 .. 99
	unsigned GetModulationWheelTarget (unsigned nTG) const; // 0 .. 7
	unsigned GetFootControlRange (unsigned nTG) const; // 0 .. 99
	unsigned GetFootControlTarget (unsigned nTG) const;  // 0 .. 7
	unsigned GetBreathControlRange (unsigned nTG) const; // 0 .. 99
	unsigned GetBreathControlTarget (unsigned nTG) const;  // 0 .. 7
	unsigned GetAftertouchRange (unsigned nTG) const; // 0 .. 99
	unsigned GetAftertouchTarget (unsigned nTG) const;  // 0 .. 7

	void SetBankNumber (unsigned nValue, unsigned nTG);
	void SetVoiceNumber (unsigned nValue, unsigned nTG);
	void SetMIDIChannel (unsigned nValue, unsigned nTG);
	void SetVolume (unsigned nValue, unsigned nTG);
	void SetPan (unsigned nValue, unsigned nTG);
	void SetDetune (int nValue, unsigned nTG);
	void SetCutoff (unsigned nValue, unsigned nTG);
	void SetResonance (unsigned nValue, unsigned nTG);
	void SetNoteLimitLow (unsigned nValue, unsigned nTG);
	void SetNoteLimitHigh (unsigned nValue, unsigned nTG);
	void SetNoteShift (int nValue, unsigned nTG);
#if defined(PLATE_REVERB_ENABLE)
	void SetReverbSend (unsigned nValue, unsigned nTG);
#endif
	void SetPitchBendRange (unsigned nValue, unsigned nTG);
	void SetPitchBendStep (unsigned nValue, unsigned nTG);
	void SetPortamentoMode (unsigned nValue, unsigned nTG);
	void SetPortamentoGlissando (unsigned nValue, unsigned nTG);
	void SetPortamentoTime (unsigned nValue, unsigned nTG);
	void SetVoiceDataToTxt (const uint8_t *pData, unsigned nTG); 
	uint8_t *GetVoiceDataFromTxt (unsigned nTG);
	void SetMonoMode (bool bOKValue, unsigned nTG); 

	void SetModulationWheelRange (unsigned nValue, unsigned nTG);
	void SetModulationWheelTarget (unsigned nValue, unsigned nTG);
	void SetFootControlRange (unsigned nValue, unsigned nTG);
	void SetFootControlTarget (unsigned nValue, unsigned nTG);
	void SetBreathControlRange (unsigned nValue, unsigned nTG);
	void SetBreathControlTarget (unsigned nValue, unsigned nTG);
	void SetAftertouchRange (unsigned nValue, unsigned nTG);
	void SetAftertouchTarget (unsigned nValue, unsigned nTG);

	// Effects
	bool GetCompressorEnable (void) const;
	bool GetReverbEnable (void) const;
	unsigned GetReverbSize (void) const;			// 0 .. 99
	unsigned GetReverbHighDamp (void) const;		// 0 .. 99
	unsigned GetReverbLowDamp (void) const;			// 0 .. 99
	unsigned GetReverbLowPass (void) const;			// 0 .. 99
	unsigned GetReverbDiffusion (void) const;		// 0 .. 99
	unsigned GetReverbLevel (void) const;			// 0 .. 99

	void SetCompressorEnable (bool bValue);
	void SetReverbEnable (bool bValue);
	void SetReverbSize (unsigned nValue);
	void SetReverbHighDamp (unsigned nValue);
	void SetReverbLowDamp (unsigned nValue);
	void SetReverbLowPass (unsigned nValue);
	void SetReverbDiffusion (unsigned nValue);
	void SetReverbLevel (unsigned nValue);

#ifdef MIXING_CONSOLE_ENABLE
	bool GetFXTubeEnable(void) const;
	unsigned GetFXTubeOverdrive(void) const;

	bool GetFXChorusEnable(void) const;
	unsigned GetFXChorusRate(void) const;
	unsigned GetFXChorusDepth(void) const;

	bool GetFXFlangerEnable(void) const;
	unsigned GetFXFlangerRate(void) const;
	unsigned GetFXFlangerDepth(void) const;
	unsigned GetFXFlangerFeedback(void) const;

	bool GetFXOrbitoneEnable(void) const;
	unsigned GetFXOrbitoneRate(void) const;
	unsigned GetFXOrbitoneDepth(void) const;

	bool GetFXPhaserEnable(void) const;
	unsigned GetFXPhaserRate(void) const;
	unsigned GetFXPhaserDepth(void) const;
	unsigned GetFXPhaserFeedback(void) const;
	unsigned GetFXPhaserNbStages(void) const;

	bool GetFXDelayEnable(void) const;
	unsigned GetFXDelayLeftDelayTime(void) const;
	unsigned GetFXDelayRightDelayTime(void) const;
	unsigned GetFXDelayFeedback(void) const;

	bool GetFXReverberatorEnable(void) const;
	unsigned GetFXReverberatorInputGain(void) const;
	unsigned GetFXReverberatorTime(void) const;
	unsigned GetFXReverberatorDiffusion(void) const;
	unsigned GetFXReverberatorLP(void) const;
	unsigned GetTGSendLevel(unsigned in, MixerOutput fx) const;
	unsigned GetFXSendLevel(MixerOutput ret, MixerOutput fx) const;

	void SetFXTubeEnable(bool bValue);
	void SetFXTubeOverdrive(unsigned nValue);

	void SetFXChorusEnable(bool bValue);
	void SetFXChorusRate(unsigned nValue);
	void SetFXChorusDepth(unsigned nValue);

	void SetFXFlangerEnable(bool bValue);
	void SetFXFlangerRate(unsigned nValue);
	void SetFXFlangerDepth(unsigned nValue);
	void SetFXFlangerFeedback(unsigned nValue);

	void SetFXOrbitoneEnable(bool bValue);
	void SetFXOrbitoneRate(unsigned nValue);
	void SetFXOrbitoneDepth(unsigned nValue);

	void SetFXPhaserEnable(bool bValue);
	void SetFXPhaserRate(unsigned nValue);
	void SetFXPhaserDepth(unsigned nValue);
	void SetFXPhaserFeedback(unsigned nValue);
	void SetFXPhaserNbStages(unsigned nValue);

	void SetFXDelayEnable(unsigned nValue);
	void SetFXDelayLeftDelayTime(unsigned nValue);
	void SetFXDelayRightDelayTime(unsigned nValue);
	void SetFXDelayFeedback(unsigned nValue);

	void SetFXReverberatorEnable(unsigned nValue);
	void SetFXReverberatorInputGain(unsigned nValue);
	void SetFXReverberatorTime(unsigned nValue);
	void SetFXReverberatorDiffusion(unsigned nValue);
	void SetFXReverberatorLP(unsigned nValue);

	void SetTGSendLevel(unsigned in, MixerOutput fx, unsigned nValue);
	void SetFXSendLevel(MixerOutput fromFX, MixerOutput toFX, unsigned nValue);

	void SetFXBypass(bool bypass);
	bool IsFXBypass() const;
#endif

	bool VoiceDataFilled(unsigned nTG);
	bool ListPerformances(); 
	//std::string m_DirName;
	void SetNewPerformance (unsigned nID);
	std::string GetPerformanceFileName(unsigned nID);
	std::string GetPerformanceName(unsigned nID);
	unsigned GetLastPerformance();
	void SetActualPerformanceID(unsigned nID);
	unsigned GetActualPerformanceID();
	bool CreateNewPerformanceFile(void);
	bool GetInternalFolderOk(); 
	std::string GetNewPerformanceDefaultName(void);
	void SetNewPerformanceName(std::string nName);
	bool DeletePerformance(unsigned nID);
	bool CheckFreePerformanceSlot(void);

private:
	CPropertiesFatFsFile m_Properties;

	unsigned m_nBankNumber[CConfig::ToneGenerators];
	unsigned m_nVoiceNumber[CConfig::ToneGenerators];
	unsigned m_nMIDIChannel[CConfig::ToneGenerators];
	unsigned m_nVolume[CConfig::ToneGenerators];
	unsigned m_nPan[CConfig::ToneGenerators];
	int m_nDetune[CConfig::ToneGenerators];
	unsigned m_nCutoff[CConfig::ToneGenerators];
	unsigned m_nResonance[CConfig::ToneGenerators];
	unsigned m_nNoteLimitLow[CConfig::ToneGenerators];
	unsigned m_nNoteLimitHigh[CConfig::ToneGenerators];
	int m_nNoteShift[CConfig::ToneGenerators];
#if defined(PLATE_REVERB_ENABLE)
	int m_nReverbSend[CConfig::ToneGenerators];
#endif
	unsigned m_nPitchBendRange[CConfig::ToneGenerators];
	unsigned m_nPitchBendStep[CConfig::ToneGenerators];
	unsigned m_nPortamentoMode[CConfig::ToneGenerators];
	unsigned m_nPortamentoGlissando[CConfig::ToneGenerators];
	unsigned m_nPortamentoTime[CConfig::ToneGenerators];
	std::string m_nVoiceDataTxt[CConfig::ToneGenerators]; 
	bool m_bMonoMode[CConfig::ToneGenerators]; 

	unsigned m_nModulationWheelRange[CConfig::ToneGenerators];
	unsigned m_nModulationWheelTarget[CConfig::ToneGenerators];
	unsigned m_nFootControlRange[CConfig::ToneGenerators];	
	unsigned m_nFootControlTarget[CConfig::ToneGenerators];	
	unsigned m_nBreathControlRange[CConfig::ToneGenerators];	
	unsigned m_nBreathControlTarget[CConfig::ToneGenerators];	
	unsigned m_nAftertouchRange[CConfig::ToneGenerators];	
	unsigned m_nAftertouchTarget[CConfig::ToneGenerators];	

	unsigned nLastPerformance;  
	unsigned nLastFileIndex;
	unsigned nActualPerformance = 0;  
	//unsigned nMenuSelectedPerformance = 0; 
	std::string m_nPerformanceFileName[NUM_PERFORMANCES];
	FATFS *m_pFileSystem; 

	bool nInternalFolderOk=false;
	bool nExternalFolderOk=false; // for future USB implementation
	std::string NewPerformanceName="";
	
	bool m_bCompressorEnable;
	bool m_bReverbEnable;
	unsigned m_nReverbSize;
	unsigned m_nReverbHighDamp;
	unsigned m_nReverbLowDamp;
	unsigned m_nReverbLowPass;
	unsigned m_nReverbDiffusion;
	unsigned m_nReverbLevel;

#if defined(MIXING_CONSOLE_ENABLE)
	bool m_bFXTubeEnable;
	unsigned m_nFXTubeWet;
	unsigned m_nFXTubeOverdrive;

	bool m_bFXChorusEnable;
	unsigned m_nFXChorusRate;
	unsigned m_nFXChorusDepth;

	bool m_bFXFlangerEnable;
	unsigned m_nFXFlangerRate;
	unsigned m_nFXFlangerDepth;
	unsigned m_nFXFlangerFeedback;

	bool m_bFXOrbitoneEnable;
	unsigned m_nFXOrbitoneRate;
	unsigned m_nFXOrbitoneDepth;

	bool m_bFXPhaserEnable;
	unsigned m_nFXPhaserRate;
	unsigned m_nFXPhaserDepth;
	unsigned m_nFXPhaserFeedback;
	unsigned m_nFXPhaserNbStages;

	bool m_bFXDelayEnable;
	unsigned m_nFXDelayLeftDelayTime;
	unsigned m_nFXDelayRightDelayTime;
	unsigned m_nFXDelayFeedback;

	bool m_bFXReverberatorEnable;
	unsigned m_nFXReverberatorInputGain;
	unsigned m_nFXReverberatorTime;
	unsigned m_nFXReverberatorDiffusion;
	unsigned m_nFXReverberatorLP;

	unsigned m_nTGSendLevel[CConfig::ToneGenerators + MixerOutput::kFXCount - 1][MixerOutput::kFXCount];
	unsigned m_nFXSendLevel[MixerOutput::kFXCount - 1][MixerOutput::kFXCount];

	bool m_bFXBypass;

#endif
};

#endif
