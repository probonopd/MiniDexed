//
// minidexed.h
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
#ifndef _minidexed_h
#define _minidexed_h

#include "dexedadapter.h"
#include "config.h"
#include "userinterface.h"
#include "sysexfileloader.h"
#include "performanceconfig.h"
#include "midikeyboard.h"
#include "pckeyboard.h"
#include "serialmididevice.h"
#include "perftimer.h"
#include <fatfs/ff.h>
#include <stdint.h>
#include <string>
#include <circle/types.h>
#include <circle/interrupt.h>
#include <circle/gpiomanager.h>
#include <circle/i2cmaster.h>
#include <circle/multicore.h>
#include <circle/soundbasedevice.h>
#include <circle/spinlock.h>
#include "common.h"
#include "effect_mixer.hpp"
#include "effect_platervbstereo.h"
#include "effect_compressor.h"

class CMiniDexed
#ifdef ARM_ALLOW_MULTI_CORE
:	public CMultiCoreSupport
#endif
{
public:
	CMiniDexed (CConfig *pConfig, CInterruptSystem *pInterrupt,
		    CGPIOManager *pGPIOManager, CI2CMaster *pI2CMaster, FATFS *pFileSystem);

	bool Initialize (void);

	void Process (bool bPlugAndPlayUpdated);

#ifdef ARM_ALLOW_MULTI_CORE
	void Run (unsigned nCore);
#endif

	CSysExFileLoader *GetSysExFileLoader (void);

	void BankSelectLSB (unsigned nBankLSB, unsigned nTG);
	void ProgramChange (unsigned nProgram, unsigned nTG);
	void SetVolume (unsigned nVolume, unsigned nTG);
	void SetPan (unsigned nPan, unsigned nTG);			// 0 .. 127
	void SetMasterTune (int nMasterTune, unsigned nTG);		// -99 .. 99
	void SetCutoff (int nCutoff, unsigned nTG);			// 0 .. 99
	void SetResonance (int nResonance, unsigned nTG);		// 0 .. 99
	void SetMIDIChannel (uint8_t uchChannel, unsigned nTG);

	void keyup (int16_t pitch, unsigned nTG);
	void keydown (int16_t pitch, uint8_t velocity, unsigned nTG);

	void setSustain (bool sustain, unsigned nTG);
	void panic (uint8_t value, unsigned nTG);
	void notesOff (uint8_t value, unsigned nTG);
	void setModWheel (uint8_t value, unsigned nTG);
	void setPitchbend (int16_t value, unsigned nTG);
	void ControllersRefresh (unsigned nTG);

	void setFootController (uint8_t value, unsigned nTG);
	void setBreathController (uint8_t value, unsigned nTG);
	void setAftertouch (uint8_t value, unsigned nTG);

	void SetReverbSend (unsigned nReverbSend, unsigned nTG);			// 0 .. 127

	void setMonoMode(uint8_t mono, uint8_t nTG);
	void setPitchbendRange(uint8_t range, uint8_t nTG);
	void setPitchbendStep(uint8_t step, uint8_t nTG);
	void setPortamentoMode(uint8_t mode, uint8_t nTG);
	void setPortamentoGlissando(uint8_t glissando, uint8_t nTG);
	void setPortamentoTime(uint8_t time, uint8_t nTG);
	void setModWheelRange(uint8_t range, uint8_t nTG);
	void setModWheelTarget(uint8_t target, uint8_t nTG);
	void setFootControllerRange(uint8_t range, uint8_t nTG);
	void setFootControllerTarget(uint8_t target, uint8_t nTG);
	void setBreathControllerRange(uint8_t range, uint8_t nTG);
	void setBreathControllerTarget(uint8_t target, uint8_t nTG);
	void setAftertouchRange(uint8_t range, uint8_t nTG);
	void setAftertouchTarget(uint8_t target, uint8_t nTG);
	void loadVoiceParameters(const uint8_t* data, uint8_t nTG);
	void setVoiceDataElement(uint8_t data, uint8_t number, uint8_t nTG);
	void getSysExVoiceDump(uint8_t* dest, uint8_t nTG);

	void setModController (unsigned controller, unsigned parameter, uint8_t value, uint8_t nTG);
	unsigned getModController (unsigned controller, unsigned parameter, uint8_t nTG);

	int16_t checkSystemExclusive(const uint8_t* pMessage, const uint16_t nLength, uint8_t nTG);

	std::string GetPerformanceFileName(unsigned nID);
	std::string GetPerformanceName(unsigned nID);
	unsigned GetLastPerformance();
	unsigned GetActualPerformanceID();
	void SetActualPerformanceID(unsigned nID);
	bool SetNewPerformance(unsigned nID);
	bool SavePerformanceNewFile ();
	
	bool DoSavePerformanceNewFile (void);
	bool DoSetNewPerformance (void);
	bool GetPerformanceSelectToLoad(void);
	bool SavePerformance (bool bSaveAsDeault);
	
	enum TParameter
	{
		ParameterCompressorEnable,
		ParameterReverbEnable,
		ParameterReverbSize,
		ParameterReverbHighDamp,
		ParameterReverbLowDamp,
		ParameterReverbLowPass,
		ParameterReverbDiffusion,
		ParameterReverbLevel,
		ParameterUnknown
	};

	void SetParameter (TParameter Parameter, int nValue);
	int GetParameter (TParameter Parameter);

	std::string GetNewPerformanceDefaultName(void);
	void SetNewPerformanceName(std::string nName);
	void SetVoiceName (std::string VoiceName, unsigned nTG);
	bool DeletePerformance(unsigned nID);
	bool DoDeletePerformance(void);

	enum TTGParameter
	{
		TGParameterVoiceBank,
		TGParameterProgram,
		TGParameterVolume,
		TGParameterPan,
		TGParameterMasterTune,
		TGParameterCutoff,
		TGParameterResonance,
		TGParameterMIDIChannel,
		TGParameterReverbSend,
		TGParameterPitchBendRange, 
		TGParameterPitchBendStep,
		TGParameterPortamentoMode,
		TGParameterPortamentoGlissando,
		TGParameterPortamentoTime,
		TGParameterMonoMode,  
				
		TGParameterMWRange,
		TGParameterMWPitch,
		TGParameterMWAmplitude,
		TGParameterMWEGBias,
		
		TGParameterFCRange,
		TGParameterFCPitch,
		TGParameterFCAmplitude,
		TGParameterFCEGBias,
		
		TGParameterBCRange,
		TGParameterBCPitch,
		TGParameterBCAmplitude,
		TGParameterBCEGBias,
		
		TGParameterATRange,
		TGParameterATPitch,
		TGParameterATAmplitude,
		TGParameterATEGBias,
		
		TGParameterUnknown
	};

	void SetTGParameter (TTGParameter Parameter, int nValue, unsigned nTG);
	int GetTGParameter (TTGParameter Parameter, unsigned nTG);

	// access (global or OP-related) parameter of the active voice of a TG
	static const unsigned NoOP = 6;		// for global parameters
	void SetVoiceParameter (uint8_t uchOffset, uint8_t uchValue, unsigned nOP, unsigned nTG);
	uint8_t GetVoiceParameter (uint8_t uchOffset, unsigned nOP, unsigned nTG);

	std::string GetVoiceName (unsigned nTG);

	bool SavePerformance (void);
	bool DoSavePerformance (void);

	void setMasterVolume (float32_t vol);

private:
	int16_t ApplyNoteLimits (int16_t pitch, unsigned nTG);	// returns < 0 to ignore note
	uint8_t m_uchOPMask[CConfig::ToneGenerators];
	void LoadPerformanceParameters(void); 
	void ProcessSound (void);

#ifdef ARM_ALLOW_MULTI_CORE
	enum TCoreStatus
	{
		CoreStatusInit,
		CoreStatusIdle,
		CoreStatusBusy,
		CoreStatusExit,
		CoreStatusUnknown
	};
#endif

private:
	CConfig *m_pConfig;

	int m_nParameter[ParameterUnknown];			// global (non-TG) parameters

	CDexedAdapter *m_pTG[CConfig::ToneGenerators];

	unsigned m_nVoiceBankID[CConfig::ToneGenerators];
	unsigned m_nProgram[CConfig::ToneGenerators];
	unsigned m_nVolume[CConfig::ToneGenerators];
	unsigned m_nPan[CConfig::ToneGenerators];
	int m_nMasterTune[CConfig::ToneGenerators];
	int m_nCutoff[CConfig::ToneGenerators];
	int m_nResonance[CConfig::ToneGenerators];
	unsigned m_nMIDIChannel[CConfig::ToneGenerators];
	unsigned m_nPitchBendRange[CConfig::ToneGenerators];	
	unsigned m_nPitchBendStep[CConfig::ToneGenerators];	
	unsigned m_nPortamentoMode[CConfig::ToneGenerators];	
	unsigned m_nPortamentoGlissando[CConfig::ToneGenerators];	
	unsigned m_nPortamentoTime[CConfig::ToneGenerators];	
	bool m_bMonoMode[CConfig::ToneGenerators]; 
				
	unsigned m_nModulationWheelRange[CConfig::ToneGenerators];
	unsigned m_nModulationWheelTarget[CConfig::ToneGenerators];
	unsigned m_nFootControlRange[CConfig::ToneGenerators];
	unsigned m_nFootControlTarget[CConfig::ToneGenerators];
	unsigned m_nBreathControlRange[CConfig::ToneGenerators];	
	unsigned m_nBreathControlTarget[CConfig::ToneGenerators];	
	unsigned m_nAftertouchRange[CConfig::ToneGenerators];	
	unsigned m_nAftertouchTarget[CConfig::ToneGenerators];
		
	unsigned m_nNoteLimitLow[CConfig::ToneGenerators];
	unsigned m_nNoteLimitHigh[CConfig::ToneGenerators];
	int m_nNoteShift[CConfig::ToneGenerators];

	unsigned m_nReverbSend[CConfig::ToneGenerators];
  
	uint8_t m_nRawVoiceData[156]; 
	
	
	bool m_bSavePerformanceNewFile;
	bool m_bSetNewPerformance;
	unsigned m_nSetNewPerformanceID;
	
	float32_t nMasterVolume;

	bool	m_bDeletePerformance;
	unsigned m_nDeletePerformanceID;

	CUserInterface m_UI;
	CSysExFileLoader m_SysExFileLoader;
	CPerformanceConfig m_PerformanceConfig;

	CMIDIKeyboard *m_pMIDIKeyboard[CConfig::MaxUSBMIDIDevices];
	CPCKeyboard m_PCKeyboard;
	CSerialMIDIDevice m_SerialMIDI;
	bool m_bUseSerial;

	CSoundBaseDevice *m_pSoundDevice;
	bool m_bChannelsSwapped;
	unsigned m_nQueueSizeFrames;

#ifdef ARM_ALLOW_MULTI_CORE
	unsigned m_nActiveTGsLog2;
	volatile TCoreStatus m_CoreStatus[CORES];
	volatile unsigned m_nFramesToProcess;
	float32_t m_OutputLevel[CConfig::ToneGenerators][CConfig::MaxChunkSize];
#endif

	CPerformanceTimer m_GetChunkTimer;
	bool m_bProfileEnabled;

	AudioEffectPlateReverb* reverb;
	AudioStereoMixer<CConfig::ToneGenerators>* tg_mixer;
	AudioStereoMixer<CConfig::ToneGenerators>* reverb_send_mixer;

	CSpinLock m_ReverbSpinLock;

	bool m_bSavePerformance;
	bool m_bLoadPerformanceBusy;
	bool m_bSaveAsDeault;
};

#endif
