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
#include <vector>
#define NUM_VOICE_PARAM 156
#define NUM_PERFORMANCES 128
#define NUM_PERFORMANCE_BANKS 128

class CPerformanceConfig	// Performance configuration
{
public:
	CPerformanceConfig (FATFS *pFileSystem);
	~CPerformanceConfig (void);
	
	bool Init (unsigned nToneGenerators);

	bool Load (void);

	bool Save (void);

	// TG#
	unsigned GetBankNumber (unsigned nTG) const;		// 0 .. 127
	unsigned GetVoiceNumber (unsigned nTG) const;		// 0 .. 31
	unsigned GetMIDIChannel (unsigned nTG) const;		// 0 .. 15, omni, off
	unsigned GetVolume (unsigned nTG) const;		// 0 .. 127
	unsigned GetPan (unsigned nTG) const;			// 0 .. 127
	unsigned GetInsertFX (unsigned nTG) const;		// 0 .. X
	std::vector<unsigned> GetInsertFXParams (unsigned nTG) const;
	unsigned GetMidiFX (unsigned nTG) const;		// 0 .. X
	std::vector<unsigned> GetMidiFXParams (unsigned nTG) const;
	int GetDetune (unsigned nTG) const;			// -99 .. 99
	unsigned GetCutoff (unsigned nTG) const;		// 0 .. 99
	unsigned GetResonance (unsigned nTG) const;		// 0 .. 99
	unsigned GetNoteLimitLow (unsigned nTG) const;		// 0 .. 127
	unsigned GetNoteLimitHigh (unsigned nTG) const;		// 0 .. 127
	int GetNoteShift (unsigned nTG) const;			// -24 .. 24
	unsigned GetSendFX1Send (unsigned nTG) const;		// 0 .. 127
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
	void SetInsertFX (unsigned nValue, unsigned nTG);
	void SetInsertFXParams (std::vector<unsigned> pParams, unsigned nTG);
	void SetMidiFX (unsigned nValue, unsigned nTG);
	void SetMidiFXParams (std::vector<unsigned> pParams, unsigned nTG);
	void SetDetune (int nValue, unsigned nTG);
	void SetCutoff (unsigned nValue, unsigned nTG);
	void SetResonance (unsigned nValue, unsigned nTG);
	void SetNoteLimitLow (unsigned nValue, unsigned nTG);
	void SetNoteLimitHigh (unsigned nValue, unsigned nTG);
	void SetNoteShift (int nValue, unsigned nTG);
	void SetSendFX1Send (unsigned nValue, unsigned nTG);
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
	unsigned GetSendFX1 (void) const;
	std::vector<unsigned> GetSendFX1Params (void) const;
	unsigned GetSendFX1SendLevel (void) const;
	unsigned GetSendFX1Level (void) const;
	unsigned GetSendFX2 (void) const;
	std::vector<unsigned> GetSendFX2Params (void) const;
	unsigned GetSendFX2Level (void) const;
	unsigned GetMasterFX (void) const;
	std::vector<unsigned> GetMasterFXParams (void) const;
	bool GetReverbEnable (void) const;
	unsigned GetReverbSize (void) const;			// 0 .. 99
	unsigned GetReverbHighDamp (void) const;		// 0 .. 99
	unsigned GetReverbLowDamp (void) const;			// 0 .. 99
	unsigned GetReverbLowPass (void) const;			// 0 .. 99
	unsigned GetReverbDiffusion (void) const;		// 0 .. 99
	unsigned GetReverbLevel (void) const;			// 0 .. 99
	unsigned GetTempo (void) const;

	void SetCompressorEnable (bool bValue);
	void SetSendFX1 (unsigned nValue);
	void SetSendFX1Params (std::vector<unsigned> pParams);
	void SetSendFX1SendLevel (unsigned nValue);
	void SetSendFX1Level (unsigned nValue);
	void SetSendFX2 (unsigned nValue);
	void SetSendFX2Params (std::vector<unsigned> pParams);
	void SetSendFX2Level (unsigned nValue);
	void SetMasterFX (unsigned nValue);
	void SetMasterFXParams (std::vector<unsigned> pParams);
	void SetReverbEnable (bool bValue);
	void SetReverbSize (unsigned nValue);
	void SetReverbHighDamp (unsigned nValue);
	void SetReverbLowDamp (unsigned nValue);
	void SetReverbLowPass (unsigned nValue);
	void SetReverbDiffusion (unsigned nValue);
	void SetReverbLevel (unsigned nValue);
	void SetTempo (unsigned nValue);

	bool VoiceDataFilled(unsigned nTG);
	bool ListPerformances(); 
	//std::string m_DirName;
	void SetNewPerformance (unsigned nID);
	unsigned FindFirstPerformance (void);
	std::string GetPerformanceFileName(unsigned nID);
	std::string GetPerformanceFullFilePath(unsigned nID);
	std::string GetPerformanceName(unsigned nID);
	unsigned GetLastPerformance();
	unsigned GetLastPerformanceBank();
	void SetActualPerformanceID(unsigned nID);
	unsigned GetActualPerformanceID();
	void SetActualPerformanceBankID(unsigned nBankID);
	unsigned GetActualPerformanceBankID();
	bool CreateNewPerformanceFile(void);
	bool GetInternalFolderOk(); 
	std::string GetNewPerformanceDefaultName(void);
	void SetNewPerformanceName(const std::string &Name);
	bool DeletePerformance(unsigned nID);
	bool CheckFreePerformanceSlot(void);
	std::string AddPerformanceBankDirName(unsigned nBankID);
	bool IsValidPerformance(unsigned nID);

	bool ListPerformanceBanks(void); 
	void SetNewPerformanceBank(unsigned nBankID);
	unsigned GetPerformanceBank(void);
	std::string GetPerformanceBankName(unsigned nBankID);
	bool IsValidPerformanceBank(unsigned nBankID);

private:
	CPropertiesFatFsFile m_Properties;
	
	unsigned m_nToneGenerators;

	unsigned m_nBankNumber[CConfig::AllToneGenerators];
	unsigned m_nVoiceNumber[CConfig::AllToneGenerators];
	unsigned m_nMIDIChannel[CConfig::AllToneGenerators];
	unsigned m_nVolume[CConfig::AllToneGenerators];
	unsigned m_nPan[CConfig::AllToneGenerators];
	unsigned m_nInsertFX[CConfig::AllToneGenerators];
	std::string m_sInsertFXParams[CConfig::AllToneGenerators];
	unsigned m_nMidiFX[CConfig::AllToneGenerators];
	std::string m_sMidiFXParams[CConfig::AllToneGenerators];
	int m_nDetune[CConfig::AllToneGenerators];
	unsigned m_nCutoff[CConfig::AllToneGenerators];
	unsigned m_nResonance[CConfig::AllToneGenerators];
	unsigned m_nNoteLimitLow[CConfig::AllToneGenerators];
	unsigned m_nNoteLimitHigh[CConfig::AllToneGenerators];
	int m_nNoteShift[CConfig::AllToneGenerators];
	int m_nSendFX1Send[CConfig::AllToneGenerators];
	int m_nReverbSend[CConfig::AllToneGenerators];
	unsigned m_nPitchBendRange[CConfig::AllToneGenerators];
	unsigned m_nPitchBendStep[CConfig::AllToneGenerators];
	unsigned m_nPortamentoMode[CConfig::AllToneGenerators];
	unsigned m_nPortamentoGlissando[CConfig::AllToneGenerators];
	unsigned m_nPortamentoTime[CConfig::AllToneGenerators];
	std::string m_nVoiceDataTxt[CConfig::AllToneGenerators]; 
	bool m_bMonoMode[CConfig::AllToneGenerators]; 

	unsigned m_nModulationWheelRange[CConfig::AllToneGenerators];
	unsigned m_nModulationWheelTarget[CConfig::AllToneGenerators];
	unsigned m_nFootControlRange[CConfig::AllToneGenerators];	
	unsigned m_nFootControlTarget[CConfig::AllToneGenerators];	
	unsigned m_nBreathControlRange[CConfig::AllToneGenerators];	
	unsigned m_nBreathControlTarget[CConfig::AllToneGenerators];	
	unsigned m_nAftertouchRange[CConfig::AllToneGenerators];	
	unsigned m_nAftertouchTarget[CConfig::AllToneGenerators];	

	unsigned m_nLastPerformance;  
	unsigned m_nActualPerformance = 0;  
	unsigned m_nActualPerformanceBank = 0;  
	unsigned m_nPerformanceBank;
	unsigned m_nLastPerformanceBank;  
	bool     m_bPerformanceDirectoryExists;
	//unsigned nMenuSelectedPerformance = 0; 
	std::string m_PerformanceFileName[NUM_PERFORMANCES];
	std::string m_PerformanceBankName[NUM_PERFORMANCE_BANKS];
	FATFS *m_pFileSystem; 

	std::string NewPerformanceName="";
	
	bool m_bCompressorEnable;
	unsigned m_nSendFX1;
	std::string m_sSendFX1Params;
	unsigned m_nSendFX1SendLevel;
	unsigned m_nSendFX1Level;
	unsigned m_nSendFX2;
	std::string m_sSendFX2Params;
	unsigned m_nSendFX2Level;
	unsigned m_nMasterFX;
	std::string m_sMasterFXParams;
	bool m_bReverbEnable;
	unsigned m_nReverbSize;
	unsigned m_nReverbHighDamp;
	unsigned m_nReverbLowDamp;
	unsigned m_nReverbLowPass;
	unsigned m_nReverbDiffusion;
	unsigned m_nReverbLevel;
	unsigned m_nTempo;

	std::string VectorToString (std::vector<unsigned> pParams);
	std::vector<unsigned> StringToVector (std::string sParams) const;
};

#endif
