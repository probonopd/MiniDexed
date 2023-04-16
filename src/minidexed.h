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

#include "extra_features.h"
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
#include <circle/sound/soundbasedevice.h>
#include <circle/spinlock.h>
#include "common.h"
#include "effect_mixer.hpp"
#include "effect_platervbstereo.h"
#include "effect_compressor.h"

#if defined(MIXING_CONSOLE_ENABLE)
#include "mixing_console.hpp"

typedef MixingConsole<CConfig::ToneGenerators> Mixer;
#endif

class CMiniDexed
#ifdef ARM_ALLOW_MULTI_CORE
:	public CMultiCoreSupport
#endif
{
public:
	CMiniDexed(
		CConfig *pConfig, 
		CInterruptSystem *pInterrupt,
		CGPIOManager *pGPIOManager, 
		CI2CMaster *pI2CMaster, 
		FATFS *pFileSystem
	);

	bool Initialize (void);

	void Process (bool bPlugAndPlayUpdated);

#ifdef ARM_ALLOW_MULTI_CORE
	void Run (unsigned nCore);
#endif

	CSysExFileLoader *GetSysExFileLoader (void);

	void BankSelect    (unsigned nBank, unsigned nTG);
	void BankSelectMSB (unsigned nBankMSB, unsigned nTG);
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

#if defined(MIXING_CONSOLE_ENABLE)
	unsigned getMixingConsoleSendLevel(unsigned nTG, MixerOutput fx) const;
	void setMixingConsoleSendLevel(unsigned nTG, MixerOutput fx, unsigned nFXSend);
	void setMixingConsoleReturnLevel(MixerOutput ret, MixerOutput fx, unsigned nFXReturn);
#elif defined(PLATE_REVERB_ENABLE)
	void SetReverbSend (unsigned nReverbSend, unsigned nTG);			// 0 .. 127
#endif

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
	
	// Must match the order in CUIMenu::TParameter
	enum TParameter
	{
		ParameterCompressorEnable,

	#if defined(PLATE_REVERB_ENABLE) || defined(MIXING_CONSOLE_ENABLE) 
		// Plate Reverb parameters
		ParameterReverbEnable,
		ParameterReverbSize,
		ParameterReverbHighDamp,
		ParameterReverbLowDamp,
		ParameterReverbLowPass,
		ParameterReverbDiffusion,
		ParameterReverbLevel,
	#endif

	// BEGIN FX global parameters definition
	#if defined(MIXING_CONSOLE_ENABLE)

		// Tube parameters
		ParameterFXTubeEnable,
		ParameterFXTubeOverdrive,

		// Chorus parameters
		ParameterFXChorusEnable,
		ParameterFXChorusRate,
		ParameterFXChorusDepth,
		
		// Flanger parameters
		ParameterFXFlangerEnable,
		ParameterFXFlangerRate,
		ParameterFXFlangerDepth,
		ParameterFXFlangerFeedback,

		// Orbitone parameters
		ParameterFXOrbitoneEnable,
		ParameterFXOrbitoneRate,
		ParameterFXOrbitoneDepth,

		// Phaser parameters
		ParameterFXPhaserEnable,
		ParameterFXPhaserRate,
		ParameterFXPhaserDepth,
		ParameterFXPhaserFeedback,
		ParameterFXPhaserNbStages,

		// Delay parameters
		ParameterFXDelayEnable,
		ParameterFXDelayLeftDelayTime,
		ParameterFXDelayRightDelayTime,
		ParameterFXDelayFeedback,
		ParameterFXDelayFlutterRate,
		ParameterFXDelayFlutterAmount,

		// Reverberator parameters
		ParameterFXReverberatorEnable,
		ParameterFXReverberatorInputGain,
		ParameterFXReverberatorTime,
		ParameterFXReverberatorDiffusion,
		ParameterFXReverberatorLP,

		// Tube Return parameters
		ParameterFXTube_ChorusReturn,
		ParameterFXTube_FlangerReturn,
		ParameterFXTube_OrbitoneReturn,
		ParameterFXTube_PhaserReturn,
		ParameterFXTube_DelayReturn,
		ParameterFXTube_PlateReverbReturn,
		ParameterFXTube_ReverberatorReturn,
		ParameterFXTube_MainOutput,

		// Chorus Return parameters
		ParameterFXChorus_TubeReturn,
		ParameterFXChorus_FlangerReturn,
		ParameterFXChorus_OrbitoneReturn,
		ParameterFXChorus_PhaserReturn,
		ParameterFXChorus_DelayReturn,
		ParameterFXChorus_PlateReverbReturn,
		ParameterFXChorus_ReverberatorReturn,
		ParameterFXChorus_MainOutput,

		// Flanger Return parameters
		ParameterFXFlanger_TubeReturn,
		ParameterFXFlanger_ChorusReturn,
		ParameterFXFlanger_OrbitoneReturn,
		ParameterFXFlanger_PhaserReturn,
		ParameterFXFlanger_DelayReturn,
		ParameterFXFlanger_PlateReverbReturn,
		ParameterFXFlanger_ReverberatorReturn,
		ParameterFXFlanger_MainOutput,

		// Orbitone Return parameters
		ParameterFXOrbitone_TubeReturn,
		ParameterFXOrbitone_ChorusReturn,
		ParameterFXOrbitone_FlangerReturn,
		ParameterFXOrbitone_PhaserReturn,
		ParameterFXOrbitone_DelayReturn,
		ParameterFXOrbitone_PlateReverbReturn,
		ParameterFXOrbitone_ReverberatorReturn,
		ParameterFXOrbitone_MainOutput,

		// Phaser Return parameters
		ParameterFXPhaser_TubeReturn,
		ParameterFXPhaser_ChorusReturn,
		ParameterFXPhaser_FlangerReturn,
		ParameterFXPhaser_OrbitoneReturn,
		ParameterFXPhaser_DelayReturn,
		ParameterFXPhaser_PlateReverbReturn,
		ParameterFXPhaser_ReverberatorReturn,
		ParameterFXPhaser_MainOutput,

		// Delay Return parameters
		ParameterFXDelay_TubeReturn,
		ParameterFXDelay_ChorusReturn,
		ParameterFXDelay_FlangerReturn,
		ParameterFXDelay_OrbitoneReturn,
		ParameterFXDelay_PhaserReturn,
		ParameterFXDelay_PlateReverbReturn,
		ParameterFXDelay_ReverberatorReturn,
		ParameterFXDelay_MainOutput,

		// Plate Reverb Return parameters
		ParameterFXPlateReverb_TubeReturn,
		ParameterFXPlateReverb_ChorusReturn,
		ParameterFXPlateReverb_FlangerReturn,
		ParameterFXPlateReverb_OrbitoneReturn,
		ParameterFXPlateReverb_PhaserReturn,
		ParameterFXPlateReverb_DelayReturn,
		ParameterFXPlateReverb_ReverberatorReturn,
		ParameterFXPlateReverb_MainOutput,

		// Reverberator Return parameters
		ParameterFXReverberator_TubeReturn,
		ParameterFXReverberator_ChorusReturn,
		ParameterFXReverberator_FlangerReturn,
		ParameterFXReverberator_OrbitoneReturn,
		ParameterFXReverberator_PhaserReturn,
		ParameterFXReverberator_DelayReturn,
		ParameterFXReverberator_PlateReverbReturn,
		ParameterFXReverberator_MainOutput,

	#endif
	// END FX global parameters definition

		ParameterUnknown
	};

	void SetParameter (TParameter Parameter, int nValue);
	int GetParameter (TParameter Parameter);

	std::string GetNewPerformanceDefaultName(void);
	void SetNewPerformanceName(std::string nName);
	void SetVoiceName (std::string VoiceName, unsigned nTG);
	bool DeletePerformance(unsigned nID);
	bool DoDeletePerformance(void);

	// Must match the order in CUIMenu::TGParameter
	enum TTGParameter
	{
		TGParameterVoiceBank,
		TGParameterVoiceBankMSB,
		TGParameterVoiceBankLSB,
		TGParameterProgram,
		TGParameterVolume,
		TGParameterPan,
		TGParameterMasterTune,
		TGParameterCutoff,
		TGParameterResonance,
		TGParameterMIDIChannel,
#if defined(PLATE_REVERB_ENABLE)
		TGParameterReverbSend,
#endif
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
		
#if defined(MIXING_CONSOLE_ENABLE)
		TGParameterMixingSendFXTube,
		TGParameterMixingSendFXChorus,
		TGParameterMixingSendFXFlanger,
		TGParameterMixingSendFXOrbittone,
		TGParameterMixingSendFXPhaser,
		TGParameterMixingSendFXDelay,
		TGParameterMixingSendFXPlateReverb,
		TGParameterMixingSendFXReverberator,
		TGParameterMixingSendFXMainOutput,
#endif // MIXING_CONSOLE_ENABLE

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
	unsigned m_nVoiceBankIDMSB[CConfig::ToneGenerators];
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

#ifdef MIXING_CONSOLE_ENABLE
	unsigned m_nFXSendLevel[CConfig::ToneGenerators][MixerOutput::kFXCount];
#elif defined(PLATE_REVERB_ENABLE)
	unsigned m_nReverbSend[CConfig::ToneGenerators];
#endif

	uint8_t m_nRawVoiceData[156]; 
	
	
	float32_t nMasterVolume;

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

#if defined(MIXING_CONSOLE_ENABLE)
	Mixer* mixing_console_;
#elif defined(PLATE_REVERB_ENABLE)
	AudioEffectPlateReverb* reverb;
	AudioStereoMixer<CConfig::ToneGenerators>* tg_mixer;
	AudioStereoMixer<CConfig::ToneGenerators>* reverb_send_mixer;
#endif

	CSpinLock m_FXSpinLock;


	bool m_bSavePerformance;
	bool m_bSavePerformanceNewFile;
	bool m_bSetNewPerformance;
	unsigned m_nSetNewPerformanceID;	
	bool	m_bDeletePerformance;
	unsigned m_nDeletePerformanceID;
	bool m_bLoadPerformanceBusy;
	bool m_bSaveAsDeault;
};

#endif
