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
#include <fatfs/ff.h>
#include <Properties/propertiesfatfsfile.h>
#define NUM_VOICE_PARAM 156
#define PERFORMANCE_DIR "performance" 

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
	unsigned GetReverbSend (unsigned nTG) const;		// 0 .. 127
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
	void SetReverbSend (unsigned nValue, unsigned nTG);
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

#ifdef ARM_ALLOW_MULTI_CORE
	bool GetFXChainEnable(void) const;
	unsigned GetFXChainWet(void) const;
	bool GetFXChainTubeEnable(void) const;
	unsigned GetFXChainTubeWet(void) const;
	unsigned GetFXChainTubeOverdrive(void) const;
	bool GetFXChainChorusEnable(void) const;
	unsigned GetFXChainChorusWet(void) const;
	unsigned GetFXChainChorusRate(void) const;
	unsigned GetFXChainChorusDepth(void) const;
	unsigned GetFXChainChorusFeedback(void) const;
	bool GetFXChainFlangerEnable(void) const;
	unsigned GetFXChainFlangerWet(void) const;
	unsigned GetFXChainFlangerDelayTime(void) const;
	unsigned GetFXChainFlangerRate(void) const;
	unsigned GetFXChainFlangerDepth(void) const;
	unsigned GetFXChainFlangerFeedback(void) const;
	bool GetFXChainOrbitoneEnable(void) const;
	unsigned GetFXChainOrbitoneWet(void) const;
	unsigned GetFXChainOrbitoneFeedback(void) const;
	bool GetFXChainPhaserEnable(void) const;
	unsigned GetFXChainPhaserWet(void) const;
	unsigned GetFXChainPhaserRate(void) const;
	unsigned GetFXChainPhaserResonance(void) const;
	bool GetFXChainTapeDelayEnable(void) const;
	unsigned GetFXChainTapeDelayWet(void) const;
	unsigned GetFXChainTapeDelayLeftDelayTime(void) const;
	unsigned GetFXChainTapeDelayRightDelayTime(void) const;
	unsigned GetFXChainTapeDelayFlutter(void) const;
	unsigned GetFXChainTapeDelayFeedback(void) const;
	bool GetFXChainShimmerReverbEnable(void) const;
	unsigned GetFXChainShimmerReverbWet(void) const;
	unsigned GetFXChainShimmerReverbDecay(void) const;
	unsigned GetFXChainShimmerReverbDiffusion(void) const;
	unsigned GetFXChainShimmerReverbPitchShift(void) const;

	void SetFXChainEnable(bool bValue);
	void SetFXChainWet(unsigned nValue);
	void SetFXChainTubeEnable(bool bValue);
	void SetFXChainTubeWet(unsigned nValue);
	void SetFXChainTubeOverdrive(unsigned nValue);
	void SetFXChainChorusEnable(bool bValue);
	void SetFXChainChorusWet(unsigned nValue);
	void SetFXChainChorusRate(unsigned nValue);
	void SetFXChainChorusDepth(unsigned nValue);
	void SetFXChainChorusFeedback(unsigned nValue);
	void SetFXChainFlangerEnable(bool bValue);
	void SetFXChainFlangerWet(unsigned nValue);
	void SetFXChainFlangerDelayTime(unsigned nValue);
	void SetFXChainFlangerRate(unsigned nValue);
	void SetFXChainFlangerDepth(unsigned nValue);
	void SetFXChainFlangerFeedback(unsigned nValue);
	void SetFXChainOrbitoneEnable(bool bValue);
	void SetFXChainOrbitoneWet(unsigned nValue);
	void SetFXChainOrbitoneFeedback(unsigned nValue);
	void SetFXChainPhaserEnable(bool bValue);
	void SetFXChainPhaserWet(unsigned nValue);
	void SetFXChainPhaserRate(unsigned nValue);
	void SetFXChainPhaserResonance(unsigned nValue);
	void SetFXChainTapeDelayEnable(unsigned nValue);
	void SetFXChainTapeDelayWet(unsigned nValue);
	void SetFXChainTapeDelayLeftDelayTime(unsigned nValue);
	void SetFXChainTapeDelayRightDelayTime(unsigned nValue);
	void SetFXChainTapeDelayFlutter(unsigned nValue);
	void SetFXChainTapeDelayFeedback(unsigned nValue);
	void SetFXChainShimmerReverbEnable(unsigned nValue);
	void SetFXChainShimmerReverbWet(unsigned nValue);
	void SetFXChainShimmerReverbDecay(unsigned nValue);
	void SetFXChainShimmerReverbDiffusion(unsigned nValue);
	void SetFXChainShimmerReverbPitchShift(unsigned nValue);
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
	int m_nReverbSend[CConfig::ToneGenerators];
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
	std::string m_nPerformanceFileName[1024];
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

#ifdef ARM_ALLOW_MULTI_CORE
	bool m_bFXChainEnable;
	unsigned m_nFXChainWet;
	bool m_bFXChainTubeEnable;
	unsigned m_nFXChainTubeWet;
	unsigned m_nFXChainTubeOverdrive;
	bool m_bFXChainChorusEnable;
	unsigned m_nFXChainChorusWet;
	unsigned m_nFXChainChorusRate;
	unsigned m_nFXChainChorusDepth;
	unsigned m_nFXChainChorusFeedback;
	bool m_bFXChainFlangerEnable;
	unsigned m_nFXChainFlangerWet;
	unsigned m_nFXChainFlangerDelayTime;
	unsigned m_nFXChainFlangerRate;
	unsigned m_nFXChainFlangerDepth;
	unsigned m_nFXChainFlangerFeedback;
	bool m_bFXChainOrbitoneEnable;
	unsigned m_nFXChainOrbitoneWet;
	unsigned m_nFXChainOrbitoneFeedback;
	bool m_bFXChainPhaserEnable;
	unsigned m_nFXChainPhaserWet;
	unsigned m_nFXChainPhaserRate;
	unsigned m_nFXChainPhaserResonance;
	bool m_bFXChainTapeDelayEnable;
	unsigned m_nFXChainTapeDelayWet;
	unsigned m_nFXChainTapeDelayLeftDelayTime;
	unsigned m_nFXChainTapeDelayRightDelayTime;
	unsigned m_nFXChainTapeDelayFlutter;
	unsigned m_nFXChainTapeDelayFeedback;
	bool m_bFXChainShimmerReverbEnable;
	unsigned m_nFXChainShimmerReverbWet;
	unsigned m_nFXChainShimmerReverbDecay;
	unsigned m_nFXChainShimmerReverbDiffusion;
	unsigned m_nFXChainShimmerReverbPitchShift;
#endif
};

#endif
