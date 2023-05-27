//
// uimenu.h
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
#ifndef _uimenu_h
#define _uimenu_h

#include <string>
#include <circle/timer.h>

class CMiniDexed;
class CUserInterface;

class CUIMenu
{
private:
	static const unsigned MaxMenuDepth = 5;

public:
	enum TMenuEvent
	{
		MenuEventUpdate,
		MenuEventSelect,
		MenuEventBack,
		MenuEventHome,
		MenuEventStepDown,
		MenuEventStepUp,
		MenuEventPressAndStepDown,
		MenuEventPressAndStepUp,
		MenuEventUnknown
	};

public:
	CUIMenu (CUserInterface *pUI, CMiniDexed *pMiniDexed);

	void EventHandler (TMenuEvent Event);
	
private:
	typedef void TMenuHandler (CUIMenu *pUIMenu, TMenuEvent Event);

	struct TMenuItem
	{
		const char *Name;
		TMenuHandler *Handler;
		const TMenuItem *MenuItem;
		unsigned Parameter;
	};

	typedef std::string TToString (int nValue);

	struct TParameter
	{
		int Minimum;
		int Maximum;
		int Increment;
		TToString *ToString;
	};

private:
	static void MenuHandler (CUIMenu *pUIMenu, TMenuEvent Event);
	static void EditGlobalParameter (CUIMenu *pUIMenu, TMenuEvent Event);
	static void EditVoiceBankNumber (CUIMenu *pUIMenu, TMenuEvent Event);
	static void EditProgramNumber (CUIMenu *pUIMenu, TMenuEvent Event);
	static void EditTGParameter (CUIMenu *pUIMenu, TMenuEvent Event);
	static void EditVoiceParameter (CUIMenu *pUIMenu, TMenuEvent Event);
	static void EditOPParameter (CUIMenu *pUIMenu, TMenuEvent Event);
	static void SavePerformance (CUIMenu *pUIMenu, TMenuEvent Event);
	static void EditTGParameter2 (CUIMenu *pUIMenu, TMenuEvent Event);
	static void EditTGParameterModulation (CUIMenu *pUIMenu, TMenuEvent Event); 	
	static void PerformanceMenu (CUIMenu *pUIMenu, TMenuEvent Event);
	static void SavePerformanceNewFile (CUIMenu *pUIMenu, TMenuEvent Event);
	
	static std::string GetGlobalValueString (unsigned nParameter, int nValue);
	static std::string GetTGValueString (unsigned nTGParameter, int nValue);
	static std::string GetVoiceValueString (unsigned nVoiceParameter, int nValue);
	static std::string GetOPValueString (unsigned nOPParameter, int nValue);

	static std::string ToVolume (int nValue);
	static std::string ToPan (int nValue);
	static std::string ToMIDIChannel (int nValue);

	static std::string ToAlgorithm (int nValue);
	static std::string ToOnOff (int nValue);
	static std::string ToLFOWaveform (int nValue);
	static std::string ToTransposeNote (int nValue);
	static std::string ToBreakpointNote (int nValue);
	static std::string ToKeyboardCurve (int nValue);
	static std::string ToOscillatorMode (int nValue);
	static std::string ToOscillatorDetune (int nValue);
	static std::string ToPortaMode (int nValue);  
	static std::string ToPortaGlissando (int nValue);   
	static std::string ToPolyMono (int nValue);

	void TGShortcutHandler (TMenuEvent Event);
	void OPShortcutHandler (TMenuEvent Event);

	static void TimerHandler (TKernelTimerHandle hTimer, void *pParam, void *pContext);

	static void InputTxt (CUIMenu *pUIMenu, TMenuEvent Event);
	static void TimerHandlerNoBack (TKernelTimerHandle hTimer, void *pParam, void *pContext);
	 
private:
	CUserInterface *m_pUI;
	CMiniDexed *m_pMiniDexed;

	const TMenuItem *m_pParentMenu;
	const TMenuItem *m_pCurrentMenu;
	unsigned m_nCurrentMenuItem;
	unsigned m_nCurrentSelection;
	unsigned m_nCurrentParameter;

	const TMenuItem *m_MenuStackParent[MaxMenuDepth];
	const TMenuItem *m_MenuStackMenu[MaxMenuDepth];
	unsigned m_nMenuStackItem[MaxMenuDepth];
	unsigned m_nMenuStackSelection[MaxMenuDepth];
	unsigned m_nMenuStackParameter[MaxMenuDepth];
	unsigned m_nCurrentMenuDepth;

	static const TMenuItem s_MenuRoot[];
	static const TMenuItem s_MainMenu[];
	static const TMenuItem s_TGMenu[];
	static const TMenuItem s_EffectsMenu[];
	static const TMenuItem s_ReverbMenu[];
	static const TMenuItem s_EditVoiceMenu[];
	static const TMenuItem s_OperatorMenu[];
	static const TMenuItem s_SaveMenu[];
	static const TMenuItem s_EditPitchBendMenu[];
	static const TMenuItem s_EditPortamentoMenu[];
	static const TMenuItem s_PerformanceMenu[];
	
	static const TMenuItem s_ModulationMenu[];
	static const TMenuItem s_ModulationMenuParameters[];
			
	static const TParameter s_GlobalParameter[];
	static const TParameter s_TGParameter[];
	static const TParameter s_VoiceParameter[];
	static const TParameter s_OPParameter[];

	static const char s_NoteName[100][4];

	std::string m_InputText="1234567890ABCD";
	unsigned m_InputTextPosition=0;
	unsigned m_InputTextChar=32;
	bool m_bPerformanceDeleteMode=false;
	bool m_bConfirmDeletePerformance=false;
	unsigned m_nSelectedPerformanceID =0;
	bool m_bSplashShow=false;

};

#endif
