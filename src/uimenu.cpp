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
#include <cmath>
#include <circle/sysconfig.h>
#include <assert.h>

using namespace std;

const CUIMenu::TMenuItem CUIMenu::s_MenuRoot[] =
{
	{"MiniDexed", CUIMenu::MenuHandler, CUIMenu::s_MainMenu},
	{0}
};

// inserting menu items before "TG1" affect TGShortcutHandler()
const CUIMenu::TMenuItem CUIMenu::s_MainMenu[] =
{
	{"TG1",			CUIMenu::MenuHandler, CUIMenu::s_TGMenu, 0},
#ifdef ARM_ALLOW_MULTI_CORE
	{"TG2",			CUIMenu::MenuHandler, CUIMenu::s_TGMenu, 1},
	{"TG3",			CUIMenu::MenuHandler, CUIMenu::s_TGMenu, 2},
	{"TG4",			CUIMenu::MenuHandler, CUIMenu::s_TGMenu, 3},
	{"TG5",			CUIMenu::MenuHandler, CUIMenu::s_TGMenu, 4},
	{"TG6",			CUIMenu::MenuHandler, CUIMenu::s_TGMenu, 5},
	{"TG7",			CUIMenu::MenuHandler, CUIMenu::s_TGMenu, 6},
	{"TG8",			CUIMenu::MenuHandler, CUIMenu::s_TGMenu, 7},
#endif
	{"Effects",		CUIMenu::MenuHandler, CUIMenu::s_EffectsMenu},
	{"Performance",	CUIMenu::MenuHandler, CUIMenu::s_PerformanceMenu}, 
	{0}
};

#if defined(MIXING_CONSOLE_ENABLE)
const CUIMenu::TMenuItem CUIMenu::s_TGFXMenu[] = 
{
	{"Tube-Send", 	CUIMenu::EditTGParameter, 0, CMiniDexed::TTGParameter::TGParameterMixingSendFXTube},
	{"Chorus-Send", CUIMenu::EditTGParameter, 0, CMiniDexed::TTGParameter::TGParameterMixingSendFXChorus},
	{"FlangR-Send", CUIMenu::EditTGParameter, 0, CMiniDexed::TTGParameter::TGParameterMixingSendFXFlanger},
	{"Orb-Send", 	CUIMenu::EditTGParameter, 0, CMiniDexed::TTGParameter::TGParameterMixingSendFXOrbittone},
	{"Phaser-Send", CUIMenu::EditTGParameter, 0, CMiniDexed::TTGParameter::TGParameterMixingSendFXPhaser},
	{"Delay-Send", 	CUIMenu::EditTGParameter, 0, CMiniDexed::TTGParameter::TGParameterMixingSendFXDelay},
	{"Plate-Send",  CUIMenu::EditTGParameter, 0, CMiniDexed::TTGParameter::TGParameterMixingSendFXPlateReverb},
	{"Rev-Send",  	CUIMenu::EditTGParameter, 0, CMiniDexed::TTGParameter::TGParameterMixingSendFXReverberator},
	{"Main Out", 	CUIMenu::EditTGParameter, 0, CMiniDexed::TTGParameter::TGParameterMixingSendFXMainOutput},
	{0}
};
#endif

const CUIMenu::TMenuItem CUIMenu::s_TGMenu[] =
{
	{"Voice",		CUIMenu::EditProgramNumber},
	{"Bank",		CUIMenu::EditVoiceBankNumber},
	{"Volume",		CUIMenu::EditTGParameter,	0, CMiniDexed::TTGParameter::TGParameterVolume},
#ifdef ARM_ALLOW_MULTI_CORE
	{"Pan",			CUIMenu::EditTGParameter,	0, CMiniDexed::TTGParameter::TGParameterPan},
#endif
#if defined(MIXING_CONSOLE_ENABLE)
	{"FX-Send", 	CUIMenu::MenuHandler, 		CUIMenu::s_TGFXMenu},
#elif defined(PLATE_REVERB_ENABLE)
	{"Reverb-Send",	CUIMenu::EditTGParameter,	0, CMiniDexed::TTGParameter::TGParameterReverbSend},
#endif
	{"Detune",		CUIMenu::EditTGParameter,	0, CMiniDexed::TTGParameter::TGParameterMasterTune},
	{"Cutoff",		CUIMenu::EditTGParameter,	0, CMiniDexed::TTGParameter::TGParameterCutoff},
	{"Resonance",	CUIMenu::EditTGParameter,	0, CMiniDexed::TTGParameter::TGParameterResonance},
	{"Pitch Bend",	CUIMenu::MenuHandler,		CUIMenu::s_EditPitchBendMenu},
	{"Portamento",	CUIMenu::MenuHandler,		CUIMenu::s_EditPortamentoMenu},
	{"Poly/Mono",	CUIMenu::EditTGParameter,	0, CMiniDexed::TTGParameter::TGParameterMonoMode}, 
	{"Modulation",	CUIMenu::MenuHandler,		CUIMenu::s_ModulationMenu},
	{"Channel",		CUIMenu::EditTGParameter,	0, CMiniDexed::TTGParameter::TGParameterMIDIChannel},
	{"Edit Voice",	CUIMenu::MenuHandler,		CUIMenu::s_EditVoiceMenu},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_EffectsMenu[] =
{
	{"Compress",CUIMenu::EditGlobalParameter, 0, CMiniDexed::ParameterCompressorEnable},
#if defined(MIXING_CONSOLE_ENABLE)
	{"Tube", 	CUIMenu::MenuHandler, CUIMenu::s_FXTube},
	{"Chorus", 	CUIMenu::MenuHandler, CUIMenu::s_FXChorus},
	{"FlangR", 	CUIMenu::MenuHandler, CUIMenu::s_FXFlanger},
	{"Orb", 	CUIMenu::MenuHandler, CUIMenu::s_FXOrbitone},
	{"PhasR", 	CUIMenu::MenuHandler, CUIMenu::s_FXPhaser},
	{"Delay", 	CUIMenu::MenuHandler, CUIMenu::s_FXDelay},
#endif
#if defined(PLATE_REVERB_ENABLE)
	{"Reverb", 	CUIMenu::MenuHandler, CUIMenu::s_FXPlateReverb},
#elif defined(MIXING_CONSOLE_ENABLE)
	{"Plt Rvb", CUIMenu::MenuHandler, CUIMenu::s_FXPlateReverb},
	{"Rvbrtor", CUIMenu::MenuHandler, CUIMenu::s_FXReverberator},
#endif
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_EditPitchBendMenu[] =
{
	{"Bend Range",	CUIMenu::EditTGParameter2, 0, CMiniDexed::TTGParameter::TGParameterPitchBendRange},
	{"Bend Step",	CUIMenu::EditTGParameter2, 0, CMiniDexed::TTGParameter::TGParameterPitchBendStep},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_EditPortamentoMenu[] =
{
	{"Mode",		CUIMenu::EditTGParameter2, 0, CMiniDexed::TTGParameter::TGParameterPortamentoMode},
	{"Glissando",	CUIMenu::EditTGParameter2, 0, CMiniDexed::TTGParameter::TGParameterPortamentoGlissando},
	{"Time",		CUIMenu::EditTGParameter2, 0, CMiniDexed::TTGParameter::TGParameterPortamentoTime},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_ModulationMenu[] =
{
	{"Mod. Wheel",		CUIMenu::MenuHandler, CUIMenu::s_ModulationMenuParameters, CMiniDexed::TTGParameter::TGParameterMWRange},
	{"Foot Control",	CUIMenu::MenuHandler, CUIMenu::s_ModulationMenuParameters, CMiniDexed::TTGParameter::TGParameterFCRange},
	{"Breath Control",	CUIMenu::MenuHandler, CUIMenu::s_ModulationMenuParameters, CMiniDexed::TTGParameter::TGParameterBCRange},
	{"Aftertouch",		CUIMenu::MenuHandler, CUIMenu::s_ModulationMenuParameters, CMiniDexed::TTGParameter::TGParameterATRange},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_ModulationMenuParameters[] =
{
	{"Range",		CUIMenu::EditTGParameterModulation, 0, 0},
	{"Pitch",		CUIMenu::EditTGParameterModulation, 0, 1},
	{"Amplitude",	CUIMenu::EditTGParameterModulation, 0, 2},
	{"EG Bias",		CUIMenu::EditTGParameterModulation, 0, 3},
	{0}
};

#if defined(PLATE_REVERB_ENABLE) || defined(MIXING_CONSOLE_ENABLE)

const CUIMenu::TMenuItem CUIMenu::s_FXPlateReverb[] =
{
	{"Enable",		CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterReverbEnable},
	{"Size",		CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterReverbSize},
	{"High damp",	CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterReverbHighDamp},
	{"Low damp",	CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterReverbLowDamp},
	{"Low pass",	CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterReverbLowPass},
	{"Diffusion",	CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterReverbDiffusion},
	{"Level",		CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterReverbLevel},
#if defined(MIXING_CONSOLE_ENABLE)
	{"Return", 		CUIMenu::MenuHandler, CUIMenu::s_FXPlateReverbReturn},
#endif
	{0}
};

#endif

#if defined(MIXING_CONSOLE_ENABLE)

const CUIMenu::TMenuItem CUIMenu::s_FXTube[] =
{
	{"Enable", 	CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXTubeEnable},
	{"Overdrv",	CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXTubeOverdrive},
	{"Return", 	CUIMenu::MenuHandler, CUIMenu::s_FXTubeReturn},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_FXChorus[] =
{
	{"Enable", 	CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXChorusEnable},
	{"Rate",	CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXChorusRate},
	{"Depth", 	CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXChorusDepth},
	{"Return", 	CUIMenu::MenuHandler, CUIMenu::s_FXChorusReturn},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_FXFlanger[] =
{
	{"Enable", 	CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXFlangerEnable},
	{"Rate",	CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXFlangerRate},
	{"Depth",	CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXFlangerDepth},
	{"Feedbck", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXFlangerFeedback},
	{"Return", 	CUIMenu::MenuHandler, CUIMenu::s_FXFlangerReturn},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_FXOrbitone[] =
{
	{"Enable", 	CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXOrbitoneEnable},
	{"Rate", 	CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXOrbitoneRate},
	{"Depth", 	CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXOrbitoneDepth},
	{"Return", 	CUIMenu::MenuHandler, CUIMenu::s_FXOrbitoneReturn},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_FXPhaser[] =
{
	{"Enable", 	CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXPhaserEnable},
	{"Rate",	CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXPhaserRate},
	{"Depth",	CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXPhaserDepth},
	{"Feedbck",	CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXPhaserFeedback},
	{"Stages",	CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXPhaserNbStages},
	{"Return", 	CUIMenu::MenuHandler, CUIMenu::s_FXPhaserReturn},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_FXDelay[] =
{
	{"Enable", 	CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXDelayEnable},
	{"L Delay",	CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXDelayLeftDelayTime},
	{"R Delay",	CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXDelayRightDelayTime},
	{"Feedbck", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXDelayFeedback},
	{"Flt Rte", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXDelayFlutterRate},
	{"Flt Amt", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXDelayFlutterAmount},
	{"Return", 	CUIMenu::MenuHandler, CUIMenu::s_FXDelayReturn},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_FXReverberator[] =
{
	{"Enable", 	CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXReverberatorEnable},
	{"Gain",	CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXReverberatorInputGain},
	{"Time",	CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXReverberatorTime},
	{"Diffus",	CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXReverberatorDiffusion},
	{"LowPass",	CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXReverberatorLP},
	{"Return", 	CUIMenu::MenuHandler, CUIMenu::s_FXReverberatorReturn},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_FXTubeReturn[] =
{
	{"ChR Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::ParameterFXTube_ChorusReturn},
	{"Flg Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::ParameterFXTube_FlangerReturn},
	{"Orb Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::ParameterFXTube_OrbitoneReturn},
	{"PhR Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::ParameterFXTube_PhaserReturn},
	{"Dly Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::ParameterFXTube_DelayReturn},
	{"Plt Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::ParameterFXTube_PlateReverbReturn},
	{"Rev Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::ParameterFXTube_ReverberatorReturn},
	{"MainOut", CUIMenu::EditGlobalParameter, 0, CMiniDexed::ParameterFXTube_MainOutput},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_FXChorusReturn[] =
{
	{"Tub Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXChorus_TubeReturn},
	{"Flg Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXChorus_FlangerReturn},
	{"Orb Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXChorus_OrbitoneReturn},
	{"PhR Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXChorus_PhaserReturn},
	{"Dly Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXChorus_DelayReturn},
	{"Plt Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXChorus_PlateReverbReturn},
	{"Rev Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXChorus_ReverberatorReturn},
	{"MainOut", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXChorus_MainOutput},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_FXFlangerReturn[] =
{
	{"Tub Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXFlanger_TubeReturn},
	{"ChR Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXFlanger_ChorusReturn},
	{"Orb Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXFlanger_OrbitoneReturn},
	{"PhR Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXFlanger_PhaserReturn},
	{"Dly Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXFlanger_DelayReturn},
	{"Plt Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXFlanger_PlateReverbReturn},
	{"Rev Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXFlanger_ReverberatorReturn},
	{"MainOut", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXFlanger_MainOutput},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_FXOrbitoneReturn[] =
{
	{"Tub Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXOrbitone_TubeReturn},
	{"ChR Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXOrbitone_ChorusReturn},
	{"Flg Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXOrbitone_FlangerReturn},
	{"PhR Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXOrbitone_PhaserReturn},
	{"Dly Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXOrbitone_DelayReturn},
	{"Plt Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXOrbitone_PlateReverbReturn},
	{"Rev Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXOrbitone_ReverberatorReturn},
	{"MainOut", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXOrbitone_MainOutput},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_FXPhaserReturn[] =
{
	{"Tub Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXPhaser_TubeReturn},
	{"ChR Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXPhaser_ChorusReturn},
	{"Flg Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXPhaser_FlangerReturn},
	{"Orb Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXPhaser_OrbitoneReturn},
	{"Dly Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXPhaser_DelayReturn},
	{"Plt Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXPhaser_PlateReverbReturn},
	{"Rev Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXPhaser_ReverberatorReturn},
	{"MainOut", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXPhaser_MainOutput},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_FXDelayReturn[] =
{
	{"Tub Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXDelay_TubeReturn},
	{"ChR Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXDelay_ChorusReturn},
	{"Flg Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXDelay_FlangerReturn},
	{"Orb Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXDelay_OrbitoneReturn},
	{"PhR Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXDelay_PhaserReturn},
	{"Plt Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXDelay_PlateReverbReturn},
	{"Rev Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXDelay_ReverberatorReturn},
	{"MainOut", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXDelay_MainOutput},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_FXPlateReverbReturn[] =
{
	{"Tub Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXPlateReverb_TubeReturn},
	{"ChR Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXPlateReverb_ChorusReturn},
	{"Flg Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXPlateReverb_FlangerReturn},
	{"Orb Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXPlateReverb_OrbitoneReturn},
	{"PhR Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXPlateReverb_PhaserReturn},
	{"Dly Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXPlateReverb_DelayReturn},
	{"Rev Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXPlateReverb_ReverberatorReturn},
	{"MainOut", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXPlateReverb_MainOutput},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_FXReverberatorReturn[] =
{
	{"Tub Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXReverberator_TubeReturn},
	{"ChR Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXReverberator_ChorusReturn},
	{"Flg Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXReverberator_FlangerReturn},
	{"Orb Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXReverberator_OrbitoneReturn},
	{"PhR Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXReverberator_PhaserReturn},
	{"Dly Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXReverberator_DelayReturn},
	{"Plt Rtn", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXReverberator_PlateReverbReturn},
	{"MainOut", CUIMenu::EditGlobalParameter, 0, CMiniDexed::TParameter::ParameterFXReverberator_MainOutput},
	{0}
};

#endif

// inserting menu items before "OP1" affect OPShortcutHandler()
const CUIMenu::TMenuItem CUIMenu::s_EditVoiceMenu[] =
{
	{"OP1",			CUIMenu::MenuHandler,		CUIMenu::s_OperatorMenu, 0},
	{"OP2",			CUIMenu::MenuHandler,		CUIMenu::s_OperatorMenu, 1},
	{"OP3",			CUIMenu::MenuHandler,		CUIMenu::s_OperatorMenu, 2},
	{"OP4",			CUIMenu::MenuHandler,		CUIMenu::s_OperatorMenu, 3},
	{"OP5",			CUIMenu::MenuHandler,		CUIMenu::s_OperatorMenu, 4},
	{"OP6",			CUIMenu::MenuHandler,		CUIMenu::s_OperatorMenu, 5},
	{"Algorithm",	CUIMenu::EditVoiceParameter,	0, 	DEXED_ALGORITHM},
	{"Feedback",	CUIMenu::EditVoiceParameter,	0, 	DEXED_FEEDBACK},
	{"P EG Rate 1",	CUIMenu::EditVoiceParameter,	0, 	DEXED_PITCH_EG_R1},
	{"P EG Rate 2",	CUIMenu::EditVoiceParameter,	0, 	DEXED_PITCH_EG_R2},
	{"P EG Rate 3",	CUIMenu::EditVoiceParameter,	0, 	DEXED_PITCH_EG_R3},
	{"P EG Rate 4",	CUIMenu::EditVoiceParameter,	0, 	DEXED_PITCH_EG_R4},
	{"P EG Level 1",CUIMenu::EditVoiceParameter,	0, 	DEXED_PITCH_EG_L1},
	{"P EG Level 2",CUIMenu::EditVoiceParameter,	0, 	DEXED_PITCH_EG_L2},
	{"P EG Level 3",CUIMenu::EditVoiceParameter,	0, 	DEXED_PITCH_EG_L3},
	{"P EG Level 4",CUIMenu::EditVoiceParameter,	0, 	DEXED_PITCH_EG_L4},
	{"Osc Key Sync",CUIMenu::EditVoiceParameter,	0, 	DEXED_OSC_KEY_SYNC},
	{"LFO Speed",	CUIMenu::EditVoiceParameter,	0, 	DEXED_LFO_SPEED},
	{"LFO Delay",	CUIMenu::EditVoiceParameter,	0, 	DEXED_LFO_DELAY},
	{"LFO PMD",		CUIMenu::EditVoiceParameter,	0, 	DEXED_LFO_PITCH_MOD_DEP},
	{"LFO AMD",		CUIMenu::EditVoiceParameter,	0, 	DEXED_LFO_AMP_MOD_DEP},
	{"LFO Sync",	CUIMenu::EditVoiceParameter,	0, 	DEXED_LFO_SYNC},
	{"LFO Wave",	CUIMenu::EditVoiceParameter,	0, 	DEXED_LFO_WAVE},
	{"P Mod Sens.",	CUIMenu::EditVoiceParameter,	0, 	DEXED_LFO_PITCH_MOD_SENS},
	{"Transpose",	CUIMenu::EditVoiceParameter,	0, 	DEXED_TRANSPOSE},
	{"Name",		CUIMenu::InputTxt,				0, 	3}, 
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_OperatorMenu[] =
{
	{"Output Level",CUIMenu::EditOPParameter, 0, DEXED_OP_OUTPUT_LEV},
	{"Freq Coarse",	CUIMenu::EditOPParameter, 0, DEXED_OP_FREQ_COARSE},
	{"Freq Fine",	CUIMenu::EditOPParameter, 0, DEXED_OP_FREQ_FINE},
	{"Osc Detune",	CUIMenu::EditOPParameter, 0, DEXED_OP_OSC_DETUNE},
	{"Osc Mode",	CUIMenu::EditOPParameter, 0, DEXED_OP_OSC_MODE},
	{"EG Rate 1",	CUIMenu::EditOPParameter, 0, DEXED_OP_EG_R1},
	{"EG Rate 2",	CUIMenu::EditOPParameter, 0, DEXED_OP_EG_R2},
	{"EG Rate 3",	CUIMenu::EditOPParameter, 0, DEXED_OP_EG_R3},
	{"EG Rate 4",	CUIMenu::EditOPParameter, 0, DEXED_OP_EG_R4},
	{"EG Level 1",	CUIMenu::EditOPParameter, 0, DEXED_OP_EG_L1},
	{"EG Level 2",	CUIMenu::EditOPParameter, 0, DEXED_OP_EG_L2},
	{"EG Level 3",	CUIMenu::EditOPParameter, 0, DEXED_OP_EG_L3},
	{"EG Level 4",	CUIMenu::EditOPParameter, 0, DEXED_OP_EG_L4},
	{"Break Point",	CUIMenu::EditOPParameter, 0, DEXED_OP_LEV_SCL_BRK_PT},
	{"L Key Depth",	CUIMenu::EditOPParameter, 0, DEXED_OP_SCL_LEFT_DEPTH},
	{"R Key Depth",	CUIMenu::EditOPParameter, 0, DEXED_OP_SCL_RGHT_DEPTH},
	{"L Key Scale",	CUIMenu::EditOPParameter, 0, DEXED_OP_SCL_LEFT_CURVE},
	{"R Key Scale",	CUIMenu::EditOPParameter, 0, DEXED_OP_SCL_RGHT_CURVE},
	{"Rate Scaling",CUIMenu::EditOPParameter, 0, DEXED_OP_OSC_RATE_SCALE},
	{"A Mod Sens.",	CUIMenu::EditOPParameter, 0, DEXED_OP_AMP_MOD_SENS},
	{"K Vel. Sens.",CUIMenu::EditOPParameter, 0, DEXED_OP_KEY_VEL_SENS},
	{"Enable", 		CUIMenu::EditOPParameter, 0, DEXED_OP_ENABLE},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_SaveMenu[] =
{
	{"Overwrite", CUIMenu::SavePerformance, 0, 0}, 
	{"New", CUIMenu::InputTxt, 0, 1}, 
	{"Save as default",	CUIMenu::SavePerformance, 0, 1}, 
	{0}
};

// must match CMiniDexed::TParameter
const CUIMenu::TParameter CUIMenu::s_GlobalParameter[CMiniDexed::TParameter::ParameterUnknown] =
{
	{0,	1,	1,	ToOnOff}	// ParameterCompressorEnable

#if defined(PLATE_REVERB_ENABLE) || defined(MIXING_CONSOLE_ENABLE) 
	,
	{0,	1,	1,	ToOnOff},	// ParameterReverbEnable
	{0,	99,	1},				// ParameterReverbSize
	{0,	99,	1},				// ParameterReverbHighDamp
	{0,	99,	1},				// ParameterReverbLowDamp
	{0,	99,	1},				// ParameterReverbLowPass
	{0,	99,	1},				// ParameterReverbDiffusion
	{0,	99,	1}				// ParameterReverbLevel
#endif 

// BEGIN FX global parameters mapping definition
#if defined(MIXING_CONSOLE_ENABLE)
	,
	// FX > Tube parameters
	{0,	1,	1,	ToOnOff},	// ParameterFXTubeEnable
	{0,	99,	1},				// ParameterFXTubeOverdrive

	// FX > Chorus parameters
	{0,	1,	1,	ToOnOff},	// ParameterFXChorusEnable
	{0,	99,	1},				// ParameterFXChorusRate
	{0,	99,	1},				// ParameterFXChorusDepth

	// FX > Flanger parameters
	{0,	1,	1,	ToOnOff},	// ParameterFXFlangerEnable
	{0,	99,	1},				// ParameterFXFlangerRate
	{0,	99,	1},				// ParameterFXFlangerDepth
	{0,	99,	1},				// ParameterFXFlangerFeedback

	// FX > Orbitone parameters
	{0,	1,	1,	ToOnOff},	// ParameterFXOrbitoneEnable
	{0,	99,	1},				// ParameterFXOrbitoneRate
	{0,	99,	1},				// ParameterFXOrbitoneDepth

	// FX > Phaser parameters
	{0,	1,	1,	ToOnOff},	// ParameterFXPhaserEnable
	{0,	99,	1},				// ParameterFXPhaserRate
	{0,	99,	1},				// ParameterFXPhaserDepth
	{0,	99,	1},				// ParameterFXPhaserFeedback
	{2,	MAX_NB_PHASES,	1},	// ParameterFXPhaserNbStages

	// FX > Delay parameters
	{0,	1,	1,	ToOnOff},	// ParameterFXDelayEnable
	{0,	99,	1},				// ParameterFXDelayLeftDelayTime
	{0,	99,	1},				// ParameterFXDelayRightDelayTime
	{0,	99,	1},				// ParameterFXDelayFeedback
	{0,	99,	1},				// ParameterFXDelayFlutterRate
	{0,	99,	1},				// ParameterFXDelayFlutterAmount

	// FX > Reverberator parameters
	{0,	1,	1,	ToOnOff},	// ParameterFXReverberatorEnable
	{0,	99,	1},				// ParameterFXReverberatorInputGain
	{0,	99,	1},				// ParameterFXReverberatorTime
	{0,	99,	1},				// ParameterFXReverberatorDiffusion
	{0,	99,	1},				// ParameterFXReverberatorLP

	// FX > Tube Return parameters
	{0, 99, 1},				// ParameterFXTube_ChorusReturn,
	{0, 99, 1},				// ParameterFXTube_FlangerReturn,
	{0, 99, 1},				// ParameterFXTube_OrbitoneReturn,
	{0, 99, 1},				// ParameterFXTube_PhaserReturn,
	{0, 99, 1},				// ParameterFXTube_DelayReturn,
	{0, 99, 1},				// ParameterFXTube_PlateReverbReturn,
	{0, 99, 1},				// ParameterFXTube_ReverberatorReturn,
	{0, 99, 1},				// ParameterFXTube_MainOutput,

	// FX > Chorus Return parameters
	{0, 99, 1},				// ParameterFXChorus_TubeReturn,
	{0, 99, 1},				// ParameterFXChorus_FlangerReturn,
	{0, 99, 1},				// ParameterFXChorus_OrbitoneReturn,
	{0, 99, 1},				// ParameterFXChorus_PhaserReturn,
	{0, 99, 1},				// ParameterFXChorus_DelayReturn,
	{0, 99, 1},				// ParameterFXChorus_PlateReverbReturn,
	{0, 99, 1},				// ParameterFXChorus_ReverberatorReturn,
	{0, 99, 1},				// ParameterFXChorus_MainOutput,

	// FX > Flanger Return parameters
	{0, 99, 1},				// ParameterFXFlanger_TubeReturn,
	{0, 99, 1},				// ParameterFXFlanger_ChorusReturn,
	{0, 99, 1},				// ParameterFXFlanger_OrbitoneReturn,
	{0, 99, 1},				// ParameterFXFlanger_PhaserReturn,
	{0, 99, 1},				// ParameterFXFlanger_DelayReturn,
	{0, 99, 1},				// ParameterFXFlanger_PlateReverbReturn,
	{0, 99, 1},				// ParameterFXFlanger_ReverberatorReturn,
	{0, 99, 1},				// ParameterFXFlanger_MainOutput,

	// FX > Orbitone Return parameters
	{0, 99, 1},				// ParameterFXOrbitone_TubeReturn,
	{0, 99, 1},				// ParameterFXOrbitone_ChorusReturn,
	{0, 99, 1},				// ParameterFXOrbitone_FlangerReturn,
	{0, 99, 1},				// ParameterFXOrbitone_PhaserReturn,
	{0, 99, 1},				// ParameterFXOrbitone_DelayReturn,
	{0, 99, 1},				// ParameterFXOrbitone_PlateReverbReturn,
	{0, 99, 1},				// ParameterFXOrbitone_ReverberatorReturn,
	{0, 99, 1},				// ParameterFXOrbitone_MainOutput,

	// FX > Phaser Return parameters
	{0, 99, 1},				// ParameterFXPhaser_TubeReturn,
	{0, 99, 1},				// ParameterFXPhaser_ChorusReturn,
	{0, 99, 1},				// ParameterFXPhaser_FlangerReturn,
	{0, 99, 1},				// ParameterFXPhaser_OrbitoneReturn,
	{0, 99, 1},				// ParameterFXPhaser_DelayReturn,
	{0, 99, 1},				// ParameterFXPhaser_PlateReverbReturn,
	{0, 99, 1},				// ParameterFXPhaser_ReverberatorReturn,
	{0, 99, 1},				// ParameterFXPhaser_MainOutput,

	// FX > Delay Return parameters
	{0, 99, 1},				// ParameterFXDelay_TubeReturn,
	{0, 99, 1},				// ParameterFXDelay_ChorusReturn,
	{0, 99, 1},				// ParameterFXDelay_FlangerReturn,
	{0, 99, 1},				// ParameterFXDelay_OrbitoneReturn,
	{0, 99, 1},				// ParameterFXDelay_PhaserReturn,
	{0, 99, 1},				// ParameterFXDelay_PlateReverbReturn,
	{0, 99, 1},				// ParameterFXDelay_ReverberatorReturn,
	{0, 99, 1},				// ParameterFXDelay_MainOutput,

	// FX > Reverb Return parameters
	{0, 99, 1},				// ParameterFXReverb_TubeReturn,
	{0, 99, 1},				// ParameterFXReverb_ChorusReturn,
	{0, 99, 1},				// ParameterFXReverb_FlangerReturn,
	{0, 99, 1},				// ParameterFXReverb_OrbitoneReturn,
	{0, 99, 1},				// ParameterFXReverb_PhaserReturn,
	{0, 99, 1},				// ParameterFXReverb_DelayReturn,
	{0, 99, 1},				// ParameterFXReverb_ReverberatorReturn,
	{0, 99, 1},				// ParameterFXReverb_MainOutput,

	// FX > Reverberator Return parameters
	{0, 99, 1},				// ParameterFXReverberator_TubeReturn,
	{0, 99, 1},				// ParameterFXReverberator_ChorusReturn,
	{0, 99, 1},				// ParameterFXReverberator_FlangerReturn,
	{0, 99, 1},				// ParameterFXReverberator_OrbitoneReturn,
	{0, 99, 1},				// ParameterFXReverberator_PhaserReturn,
	{0, 99, 1},				// ParameterFXReverberator_DelayReturn,
	{0, 99, 1},				// ParameterFXReverberator_PlateReverbReturn,
	{0, 99, 1}				// ParameterFXReverberator_MainOutput,

#endif
// END FX global parameters mapping definition
};

// must match CMiniDexed::TTGParameter
const CUIMenu::TParameter CUIMenu::s_TGParameter[CMiniDexed::TTGParameter::TGParameterUnknown] =
{
	{0,	CSysExFileLoader::MaxVoiceBankID,	1},	// TGParameterVoiceBank
	{0, 0, 0},									// TGParameterVoiceBankMSB (not used in menus)
	{0, 0, 0},									// TGParameterVoiceBankLSB (not used in menus)
	{0,	CSysExFileLoader::VoicesPerBank-1,	1},	// TGParameterProgram
	{0, 127, 8, ToVolume},						// TGParameterVolume
	{0, 127, 8, ToPan},							// TGParameterPan
	{-99, 99, 1},								// TGParameterMasterTune
	{0, 99, 1},									// TGParameterCutoff
	{0, 99, 1},									// TGParameterResonance
	{0, CMIDIDevice::ChannelUnknown-1, 1, ToMIDIChannel}, // TGParameterMIDIChannel
#if defined(PLATE_REVERB_ENABLE)
	{0, 99, 1},									// TGParameterReverbSend
#endif
	{0, 12, 1},									// TGParameterPitchBendRange
	{0, 12, 1},									// TGParameterPitchBendStep
	{0, 1, 1, ToPortaMode},						// TGParameterPortamentoMode
	{0, 1, 1, ToPortaGlissando},				// TGParameterPortamentoGlissando
	{0, 99, 1},									// TGParameterPortamentoTime
	{0, 1, 1, ToPolyMono}, 						// TGParameterMonoMode 
	{0, 99, 1}, 								//MW Range
	{0, 1, 1, ToOnOff}, 						//MW Pitch
	{0, 1, 1, ToOnOff}, 						//MW Amp
	{0, 1, 1, ToOnOff}, 						//MW EGBias
	{0, 99, 1}, 								//FC Range
	{0, 1, 1, ToOnOff}, 						//FC Pitch
	{0, 1, 1, ToOnOff}, 						//FC Amp
	{0, 1, 1, ToOnOff}, 						//FC EGBias
	{0, 99, 1}, 								//BC Range
	{0, 1, 1, ToOnOff}, 						//BC Pitch
	{0, 1, 1, ToOnOff}, 						//BC Amp
	{0, 1, 1, ToOnOff}, 						//BC EGBias
	{0, 99, 1}, 								//AT Range
	{0, 1, 1, ToOnOff}, 						//AT Pitch
	{0, 1, 1, ToOnOff}, 						//AT Amp
	{0, 1, 1, ToOnOff} 							//AT EGBias	
#if defined(MIXING_CONSOLE_ENABLE)
	,
	{0, 99, 1}, 								// TGParameterMixingSendFXTube,
	{0, 99, 1}, 								// TGParameterMixingSendFXChorus,
	{0, 99, 1}, 								// TGParameterMixingSendFXFlanger,
	{0, 99, 1}, 								// TGParameterMixingSendFXOrbittone,
	{0, 99, 1}, 								// TGParameterMixingSendFXPhaser,
	{0, 99, 1}, 								// TGParameterMixingSendFXDelay,
	{0, 99, 1}, 								// TGParameterMixingSendFXPlateReverb,
	{0, 99, 1}, 								// TGParameterMixingSendFXReverberator,
	{0, 99, 1}	 								// TGParameterMixingSendFXMainOutput,
#endif // MIXING_CONSOLE_ENABLE	
};

// must match DexedVoiceParameters in Synth_Dexed
const CUIMenu::TParameter CUIMenu::s_VoiceParameter[] =
{
	{0,	99,	1},						// DEXED_PITCH_EG_R1
	{0,	99,	1},						// DEXED_PITCH_EG_R2
	{0,	99,	1},						// DEXED_PITCH_EG_R3
	{0,	99,	1},						// DEXED_PITCH_EG_R4
	{0,	99,	1},						// DEXED_PITCH_EG_L1
	{0,	99,	1},						// DEXED_PITCH_EG_L2
	{0,	99,	1},						// DEXED_PITCH_EG_L3
	{0,	99,	1},						// DEXED_PITCH_EG_L4
	{0,	31,	1, ToAlgorithm},		// DEXED_ALGORITHM
	{0,	 7,	1},						// DEXED_FEEDBACK
	{0,	 1,	1, ToOnOff},			// DEXED_OSC_KEY_SYNC
	{0,	99,	1},						// DEXED_LFO_SPEED
	{0,	99,	1},						// DEXED_LFO_DELAY
	{0,	99,	1},						// DEXED_LFO_PITCH_MOD_DEP
	{0,	99,	1},						// DEXED_LFO_AMP_MOD_DEP
	{0,	 1,	1, ToOnOff},			// DEXED_LFO_SYNC
	{0,	 5,	1, ToLFOWaveform},		// DEXED_LFO_WAVE
	{0,	 7,	1},						// DEXED_LFO_PITCH_MOD_SENS
	{0,	48,	1, ToTransposeNote},	// DEXED_TRANSPOSE
	{0,	 1,	1}						// Voice Name - Dummy parameters for in case new item would be added in future 
};

// must match DexedVoiceOPParameters in Synth_Dexed
const CUIMenu::TParameter CUIMenu::s_OPParameter[] =
{
	{0,	99,	1},						// DEXED_OP_EG_R1
	{0,	99,	1},						// DEXED_OP_EG_R2
	{0,	99,	1},						// DEXED_OP_EG_R3
	{0,	99,	1},						// DEXED_OP_EG_R4
	{0,	99,	1},						// DEXED_OP_EG_L1
	{0,	99,	1},						// DEXED_OP_EG_L2
	{0,	99,	1},						// DEXED_OP_EG_L3
	{0,	99,	1},						// DEXED_OP_EG_L4
	{0,	99,	1, ToBreakpointNote},	// DEXED_OP_LEV_SCL_BRK_PT
	{0,	99,	1},						// DEXED_OP_SCL_LEFT_DEPTH
	{0,	99,	1},						// DEXED_OP_SCL_RGHT_DEPTH
	{0,	 3,	1, ToKeyboardCurve},	// DEXED_OP_SCL_LEFT_CURVE
	{0,	 3,	1, ToKeyboardCurve},	// DEXED_OP_SCL_RGHT_CURVE
	{0,	 7,	1},						// DEXED_OP_OSC_RATE_SCALE
	{0,	 3,	1},						// DEXED_OP_AMP_MOD_SENS
	{0,	 7,	1},						// DEXED_OP_KEY_VEL_SENS
	{0,	99,	1},						// DEXED_OP_OUTPUT_LEV
	{0,	 1, 1, ToOscillatorMode},	// DEXED_OP_OSC_MODE
	{0,	31,	1},						// DEXED_OP_FREQ_COARSE
	{0,	99,	1},						// DEXED_OP_FREQ_FINE
	{0,	14,	1, ToOscillatorDetune},	// DEXED_OP_OSC_DETUNE
	{0,  1, 1, ToOnOff}				// DEXED_OP_ENABLE
};

const char CUIMenu::s_NoteName[100][4] =
{
	"A1", "A#1", "B1", "C1", "C#1", "D1", "D#1", "E1", "F1", "F#1",	"G1", "G#1",
	"A2", "A#2", "B2", "C2", "C#2", "D2", "D#2", "E2", "F2", "F#2", "G2", "G#2",
	"A3", "A#3", "B3", "C3", "C#3", "D3", "D#3", "E3", "F3", "F#3", "G3", "G#3",
	"A4", "A#4", "B4", "C4", "C#4", "D4", "D#4", "E4", "F4", "F#4", "G4", "G#4",
	"A5", "A#5", "B5", "C5", "C#5", "D5", "D#5", "E5", "F5", "F#5", "G5", "G#5",
	"A6", "A#6", "B6", "C6", "C#6", "D6", "D#6", "E6", "F6", "F#6", "G6", "G#6",
	"A7", "A#7", "B7", "C7", "C#7", "D7", "D#7", "E7", "F7", "F#7", "G7", "G#7",
	"A8", "A#8", "B8", "C8", "C#8", "D8", "D#8", "E8", "F8", "F#8", "G8", "G#8",
	"A9", "A#9", "B9", "C9"
};
static const unsigned NoteC3 = 27;

const CUIMenu::TMenuItem CUIMenu::s_PerformanceMenu[] =
{
	{"Load",	CUIMenu::PerformanceMenu, 0, 0}, 
	{"Save",	CUIMenu::MenuHandler, s_SaveMenu},
	{"Delete",	CUIMenu::PerformanceMenu, 0, 1}, 
	{0}
};

CUIMenu::CUIMenu (CUserInterface *pUI, CMiniDexed *pMiniDexed) : 
	m_pUI (pUI),
	m_pMiniDexed (pMiniDexed),
	m_pParentMenu (s_MenuRoot),
	m_pCurrentMenu (s_MainMenu),
	m_nCurrentMenuItem (0),
	m_nCurrentSelection (0),
	m_nCurrentParameter (0),
	m_nCurrentMenuDepth (0)
{
	assert(pMiniDexed);
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
		return;
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

void CUIMenu::EditGlobalParameter (CUIMenu *pUIMenu, TMenuEvent Event)
{
	CMiniDexed::TParameter Param = (CMiniDexed::TParameter) pUIMenu->m_nCurrentParameter;
	const TParameter &rParam = s_GlobalParameter[Param];

	int nValue = pUIMenu->m_pMiniDexed->GetParameter (Param);

	switch (Event)
	{
	case MenuEventUpdate:
		break;

	case MenuEventStepDown:
		nValue -= rParam.Increment;
		if (nValue < rParam.Minimum)
		{
			nValue = rParam.Minimum;
		}
		pUIMenu->m_pMiniDexed->SetParameter (Param, nValue);
		break;

	case MenuEventStepUp:
		nValue += rParam.Increment;
		if (nValue > rParam.Maximum)
		{
			nValue = rParam.Maximum;
		}
		pUIMenu->m_pMiniDexed->SetParameter (Param, nValue);
		break;

	default:
		return;
	}

	const char *pMenuName =
		pUIMenu->m_MenuStackParent[pUIMenu->m_nCurrentMenuDepth-1]
			[pUIMenu->m_nMenuStackItem[pUIMenu->m_nCurrentMenuDepth-1]].Name;

	string Value = GetGlobalValueString (Param, pUIMenu->m_pMiniDexed->GetParameter (Param));

	pUIMenu->m_pUI->DisplayWrite (pMenuName,
				      pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
				      Value.c_str (),
				      nValue > rParam.Minimum, nValue < rParam.Maximum);
}

void CUIMenu::EditVoiceBankNumber (CUIMenu *pUIMenu, TMenuEvent Event)
{
	unsigned nTG = pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth-1];

	int nValue = pUIMenu->m_pMiniDexed->GetTGParameter (CMiniDexed::TTGParameter::TGParameterVoiceBank, nTG);

	switch (Event)
	{
	case MenuEventUpdate:
		break;

	case MenuEventStepDown:
		nValue = pUIMenu->m_pMiniDexed->GetSysExFileLoader ()->GetNextBankDown(nValue);
		pUIMenu->m_pMiniDexed->SetTGParameter (
			CMiniDexed::TTGParameter::TGParameterVoiceBank, nValue, nTG);
		break;

	case MenuEventStepUp:
		nValue = pUIMenu->m_pMiniDexed->GetSysExFileLoader ()->GetNextBankUp(nValue);
		pUIMenu->m_pMiniDexed->SetTGParameter (
			CMiniDexed::TTGParameter::TGParameterVoiceBank, nValue, nTG);
		break;

	case MenuEventPressAndStepDown:
	case MenuEventPressAndStepUp:
		pUIMenu->TGShortcutHandler (Event);
		return;

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
	int nHighestBank = pUIMenu->m_pMiniDexed->GetSysExFileLoader ()->GetNumHighestBank();

	int nValue = pUIMenu->m_pMiniDexed->GetTGParameter (CMiniDexed::TTGParameter::TGParameterProgram, nTG);

	switch (Event)
	{
	case MenuEventUpdate:
		break;

	case MenuEventStepDown:
		if (--nValue < 0)
		{
			// Switch down a voice bank and set to the last voice
			nValue = CSysExFileLoader::VoicesPerBank-1;
			int nVB = pUIMenu->m_pMiniDexed->GetTGParameter(CMiniDexed::TTGParameter::TGParameterVoiceBank, nTG);
			if (--nVB < 0)
			{
				// Wrap around to last loaded bank
				nVB = nHighestBank;
			}
			pUIMenu->m_pMiniDexed->SetTGParameter (CMiniDexed::TTGParameter::TGParameterVoiceBank, nVB, nTG);
		}
		pUIMenu->m_pMiniDexed->SetTGParameter (CMiniDexed::TTGParameter::TGParameterProgram, nValue, nTG);
		break;

	case MenuEventStepUp:
		if (++nValue > (int) CSysExFileLoader::VoicesPerBank-1)
		{
			// Switch up a voice bank and reset to voice 0
			nValue = 0;
			int nVB = pUIMenu->m_pMiniDexed->GetTGParameter(CMiniDexed::TTGParameter::TGParameterVoiceBank, nTG);
			if (++nVB > (int) nHighestBank)
			{
				// Wrap around to first bank
				nVB = 0;
			}
			pUIMenu->m_pMiniDexed->SetTGParameter (CMiniDexed::TTGParameter::TGParameterVoiceBank, nVB, nTG);
		}
		pUIMenu->m_pMiniDexed->SetTGParameter (CMiniDexed::TTGParameter::TGParameterProgram, nValue, nTG);
		break;

	case MenuEventPressAndStepDown:
	case MenuEventPressAndStepUp:
		pUIMenu->TGShortcutHandler (Event);
		return;

	default:
		return;
	}

	string voiceName = pUIMenu->m_pMiniDexed->GetVoiceName (nTG); // Skip empty voices
	if (voiceName == "EMPTY     "
	    || voiceName == "          "
	    || voiceName == "----------"
	    || voiceName == "~~~~~~~~~~" )
	{
		if (Event == MenuEventStepUp) {
			CUIMenu::EditProgramNumber (pUIMenu, MenuEventStepUp);
		}
		if (Event == MenuEventStepDown) {
			CUIMenu::EditProgramNumber (pUIMenu, MenuEventStepDown);
		}
	} else {
		string TG ("TG");
		TG += to_string (nTG+1);

		string Value = to_string (nValue+1) + "=" + pUIMenu->m_pMiniDexed->GetVoiceName (nTG);

		pUIMenu->m_pUI->DisplayWrite (TG.c_str (),
					      pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
					      Value.c_str (),
					      nValue > 0, nValue < (int) CSysExFileLoader::VoicesPerBank-1);
	}
}

void CUIMenu::EditTGParameter (CUIMenu *pUIMenu, TMenuEvent Event)
{
	unsigned nTG = pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth-1];

	CMiniDexed::TTGParameter Param = (CMiniDexed::TTGParameter) pUIMenu->m_nCurrentParameter;
	const TParameter &rParam = s_TGParameter[Param];

	int nValue = pUIMenu->m_pMiniDexed->GetTGParameter (Param, nTG);

	switch (Event)
	{
	case MenuEventUpdate:
		break;

	case MenuEventStepDown:
		nValue -= rParam.Increment;
		if (nValue < rParam.Minimum)
		{
			nValue = rParam.Minimum;
		}
		pUIMenu->m_pMiniDexed->SetTGParameter (Param, nValue, nTG);
		break;

	case MenuEventStepUp:
		nValue += rParam.Increment;
		if (nValue > rParam.Maximum)
		{
			nValue = rParam.Maximum;
		}
		pUIMenu->m_pMiniDexed->SetTGParameter (Param, nValue, nTG);
		break;

	case MenuEventPressAndStepDown:
	case MenuEventPressAndStepUp:
		pUIMenu->TGShortcutHandler (Event);
		return;

	default:
		return;
	}

	string TG ("TG");
	TG += to_string (nTG+1);

	string Value = GetTGValueString (Param, pUIMenu->m_pMiniDexed->GetTGParameter (Param, nTG));

	pUIMenu->m_pUI->DisplayWrite (TG.c_str (),
				      pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
				      Value.c_str (),
				      nValue > rParam.Minimum, nValue < rParam.Maximum);
}

void CUIMenu::EditTGParameter2 (CUIMenu *pUIMenu, TMenuEvent Event) // second menu level. Redundant code but in order to not modified original code
{

	unsigned nTG = pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth-2]; 

	CMiniDexed::TTGParameter Param = (CMiniDexed::TTGParameter) pUIMenu->m_nCurrentParameter;
	const TParameter &rParam = s_TGParameter[Param];

	int nValue = pUIMenu->m_pMiniDexed->GetTGParameter (Param, nTG);

	switch (Event)
	{
	case MenuEventUpdate:
		break;

	case MenuEventStepDown:
		nValue -= rParam.Increment;
		if (nValue < rParam.Minimum)
		{
			nValue = rParam.Minimum;
		}
		pUIMenu->m_pMiniDexed->SetTGParameter (Param, nValue, nTG);
		break;

	case MenuEventStepUp:
		nValue += rParam.Increment;
		if (nValue > rParam.Maximum)
		{
			nValue = rParam.Maximum;
		}
		pUIMenu->m_pMiniDexed->SetTGParameter (Param, nValue, nTG);
		break;

	case MenuEventPressAndStepDown:
	case MenuEventPressAndStepUp:
		pUIMenu->TGShortcutHandler (Event);
		return;

	default:
		return;
	}

	string TG ("TG");
	TG += to_string (nTG+1);

	string Value = GetTGValueString (Param, pUIMenu->m_pMiniDexed->GetTGParameter (Param, nTG));

	pUIMenu->m_pUI->DisplayWrite (TG.c_str (),
				      pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
				      Value.c_str (),
				      nValue > rParam.Minimum, nValue < rParam.Maximum);
				   
}

void CUIMenu::EditVoiceParameter (CUIMenu *pUIMenu, TMenuEvent Event)
{
	unsigned nTG = pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth-2];

	unsigned nParam = pUIMenu->m_nCurrentParameter;
	const TParameter &rParam = s_VoiceParameter[nParam];

	int nValue = pUIMenu->m_pMiniDexed->GetVoiceParameter (nParam, CMiniDexed::NoOP, nTG);

	switch (Event)
	{
	case MenuEventUpdate:
		break;

	case MenuEventStepDown:
		nValue -= rParam.Increment;
		if (nValue < rParam.Minimum)
		{
			nValue = rParam.Minimum;
		}
		pUIMenu->m_pMiniDexed->SetVoiceParameter (nParam, nValue, CMiniDexed::NoOP, nTG);
		break;

	case MenuEventStepUp:
		nValue += rParam.Increment;
		if (nValue > rParam.Maximum)
		{
			nValue = rParam.Maximum;
		}
		pUIMenu->m_pMiniDexed->SetVoiceParameter (nParam, nValue, CMiniDexed::NoOP, nTG);
		break;

	case MenuEventPressAndStepDown:
	case MenuEventPressAndStepUp:
		pUIMenu->TGShortcutHandler (Event);
		return;

	default:
		return;
	}

	string TG ("TG");
	TG += to_string (nTG+1);

	string Value = GetVoiceValueString (nParam, nValue);

	pUIMenu->m_pUI->DisplayWrite (TG.c_str (),
				      pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
				      Value.c_str (),
				      nValue > rParam.Minimum, nValue < rParam.Maximum);
}

void CUIMenu::EditOPParameter (CUIMenu *pUIMenu, TMenuEvent Event)
{
	unsigned nTG = pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth-3];
	unsigned nOP = pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth-1];

	unsigned nParam = pUIMenu->m_nCurrentParameter;
	const TParameter &rParam = s_OPParameter[nParam];

	int nValue = pUIMenu->m_pMiniDexed->GetVoiceParameter (nParam, nOP, nTG);

	switch (Event)
	{
	case MenuEventUpdate:
		break;

	case MenuEventStepDown:
		nValue -= rParam.Increment;
		if (nValue < rParam.Minimum)
		{
			nValue = rParam.Minimum;
		}
		pUIMenu->m_pMiniDexed->SetVoiceParameter (nParam, nValue, nOP, nTG);
		break;

	case MenuEventStepUp:
		nValue += rParam.Increment;
		if (nValue > rParam.Maximum)
		{
			nValue = rParam.Maximum;
		}
		pUIMenu->m_pMiniDexed->SetVoiceParameter (nParam, nValue, nOP, nTG);
		break;

	case MenuEventPressAndStepDown:
	case MenuEventPressAndStepUp:
		pUIMenu->OPShortcutHandler (Event);
		return;

	default:
		return;
	}

	string OP ("OP");
	OP += to_string (nOP+1);

	string Value;

	static const int FixedMultiplier[4] = {1, 10, 100, 1000};
	if (nParam == DEXED_OP_FREQ_COARSE)
	{
		if (!pUIMenu->m_pMiniDexed->GetVoiceParameter (DEXED_OP_OSC_MODE, nOP, nTG))
		{
			// Ratio
			if (!nValue)
			{
				Value = "0.50";
			}
			else
			{
				Value = to_string (nValue);
				Value += ".00";
			}
		}
		else
		{
			// Fixed
			Value = to_string (FixedMultiplier[nValue % 4]);
		}
	}
	else if (nParam == DEXED_OP_FREQ_FINE)
	{
		int nCoarse = pUIMenu->m_pMiniDexed->GetVoiceParameter (
							DEXED_OP_FREQ_COARSE, nOP, nTG);

		char Buffer[20];
		if (!pUIMenu->m_pMiniDexed->GetVoiceParameter (DEXED_OP_OSC_MODE, nOP, nTG))
		{
			// Ratio
			float fValue = 1.0f + nValue / 100.0f;
			fValue *= !nCoarse ? 0.5f : (float) nCoarse;
			sprintf (Buffer, "%.2f", (double) fValue);
		}
		else
		{
			// Fixed
			float fValue = powf (1.023293f, (float) nValue);
			fValue *= (float) FixedMultiplier[nCoarse % 4];
			sprintf (Buffer, "%.3fHz", (double) fValue);
		}

		Value = Buffer;
	}
	else
	{
		Value = GetOPValueString (nParam, nValue);
	}

	pUIMenu->m_pUI->DisplayWrite (OP.c_str (),
				      pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
				      Value.c_str (),
				      nValue > rParam.Minimum, nValue < rParam.Maximum);
}

void CUIMenu::SavePerformance (CUIMenu *pUIMenu, TMenuEvent Event)
{
	if (Event != MenuEventUpdate)
	{
		return;
	}

	bool bOK = pUIMenu->m_pMiniDexed->SavePerformance (pUIMenu->m_nCurrentParameter == 1);

	const char *pMenuName =
		pUIMenu->m_MenuStackParent[pUIMenu->m_nCurrentMenuDepth-1]
			[pUIMenu->m_nMenuStackItem[pUIMenu->m_nCurrentMenuDepth-1]].Name;

	pUIMenu->m_pUI->DisplayWrite (pMenuName,
				      pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
				      bOK ? "Completed" : "Error",
				      false, false);

	CTimer::Get ()->StartKernelTimer (MSEC2HZ (1500), TimerHandler, 0, pUIMenu);
}

string CUIMenu::GetGlobalValueString (unsigned nParameter, int nValue)
{
	string Result;

	assert (nParameter < sizeof CUIMenu::s_GlobalParameter / sizeof CUIMenu::s_GlobalParameter[0]);

	CUIMenu::TToString *pToString = CUIMenu::s_GlobalParameter[nParameter].ToString;
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

string CUIMenu::GetVoiceValueString (unsigned nVoiceParameter, int nValue)
{
	string Result;

	assert (nVoiceParameter < sizeof CUIMenu::s_VoiceParameter / sizeof CUIMenu::s_VoiceParameter[0]);

	CUIMenu::TToString *pToString = CUIMenu::s_VoiceParameter[nVoiceParameter].ToString;
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

string CUIMenu::GetOPValueString (unsigned nOPParameter, int nValue)
{
	string Result;

	assert (nOPParameter < sizeof CUIMenu::s_OPParameter / sizeof CUIMenu::s_OPParameter[0]);

	CUIMenu::TToString *pToString = CUIMenu::s_OPParameter[nOPParameter].ToString;
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

string CUIMenu::ToAlgorithm (int nValue)
{
	return to_string (nValue + 1);
}

string CUIMenu::ToOnOff (int nValue)
{
	static const char *OnOff[] = {"Off", "On"};

	assert ((unsigned) nValue < sizeof OnOff / sizeof OnOff[0]);

	return OnOff[nValue];
}

string CUIMenu::ToLFOWaveform (int nValue)
{
	static const char *Waveform[] = {"Triangle", "Saw down", "Saw up",
					 "Square", "Sine", "Sample/Hold"};

	assert ((unsigned) nValue < sizeof Waveform / sizeof Waveform[0]);

	return Waveform[nValue];
}

string CUIMenu::ToTransposeNote (int nValue)
{
	nValue += NoteC3 - 24;

	assert ((unsigned) nValue < sizeof s_NoteName / sizeof s_NoteName[0]);

	return s_NoteName[nValue];
}

string CUIMenu::ToBreakpointNote (int nValue)
{
	assert ((unsigned) nValue < sizeof s_NoteName / sizeof s_NoteName[0]);

	return s_NoteName[nValue];
}

string CUIMenu::ToKeyboardCurve (int nValue)
{
	static const char *Curve[] = {"-Lin", "-Exp", "+Exp", "+Lin"};

	assert ((unsigned) nValue < sizeof Curve / sizeof Curve[0]);

	return Curve[nValue];
}

string CUIMenu::ToOscillatorMode (int nValue)
{
	static const char *Mode[] = {"Ratio", "Fixed"};

	assert ((unsigned) nValue < sizeof Mode / sizeof Mode[0]);

	return Mode[nValue];
}

string CUIMenu::ToOscillatorDetune (int nValue)
{
	string Result;

	nValue -= 7;

	if (nValue > 0)
	{
		Result = "+" + to_string (nValue);
	}
	else
	{
		Result = to_string (nValue);
	}

	return Result;
}

string CUIMenu::ToPortaMode (int nValue)
{
	switch (nValue)
	{
	case 0:		return "Fingered";
	case 1:		return "Full time";
	default:	return to_string (nValue);
	}
};

string CUIMenu::ToPortaGlissando (int nValue)
{
	switch (nValue)
	{
	case 0:		return "Off";
	case 1:		return "On";
	default:	return to_string (nValue);
	}
};

string CUIMenu::ToPolyMono (int nValue)
{
	switch (nValue)
	{
	case 0:		return "Poly";
	case 1:		return "Mono";
	default:	return to_string (nValue);
	}
}

void CUIMenu::TGShortcutHandler (TMenuEvent Event)
{
	assert (m_nCurrentMenuDepth >= 2);
	assert (m_MenuStackMenu[0] = s_MainMenu);
	unsigned nTG = m_nMenuStackSelection[0];
	assert (nTG < CConfig::ToneGenerators);
	assert (m_nMenuStackItem[1] == nTG);
	assert (m_nMenuStackParameter[1] == nTG);

	assert (   Event == MenuEventPressAndStepDown
		|| Event == MenuEventPressAndStepUp);
	if (Event == MenuEventPressAndStepDown)
	{
		nTG--;
	}
	else
	{
		nTG++;
	}

	if (nTG < CConfig::ToneGenerators)
	{
		m_nMenuStackSelection[0] = nTG;
		m_nMenuStackItem[1] = nTG;
		m_nMenuStackParameter[1] = nTG;

		EventHandler (MenuEventUpdate);
	}
}

void CUIMenu::OPShortcutHandler (TMenuEvent Event)
{
	assert (m_nCurrentMenuDepth >= 3);
	assert (m_MenuStackMenu[m_nCurrentMenuDepth-2] = s_EditVoiceMenu);
	unsigned nOP = m_nMenuStackSelection[m_nCurrentMenuDepth-2];
	assert (nOP < 6);
	assert (m_nMenuStackItem[m_nCurrentMenuDepth-1] == nOP);
	assert (m_nMenuStackParameter[m_nCurrentMenuDepth-1] == nOP);

	assert (   Event == MenuEventPressAndStepDown
		|| Event == MenuEventPressAndStepUp);
	if (Event == MenuEventPressAndStepDown)
	{
		nOP--;
	}
	else
	{
		nOP++;
	}

	if (nOP < 6)
	{
		m_nMenuStackSelection[m_nCurrentMenuDepth-2] = nOP;
		m_nMenuStackItem[m_nCurrentMenuDepth-1] = nOP;
		m_nMenuStackParameter[m_nCurrentMenuDepth-1] = nOP;

		EventHandler (MenuEventUpdate);
	}
}

void CUIMenu::TimerHandler (TKernelTimerHandle hTimer, void *pParam, void *pContext)
{
	CUIMenu *pThis = static_cast<CUIMenu *> (pContext);
	assert (pThis);

	pThis->EventHandler (MenuEventBack);
}

void CUIMenu::TimerHandlerNoBack (TKernelTimerHandle hTimer, void *pParam, void *pContext)
{
	CUIMenu *pThis = static_cast<CUIMenu *> (pContext);
	assert (pThis);
	
	pThis->m_bSplashShow = false;
	
	pThis->EventHandler (MenuEventUpdate);
}

void CUIMenu::PerformanceMenu (CUIMenu *pUIMenu, TMenuEvent Event)
{
	bool bPerformanceSelectToLoad = pUIMenu->m_pMiniDexed->GetPerformanceSelectToLoad();
	unsigned nValue = pUIMenu->m_nSelectedPerformanceID;
	std::string Value;
		
	if (Event == MenuEventUpdate)
	{
		pUIMenu->m_bPerformanceDeleteMode=false;
	}
	
	if (pUIMenu->m_bSplashShow)
	{
		return;
	}		
	
	if(!pUIMenu->m_bPerformanceDeleteMode)
	{		
		switch (Event)
		{
		case MenuEventUpdate:
			break;

		case MenuEventStepDown:
			if (nValue > 0)
			{
				--nValue;
			}
			pUIMenu->m_nSelectedPerformanceID = nValue;
			if (!bPerformanceSelectToLoad && pUIMenu->m_nCurrentParameter==0)
			{
				pUIMenu->m_pMiniDexed->SetNewPerformance(nValue);
			}
			break;

		case MenuEventStepUp:
			if (++nValue > (unsigned) pUIMenu->m_pMiniDexed->GetLastPerformance()-1)
			{
				nValue = pUIMenu->m_pMiniDexed->GetLastPerformance()-1;
			}
			pUIMenu->m_nSelectedPerformanceID = nValue;
			if (!bPerformanceSelectToLoad && pUIMenu->m_nCurrentParameter==0)
			{
				pUIMenu->m_pMiniDexed->SetNewPerformance(nValue);
			}
			break;

		case MenuEventSelect:	
			switch (pUIMenu->m_nCurrentParameter)
			{
			case 0:
				if (bPerformanceSelectToLoad)
				{
				pUIMenu->m_pMiniDexed->SetNewPerformance(nValue);
				}

				break;
			case 1:
				if (pUIMenu->m_nSelectedPerformanceID != 0)
				{
					pUIMenu->m_bPerformanceDeleteMode=true;
					pUIMenu->m_bConfirmDeletePerformance=false;
				}
				break;
			default:
				break;
			}
			break;
		default:
			return;
		}
	}
	else
	{
		switch (Event)
		{
		case MenuEventUpdate:
			break;

		case MenuEventStepDown:
			pUIMenu->m_bConfirmDeletePerformance=false;
			break;

		case MenuEventStepUp:
			pUIMenu->m_bConfirmDeletePerformance=true;
			break;

		case MenuEventSelect:	
			pUIMenu->m_bPerformanceDeleteMode=false;
			if (pUIMenu->m_bConfirmDeletePerformance)
			{
				pUIMenu->m_nSelectedPerformanceID = 0;
				pUIMenu->m_bConfirmDeletePerformance=false;
				pUIMenu->m_pUI->DisplayWrite ("", "Delete", pUIMenu->m_pMiniDexed->DeletePerformance(nValue) ? "Completed" : "Error", false, false);
				pUIMenu->m_bSplashShow=true;
				CTimer::Get ()->StartKernelTimer (MSEC2HZ (1500), TimerHandlerNoBack, 0, pUIMenu);
				return;
			}
			else
			{
				break;
			}
			
		default:
			return;
		}		
	}
		
	if(!pUIMenu->m_bPerformanceDeleteMode)
	{
		Value = pUIMenu->m_pMiniDexed->GetPerformanceName(nValue);
		

		std::string nPSelected = "";
		if(nValue == pUIMenu->m_pMiniDexed->GetActualPerformanceID())
		{
			nPSelected= "[L]";
		}
					
		pUIMenu->m_pUI->DisplayWrite (pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name, nPSelected.c_str(),
						  Value.c_str (),
						 (int) nValue > 0, (int) nValue < (int) pUIMenu->m_pMiniDexed->GetLastPerformance()-1);
	}
	else
	{
		pUIMenu->m_pUI->DisplayWrite ("", "Delete?", pUIMenu->m_bConfirmDeletePerformance ? "Yes" : "No", false, false);
	}
}

void CUIMenu::InputTxt (CUIMenu *pUIMenu, TMenuEvent Event)
{
	unsigned nTG=0;
	string TG ("TG");
	
	std::string MsgOk;
	std::string NoValidChars;
	unsigned MaxChars;
	std::string MenuTitleR;
	std::string MenuTitleL;
	std::string OkTitleL;
	std::string OkTitleR;
	
	switch(pUIMenu->m_nCurrentParameter)
	{
		case 1: // save new performance
			NoValidChars = {92, 47, 58, 42, 63, 34, 60,62, 124};
			MaxChars=14;
			MenuTitleL="Performance Name";
			MenuTitleR="";
			OkTitleL="New Performance"; // \E[?25l
			OkTitleR="";
		 break;
		 
		case 2: // Rename performance - NOT Implemented yet
			NoValidChars = {92, 47, 58, 42, 63, 34, 60,62, 124};
			MaxChars=14;
			MenuTitleL="Performance Name";
			MenuTitleR="";
			OkTitleL="Rename Perf."; // \E[?25l
			OkTitleR="";
		break;
		
		case 3: // Voice name
			nTG = pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth-2];
			NoValidChars = {127};
			MaxChars=10;
			MenuTitleL="Name";
			TG += to_string (nTG+1);
			MenuTitleR=TG;
			OkTitleL="";
			OkTitleR="";
		break;
			
		default:
		return;
	}
	
	bool bOK;
	unsigned nPosition = pUIMenu->m_InputTextPosition;
	unsigned nChar = pUIMenu->m_InputText[nPosition];

	
	switch (Event)
	{
	case MenuEventUpdate:
		if(pUIMenu->m_nCurrentParameter == 1 || pUIMenu->m_nCurrentParameter == 2)
		{
			pUIMenu->m_InputText = pUIMenu->m_pMiniDexed->GetNewPerformanceDefaultName();
			pUIMenu->m_InputText += "              ";
			pUIMenu->m_InputText =  pUIMenu->m_InputText.substr(0,14);
			pUIMenu->m_InputTextPosition=0;
			nPosition=pUIMenu->m_InputTextPosition;
			nChar = pUIMenu->m_InputText[nPosition];
		}
		else
		{
			
			pUIMenu->m_InputText = pUIMenu->m_pMiniDexed->GetVoiceName(nTG);
			pUIMenu->m_InputText += "          ";
			pUIMenu->m_InputText =  pUIMenu->m_InputText.substr(0,10);
			pUIMenu->m_InputTextPosition=0;
			nPosition=pUIMenu->m_InputTextPosition;
			nChar = pUIMenu->m_InputText[nPosition];
		}
		break;

	case MenuEventStepDown:
		if (nChar > 32)
		{
		do	{
			--nChar;
			}
		while (NoValidChars.find(nChar) != std::string::npos);
		}
		pUIMenu->m_InputTextChar = nChar;
		break;

	case MenuEventStepUp:
		if (nChar < 126)
		{
		do	{
				++nChar;
			}
		while (NoValidChars.find(nChar) != std::string::npos);			
		}
		pUIMenu->m_InputTextChar = nChar;
		break;	
		
		
		
	case MenuEventSelect:	
		if(pUIMenu->m_nCurrentParameter == 1)
		{	
			pUIMenu->m_pMiniDexed->SetNewPerformanceName(pUIMenu->m_InputText);
			bOK = pUIMenu->m_pMiniDexed->SavePerformanceNewFile ();
			MsgOk=bOK ? "Completed" : "Error";
			pUIMenu->m_pUI->DisplayWrite (OkTitleR.c_str(), OkTitleL.c_str(), MsgOk.c_str(), false, false);
			CTimer::Get ()->StartKernelTimer (MSEC2HZ (1500), TimerHandler, 0, pUIMenu);
			return;
		}
		else
		{
			break; // Voice Name Edit
		}
	
	case MenuEventPressAndStepDown:
		if (nPosition > 0)
			{
				--nPosition;
			}
		pUIMenu->m_InputTextPosition = nPosition;
		nChar = pUIMenu->m_InputText[nPosition];
		break;
	
	case MenuEventPressAndStepUp:
		if (nPosition < MaxChars-1)
		{
			++nPosition;
		}
		pUIMenu->m_InputTextPosition = nPosition;
		nChar = pUIMenu->m_InputText[nPosition];
		break;

	default:
		return;
	}
	
	
	// \E[2;%dH	Cursor move to row %1 and column %2 (starting at 1)
	// \E[?25h	Normal cursor visible
	// \E[?25l	Cursor invisible
	
	std::string escCursor="\E[?25h\E[2;"; // this is to locate cursor
	escCursor += to_string(nPosition + 2);
	escCursor += "H";
	

	std::string Value = pUIMenu->m_InputText;
	Value[nPosition]=nChar;
	pUIMenu->m_InputText = Value;
	
	if(pUIMenu->m_nCurrentParameter == 3)
		{
			pUIMenu->m_pMiniDexed->SetVoiceName(pUIMenu->m_InputText, nTG);
		}	
		
	Value = Value + " " + escCursor ;
	pUIMenu->m_pUI->DisplayWrite (MenuTitleR.c_str(),MenuTitleL.c_str(), Value.c_str(), false, false);
	
	
}

void CUIMenu::EditTGParameterModulation (CUIMenu *pUIMenu, TMenuEvent Event) 
{

	unsigned nTG = pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth-3]; 
	unsigned nController = pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth-1]; 
	unsigned nParameter = pUIMenu->m_nCurrentParameter + nController;
	
	CMiniDexed::TTGParameter Param = (CMiniDexed::TTGParameter) nParameter;
	const TParameter &rParam = s_TGParameter[Param];

	int nValue = pUIMenu->m_pMiniDexed->GetTGParameter (Param, nTG);

	switch (Event)
	{
	case MenuEventUpdate:
		break;

	case MenuEventStepDown:
		nValue -= rParam.Increment;
		if (nValue < rParam.Minimum)
		{
			nValue = rParam.Minimum;
		}
		pUIMenu->m_pMiniDexed->SetTGParameter (Param, nValue, nTG);
		break;

	case MenuEventStepUp:
		nValue += rParam.Increment;
		if (nValue > rParam.Maximum)
		{
			nValue = rParam.Maximum;
		}
		pUIMenu->m_pMiniDexed->SetTGParameter (Param, nValue, nTG);
		break;

	case MenuEventPressAndStepDown:
	case MenuEventPressAndStepUp:
		pUIMenu->TGShortcutHandler (Event);
		return;

	default:
		return;
	}

	string TG ("TG");
	TG += to_string (nTG+1);

	string Value = GetTGValueString (Param, pUIMenu->m_pMiniDexed->GetTGParameter (Param, nTG));

	pUIMenu->m_pUI->DisplayWrite (TG.c_str (),
				      pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
				      Value.c_str (),
				      nValue > rParam.Minimum, nValue < rParam.Maximum);
				   
}


