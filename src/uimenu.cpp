//
// uimenu.cpp
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
#include "uimenu.h"
#include "minidexed.h"
#include "mididevice.h"
#include "userinterface.h"
#include "sysexfileloader.h"
#include "config.h"
#include <circle/sysconfig.h>
#include <assert.h>

using namespace std;

const CUIMenu::TMenuItem CUIMenu::s_MenuRoot[] =
{
	{"MiniDexed", MenuHandler, s_MainMenu},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_MainMenu[] =
{
	{"TG1",		MenuHandler,	s_TGMenu, 0},
#ifdef ARM_ALLOW_MULTI_CORE
	{"TG2",		MenuHandler,	s_TGMenu, 1},
	{"TG3",		MenuHandler,	s_TGMenu, 2},
	{"TG4",		MenuHandler,	s_TGMenu, 3},
	{"TG5",		MenuHandler,	s_TGMenu, 4},
	{"TG6",		MenuHandler,	s_TGMenu, 5},
	{"TG7",		MenuHandler,	s_TGMenu, 6},
	{"TG8",		MenuHandler,	s_TGMenu, 7},
#endif
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_TGMenu[] =
{
	{"Voice",	EditProgramNumber},
	{"Bank",	EditVoiceBankNumber},
	{"Volume",	EditTGParameter,	0,	CMiniDexed::TGParameterVolume},
#ifdef ARM_ALLOW_MULTI_CORE
	{"Pan",		EditTGParameter,	0,	CMiniDexed::TGParameterPan},
#endif
	{"Detune",	EditTGParameter,	0,	CMiniDexed::TGParameterMasterTune},
	{"Channel",	EditTGParameter,	0,	CMiniDexed::TGParameterMIDIChannel},
	{0}
};

// must match CMiniDexed::TTGParameter
const CUIMenu::TParameter CUIMenu::s_TGParameter[CMiniDexed::TGParameterUnknown] =
{
	{0,	CSysExFileLoader::MaxVoiceBankID,	1},		// TGParameterVoiceBank
	{0,	CSysExFileLoader::VoicesPerBank-1,	1},		// TGParameterProgram
	{0,	127,					8, ToVolume},	// TGParameterVolume
	{0,	127,					8, ToPan},	// TGParameterPan
	{-99,	99,					1},		// TGParameterMasterTune
	{0,	CMIDIDevice::ChannelUnknown-1,		1, ToMIDIChannel} // TGParameterMIDIChannel
};

CUIMenu::CUIMenu (CUserInterface *pUI, CMiniDexed *pMiniDexed)
:	m_pUI (pUI),
	m_pMiniDexed (pMiniDexed),
	m_pParentMenu (s_MenuRoot),
	m_pCurrentMenu (s_MainMenu),
	m_nCurrentMenuItem (0),
	m_nCurrentSelection (0),
	m_nCurrentParameter (0),
	m_nCurrentMenuDepth (0)
{
}

void CUIMenu::EventHandler (TMenuEvent Event)
{
	switch (Event)
	{
	case MenuEventBack:				// pop menu
		if (m_nCurrentMenuDepth)
		{
			m_nCurrentMenuDepth--;

			m_pParentMenu = m_MenuStackParent[m_nCurrentMenuDepth];
			m_pCurrentMenu = m_MenuStackMenu[m_nCurrentMenuDepth];
			m_nCurrentMenuItem = m_nMenuStackItem[m_nCurrentMenuDepth];
			m_nCurrentSelection = m_nMenuStackSelection[m_nCurrentMenuDepth];
			m_nCurrentParameter = m_nMenuStackParameter[m_nCurrentMenuDepth];

			EventHandler (MenuEventUpdate);
		}
		break;

	case MenuEventHome:
		m_pParentMenu = s_MenuRoot;
		m_pCurrentMenu = s_MainMenu;
		m_nCurrentMenuItem = 0;
		m_nCurrentSelection = 0;
		m_nCurrentParameter = 0;
		m_nCurrentMenuDepth = 0;

		EventHandler (MenuEventUpdate);
		break;

	default:
		(*m_pParentMenu[m_nCurrentMenuItem].Handler) (this, Event);
		break;
	}
}

void CUIMenu::MenuHandler (CUIMenu *pUIMenu, TMenuEvent Event)
{
	switch (Event)
	{
	case MenuEventUpdate:
		break;

	case MenuEventSelect:				// push menu
		assert (pUIMenu->m_nCurrentMenuDepth < MaxMenuDepth);
		pUIMenu->m_MenuStackParent[pUIMenu->m_nCurrentMenuDepth] = pUIMenu->m_pParentMenu;
		pUIMenu->m_MenuStackMenu[pUIMenu->m_nCurrentMenuDepth] = pUIMenu->m_pCurrentMenu;
		pUIMenu->m_nMenuStackItem[pUIMenu->m_nCurrentMenuDepth]
			= pUIMenu->m_nCurrentMenuItem;
		pUIMenu->m_nMenuStackSelection[pUIMenu->m_nCurrentMenuDepth]
			= pUIMenu->m_nCurrentSelection;
		pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth]
			= pUIMenu->m_nCurrentParameter;
		pUIMenu->m_nCurrentMenuDepth++;

		pUIMenu->m_pParentMenu = pUIMenu->m_pCurrentMenu;
		pUIMenu->m_nCurrentParameter =
			pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].Parameter;
		pUIMenu->m_pCurrentMenu =
			pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].MenuItem;
		pUIMenu->m_nCurrentMenuItem = pUIMenu->m_nCurrentSelection;
		pUIMenu->m_nCurrentSelection = 0;
		break;

	case MenuEventStepDown:
		if (pUIMenu->m_nCurrentSelection > 0)
		{
			pUIMenu->m_nCurrentSelection--;
		}
		break;

	case MenuEventStepUp:
		++pUIMenu->m_nCurrentSelection;
		if (!pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].Name)  // more entries?
		{
			pUIMenu->m_nCurrentSelection--;
		}
		break;

	default:
		assert (0);
		break;
	}

	if (pUIMenu->m_pCurrentMenu)				// if this is another menu?
	{
		pUIMenu->m_pUI->DisplayWrite (
			pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
			"",
			pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].Name,
			pUIMenu->m_nCurrentSelection > 0,
			!!pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection+1].Name);
	}
	else
	{
		pUIMenu->EventHandler (MenuEventUpdate);	// no, update parameter display
	}
}

void CUIMenu::EditVoiceBankNumber (CUIMenu *pUIMenu, TMenuEvent Event)
{
	unsigned nTG = pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth-1];

	int nValue = pUIMenu->m_pMiniDexed->GetTGParameter (CMiniDexed::TGParameterVoiceBank, nTG);

	switch (Event)
	{
	case MenuEventUpdate:
		break;

	case MenuEventStepDown:
		if (--nValue < 0)
		{
			nValue = 0;
		}
		pUIMenu->m_pMiniDexed->SetTGParameter (
			CMiniDexed::TGParameterVoiceBank, nValue, nTG);
		break;

	case MenuEventStepUp:
		if (++nValue > (int) CSysExFileLoader::MaxVoiceBankID)
		{
			nValue = CSysExFileLoader::MaxVoiceBankID;
		}
		pUIMenu->m_pMiniDexed->SetTGParameter (
			CMiniDexed::TGParameterVoiceBank, nValue, nTG);
		break;

	default:
		return;
	}

	string TG ("TG");
	TG += to_string (nTG+1);

	string Value =   to_string (nValue+1) + "="
		       + pUIMenu->m_pMiniDexed->GetSysExFileLoader ()->GetBankName (nValue);

	pUIMenu->m_pUI->DisplayWrite (TG.c_str (),
				      pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
				      Value.c_str (),
				      nValue > 0, nValue < (int) CSysExFileLoader::MaxVoiceBankID);
}

void CUIMenu::EditProgramNumber (CUIMenu *pUIMenu, TMenuEvent Event)
{
	unsigned nTG = pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth-1];

	int nValue = pUIMenu->m_pMiniDexed->GetTGParameter (CMiniDexed::TGParameterProgram, nTG);

	switch (Event)
	{
	case MenuEventUpdate:
		break;

	case MenuEventStepDown:
		if (--nValue < 0)
		{
			nValue = 0;
		}
		pUIMenu->m_pMiniDexed->SetTGParameter (CMiniDexed::TGParameterProgram, nValue, nTG);
		break;

	case MenuEventStepUp:
		if (++nValue > (int) CSysExFileLoader::VoicesPerBank-1)
		{
			nValue = CSysExFileLoader::VoicesPerBank-1;
		}
		pUIMenu->m_pMiniDexed->SetTGParameter (CMiniDexed::TGParameterProgram, nValue, nTG);
		break;

	default:
		return;
	}

	string TG ("TG");
	TG += to_string (nTG+1);

	string Value = to_string (nValue+1) + "=" + pUIMenu->m_pMiniDexed->GetVoiceName (nTG);

	pUIMenu->m_pUI->DisplayWrite (TG.c_str (),
				      pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
				      Value.c_str (),
				      nValue > 0, nValue < (int) CSysExFileLoader::VoicesPerBank-1);
}

void CUIMenu::EditTGParameter (CUIMenu *pUIMenu, TMenuEvent Event)
{
	unsigned nTG = pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth-1];

	int nValue = pUIMenu->m_pMiniDexed->GetTGParameter (
		(CMiniDexed::TTGParameter) pUIMenu->m_nCurrentParameter, nTG);

	switch (Event)
	{
	case MenuEventUpdate:
		break;

	case MenuEventStepDown:
		nValue -= s_TGParameter[pUIMenu->m_nCurrentParameter].Increment;
		if (nValue < s_TGParameter[pUIMenu->m_nCurrentParameter].Minimum)
		{
			nValue = s_TGParameter[pUIMenu->m_nCurrentParameter].Minimum;
		}
		pUIMenu->m_pMiniDexed->SetTGParameter (
			(CMiniDexed::TTGParameter) pUIMenu->m_nCurrentParameter, nValue, nTG);
		break;

	case MenuEventStepUp:
		nValue += s_TGParameter[pUIMenu->m_nCurrentParameter].Increment;
		if (nValue > s_TGParameter[pUIMenu->m_nCurrentParameter].Maximum)
		{
			nValue = s_TGParameter[pUIMenu->m_nCurrentParameter].Maximum;
		}
		pUIMenu->m_pMiniDexed->SetTGParameter (
			(CMiniDexed::TTGParameter) pUIMenu->m_nCurrentParameter, nValue, nTG);
		break;

	default:
		return;
	}

	string TG ("TG");
	TG += to_string (nTG+1);

	string Value = GetTGValueString (
		pUIMenu->m_nCurrentParameter,
		pUIMenu->m_pMiniDexed->GetTGParameter (
			(CMiniDexed::TTGParameter) pUIMenu->m_nCurrentParameter, nTG));

	pUIMenu->m_pUI->DisplayWrite (TG.c_str (),
				      pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
				      Value.c_str (),
				      nValue > s_TGParameter[pUIMenu->m_nCurrentParameter].Minimum, nValue < s_TGParameter[pUIMenu->m_nCurrentParameter].Maximum);
}

string CUIMenu::GetTGValueString (unsigned nTGParameter, int nValue)
{
	string Result;

	assert (nTGParameter < sizeof CUIMenu::s_TGParameter / sizeof CUIMenu::s_TGParameter[0]);

	CUIMenu::TToString *pToString = CUIMenu::s_TGParameter[nTGParameter].ToString;
	if (pToString)
	{
		Result = (*pToString) (nValue);
	}
	else
	{
		Result = to_string (nValue);
	}

	return Result;
}

string CUIMenu::ToVolume (int nValue)
{
	static const size_t MaxChars = CConfig::LCDColumns-2;
	char VolumeBar[MaxChars+1];
	memset (VolumeBar, 0xFF, sizeof VolumeBar);	// 0xFF is the block character
	VolumeBar[nValue * MaxChars / 127] = '\0';

	return VolumeBar;
}

string CUIMenu::ToPan (int nValue)
{
	assert (CConfig::LCDColumns == 16);
	static const size_t MaxChars = CConfig::LCDColumns-3;
	char PanMarker[MaxChars+1] = "......:......";
	unsigned nIndex = nValue * MaxChars / 127;
	if (nIndex == MaxChars)
	{
		nIndex--;
	}
	PanMarker[nIndex] = '\xFF';			// 0xFF is the block character

	return PanMarker;
}

string CUIMenu::ToMIDIChannel (int nValue)
{
	switch (nValue)
	{
	case CMIDIDevice::OmniMode:	return "Omni";
	case CMIDIDevice::Disabled:	return "Off";
	default:			return to_string (nValue+1);
	}
}
