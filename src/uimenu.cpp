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
LOGMODULE ("uimenu");

const CUIMenu::TMenuItem CUIMenu::s_MenuRoot[] =
{
	{"MiniDexed", MainMenuHandler, s_MainMenu},
	{0}
};

// inserting menu items before "TG1" affect TGShortcutHandler()
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
#if (RASPPI==4 || RASPPI==5)
	{"TG9",		MenuHandler,	s_TGMenu, 8},
	{"TG10",	MenuHandler,	s_TGMenu, 9},
	{"TG11",	MenuHandler,	s_TGMenu, 10},
	{"TG12",	MenuHandler,	s_TGMenu, 11},
	{"TG13",	MenuHandler,	s_TGMenu, 12},
	{"TG14",	MenuHandler,	s_TGMenu, 13},
	{"TG15",	MenuHandler,	s_TGMenu, 14},
	{"TG16",	MenuHandler,	s_TGMenu, 15},
#endif
#endif
	{"Effects",	MenuHandler,	s_EffectsMenu},
	{"Tempo", EditGlobalParameter, 0, CMiniDexed::ParameterTempo},
	{"Performance",	MenuHandler, s_PerformanceMenu}, 
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
    {"Insert FX", MenuHandlerInsertFX},
	{"Send FX 1",	EditTGParameter,	0,	CMiniDexed::TGParameterSendFX1},
	{"Send FX 2",	EditTGParameter,	0,	CMiniDexed::TGParameterReverbSend},
	{"Detune",	EditTGParameter,	0,	CMiniDexed::TGParameterMasterTune},
	{"Cutoff",	EditTGParameter,	0,	CMiniDexed::TGParameterCutoff},
	{"Resonance",	EditTGParameter,	0,	CMiniDexed::TGParameterResonance},
	{"Pitch Bend",	MenuHandler,		s_EditPitchBendMenu},
	{"Portamento",		MenuHandler,		s_EditPortamentoMenu},
	{"Poly/Mono",		EditTGParameter,	0,	CMiniDexed::TGParameterMonoMode}, 
	{"Modulation",		MenuHandler,		s_ModulationMenu},
	{"Midi FX",	MenuHandlerMidiFX},
	{"Channel",	EditTGParameter,	0,	CMiniDexed::TGParameterMIDIChannel},
	{"Edit Voice",	MenuHandler,		s_EditVoiceMenu},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_EffectsMenu[] =
{
	{"Compress", EditGlobalParameter, 0, CMiniDexed::ParameterCompressorEnable},
	{"Send FX 1", MenuHandlerSendFX1},
	{"Send FX 2", MenuHandlerSendFX2},
	{"Master FX", MenuHandlerMasterFX},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_EditPitchBendMenu[] =
{
	{"Bend Range",	EditTGParameter2,	0,	CMiniDexed::TGParameterPitchBendRange},
	{"Bend Step",		EditTGParameter2,	0,	CMiniDexed::TGParameterPitchBendStep},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_EditPortamentoMenu[] =
{
	{"Mode",		EditTGParameter2,	0,	CMiniDexed::TGParameterPortamentoMode},
	{"Glissando",		EditTGParameter2,	0,	CMiniDexed::TGParameterPortamentoGlissando},
	{"Time",		EditTGParameter2,	0,	CMiniDexed::TGParameterPortamentoTime},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_ModulationMenu[] =
{
	{"Mod. Wheel",		MenuHandler,	s_ModulationMenuParameters,	CMiniDexed::TGParameterMWRange},
	{"Foot Control",	MenuHandler,	s_ModulationMenuParameters,	CMiniDexed::TGParameterFCRange},
	{"Breath Control",	MenuHandler,	s_ModulationMenuParameters,	CMiniDexed::TGParameterBCRange},
	{"Aftertouch",	MenuHandler,	s_ModulationMenuParameters,	CMiniDexed::TGParameterATRange},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_ModulationMenuParameters[] =
{
	{"Range",		EditTGParameterModulation,	0, 0},
	{"Pitch",		EditTGParameterModulation,	0, 1},
	{"Amplitude",	EditTGParameterModulation,	0, 2},
	{"EG Bias",		EditTGParameterModulation,	0, 3},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_SendFX1Menu[] =
{
	{"Type:", EditGlobalParameter, 0, CMiniDexed::ParameterSendFX1Type},
	{"Edit:", EditSendFX1},
	{"To FX 2", EditGlobalParameter, 0, CMiniDexed::ParameterSendFX1SendFXLevel},
	{"Level", EditGlobalParameter, 0, CMiniDexed::ParameterSendFX1Level},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_SendFX2Menu[] =
{
	{"Type:", EditGlobalParameter, 0, CMiniDexed::ParameterSendFX2Type},
	{"Edit:", EditSendFX2},
	{"Level", EditGlobalParameter, 0, CMiniDexed::ParameterSendFX2Level},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_MasterFXMenu[] =
{
	{"Type:", EditGlobalParameter, 0, CMiniDexed::ParameterMasterFXType},
	{"Edit:", EditMasterFX},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_InsertFX[] =
{
	{"Type:", EditTGParameter2, 0, CMiniDexed::TGParameterInsertFXType},
	{"Edit:", EditInsertFX},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_FXDisabled[] =
{
	{"Disabled"},
	{0}
};

CUIMenu::TMenuItem CUIMenu::s_FXNone[] =
{
	{"None"},
	{0}
};

CUIMenu::TMenuItem CUIMenu::s_FXChorus[] =
{
	{"Bypass", EditTGFXParameter, 0, AudioEffectChorus::Param::BYPASS},
	{"Chorus I", EditTGFXParameter, 0, AudioEffectChorus::Param::CHORUS_I_ENABLE},
	{"Chorus II", EditTGFXParameter, 0, AudioEffectChorus::Param::CHORUS_II_ENABLE},
	{"Rate I", EditTGFXParameter, 0, AudioEffectChorus::Param::CHORUS_I_RATE},
	{"Rate II", EditTGFXParameter, 0, AudioEffectChorus::Param::CHORUS_II_RATE},
	{0}
};

CUIMenu::TMenuItem CUIMenu::s_FXDelay[] =
{
	{"Bypass", EditTGFXParameter, 0, AudioEffectDelay::Param::BYPASS},
	{"Time L", EditTGFXParameter, 0, AudioEffectDelay::Param::TIME_L},
	{"Time R", EditTGFXParameter, 0, AudioEffectDelay::Param::TIME_R},
	{"Feedback", EditTGFXParameter, 0, AudioEffectDelay::Param::FEEDBACK},
	{"Tone", EditTGFXParameter, 0, AudioEffectDelay::Param::TONE},
	{"Ping Pong", EditTGFXParameter, 0, AudioEffectDelay::Param::PING_PONG},
	{"Mix", EditTGFXParameter, 0, AudioEffectDelay::Param::MIX},
	{0}
};

CUIMenu::TMenuItem CUIMenu::s_FXLPFilter[] =
{
	{"Bypass", EditTGFXParameter, 0, AudioEffectLPF::Param::BYPASS},
	{"Cutoff", EditTGFXParameter, 0, AudioEffectLPF::Param::CUTOFF},
	{"Resonance", EditTGFXParameter, 0, AudioEffectLPF::Param::RESONANCE},
	{0}
};

CUIMenu::TMenuItem CUIMenu::s_FXDS1[] =
{
	{"Bypass", EditTGFXParameter, 0, AudioEffectDS1::Param::BYPASS},
	{"Dist", EditTGFXParameter, 0, AudioEffectDS1::Param::DIST},
	{"Tone", EditTGFXParameter, 0, AudioEffectDS1::Param::TONE},
	{"Level", EditTGFXParameter, 0, AudioEffectDS1::Param::LEVEL},
	{0}
};

CUIMenu::TMenuItem CUIMenu::s_FXBigMuff[] =
{
	{"Bypass", EditTGFXParameter, 0, AudioEffectBigMuff::Param::BYPASS},
	{"Sustain", EditTGFXParameter, 0, AudioEffectBigMuff::Param::SUSTAIN},
	{"Tone", EditTGFXParameter, 0, AudioEffectBigMuff::Param::TONE},
	{"Level", EditTGFXParameter, 0, AudioEffectBigMuff::Param::LEVEL},
	{0}
};

CUIMenu::TMenuItem CUIMenu::s_FXTalReverb3[] =
{
	{"Bypass", EditTGFXParameter, 0, AudioEffectTalReverb3::Param::BYPASS},
	{"Dry", EditTGFXParameter, 0, AudioEffectTalReverb3::Param::DRY},
	{"Wet", EditTGFXParameter, 0, AudioEffectTalReverb3::Param::WET},
	{"Decay Time", EditTGFXParameter, 0, AudioEffectTalReverb3::Param::DECAYTIME},
	{"Predelay", EditTGFXParameter, 0, AudioEffectTalReverb3::Param::PREDELAY},
	{"Low Shelf", EditTGFXParameter, 0, AudioEffectTalReverb3::Param::LOWSHELFGAIN},
	{"High Shelf", EditTGFXParameter, 0, AudioEffectTalReverb3::Param::HIGHSHELFGAIN},
	{"Stereo Width", EditTGFXParameter, 0, AudioEffectTalReverb3::Param::STEREO},
	{"Stereo Mode", EditTGFXParameter, 0, AudioEffectTalReverb3::Param::REALSTEREOMODE},
	{"Power", EditTGFXParameter, 0, AudioEffectTalReverb3::Param::POWER},
	{0}
};

CUIMenu::TMenuItem CUIMenu::s_FXReverb[] =
{
	{"Bypass", EditTGFXParameter, 0, AudioEffectPlateReverb::Param::BYPASS},
	{"Size", EditTGFXParameter, 0, AudioEffectPlateReverb::Param::SIZE},
	{"High damp", EditTGFXParameter, 0, AudioEffectPlateReverb::Param::HIGH_DAMP},
	{"Low damp", EditTGFXParameter, 0, AudioEffectPlateReverb::Param::LOW_DAMP},
	{"Low pass", EditTGFXParameter, 0, AudioEffectPlateReverb::Param::LOW_PASS},
	{"Diffusion", EditTGFXParameter, 0, AudioEffectPlateReverb::Param::DIFFUSION},
	{"Mix", EditTGFXParameter, 0, AudioEffectPlateReverb::Param::MIX},
	{"Level", EditTGFXParameter, 0, AudioEffectPlateReverb::Param::LEVEL},
	{0}
};

CUIMenu::TMenuItem CUIMenu::s_FXMVerb[] =
{
	{"Bypass", EditTGFXParameter, 0, AudioEffectMVerb::Param::BYPASS},
	{"Damp Fq", EditTGFXParameter, 0, AudioEffectMVerb::Param::DAMPINGFREQ},
	{"Density", EditTGFXParameter, 0, AudioEffectMVerb::Param::DENSITY},
	{"Band Fq", EditTGFXParameter, 0, AudioEffectMVerb::Param::BANDWIDTHFREQ},
	{"Decay", EditTGFXParameter, 0, AudioEffectMVerb::Param::DECAY},
	{"Predelay", EditTGFXParameter, 0, AudioEffectMVerb::Param::PREDELAY},
	{"Size", EditTGFXParameter, 0, AudioEffectMVerb::Param::SIZE},
	{"Gain", EditTGFXParameter, 0, AudioEffectMVerb::Param::GAIN},
	{"Mix", EditTGFXParameter, 0, AudioEffectMVerb::Param::MIX},
	{"Early Mix", EditTGFXParameter, 0, AudioEffectMVerb::Param::EARLYMIX},
	{0}
};

CUIMenu::TMenuItem CUIMenu::s_FX3BandEQ[] =
{
	{"Bypass", EditTGFXParameter, 0, AudioEffect3BandEQ::Param::BYPASS},
	{"Low", EditTGFXParameter, 0, AudioEffect3BandEQ::Param::EQ_LOW},
	{"Mid", EditTGFXParameter, 0, AudioEffect3BandEQ::Param::EQ_MID},
	{"High", EditTGFXParameter, 0, AudioEffect3BandEQ::Param::EQ_HIGH},
	{"Master", EditTGFXParameter, 0, AudioEffect3BandEQ::Param::MASTER},
	{"Low Mid FQ", EditTGFXParameter, 0, AudioEffect3BandEQ::Param::LOW_MID_FQ},
	{"Mid High FQ", EditTGFXParameter, 0, AudioEffect3BandEQ::Param::MID_HIGH_FQ},
	{0}
};

CUIMenu::TMenuItem CUIMenu::s_FXPhaser[] =
{
	{"Bypass", EditTGFXParameter, 0, AudioEffectPhaser::Param::BYPASS},
	{"Mix", EditTGFXParameter, 0, AudioEffectPhaser::Param::WETDRY},
	{"Pan", EditTGFXParameter, 0, AudioEffectPhaser::Param::PAN},
	{"Freq", EditTGFXParameter, 0, AudioEffectPhaser::Param::PH_FREQ},
	{"Random", EditTGFXParameter, 0, AudioEffectPhaser::Param::PH_RND},
	{"Type", EditTGFXParameter, 0, AudioEffectPhaser::Param::TYPE},
	{"Stereo", EditTGFXParameter, 0, AudioEffectPhaser::Param::STDL},
	{"Depth", EditTGFXParameter, 0, AudioEffectPhaser::Param::PH_DEPTH},
	{"Feedback", EditTGFXParameter, 0, AudioEffectPhaser::Param::FB},
	{"Stages", EditTGFXParameter, 0, AudioEffectPhaser::Param::STAGES},
	{"L/R Cross", EditTGFXParameter, 0, AudioEffectPhaser::Param::LRCR},
	{"Sub", EditTGFXParameter, 0, AudioEffectPhaser::Param::SUB},
	{"Phase", EditTGFXParameter, 0, AudioEffectPhaser::Param::PHASE},
	{0}
};

CUIMenu::TMenuItem CUIMenu::s_FXAPhaser[] =
{
	{"Bypass", EditTGFXParameter, 0, AudioEffectAPhaser::Param::BYPASS},
	{"Mix", EditTGFXParameter, 0, AudioEffectAPhaser::Param::WETDRY},
	{"Distortion", EditTGFXParameter, 0, AudioEffectAPhaser::Param::DISTORTION},
	{"Freq", EditTGFXParameter, 0, AudioEffectAPhaser::Param::PH_FREQ},
	{"Random", EditTGFXParameter, 0, AudioEffectAPhaser::Param::PH_RND},
	{"Type", EditTGFXParameter, 0, AudioEffectAPhaser::Param::TYPE},
	{"Stereo", EditTGFXParameter, 0, AudioEffectAPhaser::Param::STDL},
	{"Width", EditTGFXParameter, 0, AudioEffectAPhaser::Param::WIDTH},
	{"Feedback", EditTGFXParameter, 0, AudioEffectAPhaser::Param::FB},
	{"Stages", EditTGFXParameter, 0, AudioEffectAPhaser::Param::STAGES},
	{"Offset", EditTGFXParameter, 0, AudioEffectAPhaser::Param::OFFSET},
	{"Sub", EditTGFXParameter, 0, AudioEffectAPhaser::Param::SUB},
	{"Depth", EditTGFXParameter, 0, AudioEffectAPhaser::Param::PH_DEPTH},
	{"Hyper", EditTGFXParameter, 0, AudioEffectAPhaser::Param::HYPER},
	{0}
};

CUIMenu::TMenuItem CUIMenu::s_FXFlanger[] =
{
	{"Bypass", EditTGFXParameter, 0, AudioEffectFlanger::Param::BYPASS},
	{"Mix", EditTGFXParameter, 0, AudioEffectFlanger::Param::MIX},
	{"Pan", EditTGFXParameter, 0, AudioEffectFlanger::Param::PAN},
	{"Freq", EditTGFXParameter, 0, AudioEffectFlanger::Param::FL_FREQ},
	{"Random", EditTGFXParameter, 0, AudioEffectFlanger::Param::FL_RND},
	{"Type", EditTGFXParameter, 0, AudioEffectFlanger::Param::TYPE},
	{"Stereo", EditTGFXParameter, 0, AudioEffectFlanger::Param::STDL},
	{"Depth", EditTGFXParameter, 0, AudioEffectFlanger::Param::FL_DEPTH},
	{"Delay", EditTGFXParameter, 0, AudioEffectFlanger::Param::DELAY},
	{"Feedback", EditTGFXParameter, 0, AudioEffectFlanger::Param::FB},
	{"L/R Cross", EditTGFXParameter, 0, AudioEffectFlanger::Param::LRCR},
	{"Mode", EditTGFXParameter, 0, AudioEffectFlanger::Param::MODE},
	{"Sub", EditTGFXParameter, 0, AudioEffectFlanger::Param::SUB},
	{"Awesome", EditTGFXParameter, 0, AudioEffectFlanger::Param::AWESOME},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_MidiFX[] =
{
	{"Type:", EditTGParameter2, 0, CMiniDexed::TGParameterMidiFXType},
	{"Edit:", EditMidiFX},
	{0}
};

CUIMenu::TMenuItem CUIMenu::s_MidiFXNone[] =
{
	{"None"},
	{0}
};

CUIMenu::TMenuItem CUIMenu::s_MidiFXArp[] =
{
	{"Bypass", EditTGMidiFXParameter, 0, MidiArp::Param::BYPASS},
	{"Latch", EditTGMidiFXParameter, 0, MidiArp::Param::LATCH},
	{"Sync", EditTGMidiFXParameter, 0, MidiArp::Param::SYNC},
	{"Arp Mode", EditTGMidiFXParameter, 0, MidiArp::Param::ARP_MODE},
	{"Division", EditTGMidiFXParameter, 0, MidiArp::Param::DIVISION},
	{"Note Length", EditTGMidiFXParameter, 0, MidiArp::Param::NOTE_LENGTH},
	{"Velocity", EditTGMidiFXParameter, 0, MidiArp::Param::VELOCITY},
	{"Oct Spread", EditTGMidiFXParameter, 0, MidiArp::Param::OCTAVE_SPREAD},
	{"Oct Mode", EditTGMidiFXParameter, 0, MidiArp::Param::OCTAVE_MODE},
	{"Panic", EditTGMidiFXParameter, 0, MidiArp::Param::PANIC},
	{0}
};

// inserting menu items before "OP1" affect OPShortcutHandler()
const CUIMenu::TMenuItem CUIMenu::s_EditVoiceMenu[] =
{
	{"OP1",		MenuHandler,		s_OperatorMenu, 0},
	{"OP2",		MenuHandler,		s_OperatorMenu, 1},
	{"OP3",		MenuHandler,		s_OperatorMenu, 2},
	{"OP4",		MenuHandler,		s_OperatorMenu, 3},
	{"OP5",		MenuHandler,		s_OperatorMenu, 4},
	{"OP6",		MenuHandler,		s_OperatorMenu, 5},
	{"Algorithm",	EditVoiceParameter,	0,		DEXED_ALGORITHM},
	{"Feedback",	EditVoiceParameter,	0,		DEXED_FEEDBACK},
	{"P EG Rate 1",	EditVoiceParameter,	0,		DEXED_PITCH_EG_R1},
	{"P EG Rate 2",	EditVoiceParameter,	0,		DEXED_PITCH_EG_R2},
	{"P EG Rate 3",	EditVoiceParameter,	0,		DEXED_PITCH_EG_R3},
	{"P EG Rate 4",	EditVoiceParameter,	0,		DEXED_PITCH_EG_R4},
	{"P EG Level 1",EditVoiceParameter,	0,		DEXED_PITCH_EG_L1},
	{"P EG Level 2",EditVoiceParameter,	0,		DEXED_PITCH_EG_L2},
	{"P EG Level 3",EditVoiceParameter,	0,		DEXED_PITCH_EG_L3},
	{"P EG Level 4",EditVoiceParameter,	0,		DEXED_PITCH_EG_L4},
	{"Osc Key Sync",EditVoiceParameter,	0,		DEXED_OSC_KEY_SYNC},
	{"LFO Speed",	EditVoiceParameter,	0,		DEXED_LFO_SPEED},
	{"LFO Delay",	EditVoiceParameter,	0,		DEXED_LFO_DELAY},
	{"LFO PMD",	EditVoiceParameter,	0,		DEXED_LFO_PITCH_MOD_DEP},
	{"LFO AMD",	EditVoiceParameter,	0,		DEXED_LFO_AMP_MOD_DEP},
	{"LFO Sync",	EditVoiceParameter,	0,		DEXED_LFO_SYNC},
	{"LFO Wave",	EditVoiceParameter,	0,		DEXED_LFO_WAVE},
	{"P Mod Sens.",	EditVoiceParameter,	0,		DEXED_LFO_PITCH_MOD_SENS},
	{"Transpose",	EditVoiceParameter,	0,		DEXED_TRANSPOSE},
	{"Name",	InputTxt,0 , 3}, 
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_OperatorMenu[] =
{
	{"Output Level",EditOPParameter,	0,	DEXED_OP_OUTPUT_LEV},
	{"Freq Coarse",	EditOPParameter,	0,	DEXED_OP_FREQ_COARSE},
	{"Freq Fine",	EditOPParameter,	0,	DEXED_OP_FREQ_FINE},
	{"Osc Detune",	EditOPParameter,	0,	DEXED_OP_OSC_DETUNE},
	{"Osc Mode",	EditOPParameter,	0,	DEXED_OP_OSC_MODE},
	{"EG Rate 1",	EditOPParameter,	0,	DEXED_OP_EG_R1},
	{"EG Rate 2",	EditOPParameter,	0,	DEXED_OP_EG_R2},
	{"EG Rate 3",	EditOPParameter,	0,	DEXED_OP_EG_R3},
	{"EG Rate 4",	EditOPParameter,	0,	DEXED_OP_EG_R4},
	{"EG Level 1",	EditOPParameter,	0,	DEXED_OP_EG_L1},
	{"EG Level 2",	EditOPParameter,	0,	DEXED_OP_EG_L2},
	{"EG Level 3",	EditOPParameter,	0,	DEXED_OP_EG_L3},
	{"EG Level 4",	EditOPParameter,	0,	DEXED_OP_EG_L4},
	{"Break Point",	EditOPParameter,	0,	DEXED_OP_LEV_SCL_BRK_PT},
	{"L Key Depth",	EditOPParameter,	0,	DEXED_OP_SCL_LEFT_DEPTH},
	{"R Key Depth",	EditOPParameter,	0,	DEXED_OP_SCL_RGHT_DEPTH},
	{"L Key Scale",	EditOPParameter,	0,	DEXED_OP_SCL_LEFT_CURVE},
	{"R Key Scale",	EditOPParameter,	0,	DEXED_OP_SCL_RGHT_CURVE},
	{"Rate Scaling",EditOPParameter,	0,	DEXED_OP_OSC_RATE_SCALE},
	{"A Mod Sens.",	EditOPParameter,	0,	DEXED_OP_AMP_MOD_SENS},
	{"K Vel. Sens.",EditOPParameter,	0,	DEXED_OP_KEY_VEL_SENS},
	{"Enable", EditOPParameter, 0, DEXED_OP_ENABLE},
	{0}
};

const CUIMenu::TMenuItem CUIMenu::s_SaveMenu[] =
{
	{"Overwrite",	SavePerformance, 0, 0}, 
	{"New",	InputTxt,0 , 1}, 
	{"Save as default",	SavePerformance, 0, 1}, 
	{0}
};

// must match CMiniDexed::TParameter
const CUIMenu::TParameter CUIMenu::s_GlobalParameter[CMiniDexed::ParameterUnknown] =
{
	{0,	1,	1,	ToOnOff},		// ParameterCompressorEnable
	{0,	AudioEffects::Types::UNKNOWN - 1, 1, ToFXType}, // ParameterSendFX1Type
	{0,	99, 1}, // ParameterSendFX1SendFXLevel
	{0,	100, 1}, // ParameterSendFX2Level
	{0,	AudioEffects::Types::UNKNOWN - 1, 1, ToFXType}, // ParameterSendFX2Type
	{0,	100, 1}, // ParameterSendFX2Level
	{0,	AudioEffects::Types::UNKNOWN - 1, 1, ToFXType}, // ParameterMasterFXType
	{0,	CMIDIDevice::ChannelUnknown-1,		1, ToMIDIChannel}, 	// ParameterPerformanceSelectChannel
	{0, NUM_PERFORMANCE_BANKS, 1},	// ParameterPerformanceBank
	{30, 250, 1} // ParameterTempo
};

// must match CMiniDexed::TTGParameter
const CUIMenu::TParameter CUIMenu::s_TGParameter[CMiniDexed::TGParameterUnknown] =
{
	{0,	CSysExFileLoader::MaxVoiceBankID,	1},			// TGParameterVoiceBank
	{0, 0, 0},											// TGParameterVoiceBankMSB (not used in menus)
	{0, 0, 0},											// TGParameterVoiceBankLSB (not used in menus)
	{0,	CSysExFileLoader::VoicesPerBank-1,	1},			// TGParameterProgram
	{0,	127,					8, ToVolume},		// TGParameterVolume
	{0,	127,					8, ToPan},		// TGParameterPan
	{0,	AudioEffects::Types::UNKNOWN - 1, 1, ToFXType}, // TGParameterInsertFXType
	{0,	MidiEffects::Types::UNKNOWN - 1, 1, ToMidiFXType}, // TGParameterMidiFXType
	{-99,	99,					1},			// TGParameterMasterTune
	{0,	99,					1},			// TGParameterCutoff
	{0,	99,					1},			// TGParameterResonance
	{0,	CMIDIDevice::ChannelUnknown-1,		1, ToMIDIChannel}, 	// TGParameterMIDIChannel
	{0, 100, 1},								// TGParameterSendFX1
	{0, 100, 1},								// TGParameterReverbSend
	{0,	12,					1},			// TGParameterPitchBendRange
	{0,	12,					1},			// TGParameterPitchBendStep
	{0,	1,					1, ToPortaMode},	// TGParameterPortamentoMode
	{0,	1,					1, ToPortaGlissando},	// TGParameterPortamentoGlissando
	{0,	99,					1},			// TGParameterPortamentoTime
	{0,	1,					1, ToPolyMono}, 		// TGParameterMonoMode 
	{0, 99, 1}, //MW Range
	{0, 1, 1, ToOnOff}, //MW Pitch
	{0, 1, 1, ToOnOff}, //MW Amp
	{0, 1, 1, ToOnOff}, //MW EGBias
	{0, 99, 1}, //FC Range
	{0, 1, 1, ToOnOff}, //FC Pitch
	{0, 1, 1, ToOnOff}, //FC Amp
	{0, 1, 1, ToOnOff}, //FC EGBias
	{0, 99, 1}, //BC Range
	{0, 1, 1, ToOnOff}, //BC Pitch
	{0, 1, 1, ToOnOff}, //BC Amp
	{0, 1, 1, ToOnOff}, //BC EGBias
	{0, 99, 1}, //AT Range
	{0, 1, 1, ToOnOff}, //AT Pitch
	{0, 1, 1, ToOnOff}, //AT Amp
	{0, 1, 1, ToOnOff} //AT EGBias	
};

// must match AudioEffectChorus::Param
const CUIMenu::TParameter CUIMenu::s_TGFXChorusParam[AudioEffectChorus::Param::UNKNOWN] =
{
	{0, 1, 1, ToOnOff}, // BYPASS
	{0, 1, 1, ToOnOff}, // CHORUS_I_ENABLE
	{0, 1, 1, ToOnOff}, // CHORUS_II_ENABLE
	{0, 100, 1}, // CHORUS_I_RATE,
	{0, 100, 1} // CHORUS_II_RATE
};

// must match AudioEffectDelay::Param
const CUIMenu::TParameter CUIMenu::s_TGFXDelayParam[AudioEffectDelay::Param::UNKNOWN] =
{
	{0, 1, 1, ToOnOff}, // BYPASS
	{0, AudioEffectDelay::MAX_DELAY_TIME * 1000 + AudioEffectDelay::SyncTime::T_UNKNOWN, 1, ToDelayTime}, // TIME_L
	{0, AudioEffectDelay::MAX_DELAY_TIME * 1000 + AudioEffectDelay::SyncTime::T_UNKNOWN, 1, ToDelayTime}, // TIME_R
	{0, 100, 1}, // FEEDBACK,
	{0, 100, 1}, // TONE
	{0, 1, 1, ToOnOff}, // PING_PONG
	{0, 100, 1, ToMix} // MIX
};

// must match AudioEffectLPF::Param
const CUIMenu::TParameter CUIMenu::s_TGFXLPFParam[AudioEffectLPF::Param::UNKNOWN] =
{
	{0, 1, 1, ToOnOff}, // BYPASS
	{0, 100, 1}, // CUTOFF
	{0, 100, 1} // RESONANCE
};

// must match AudioEffectDS1::Param
const CUIMenu::TParameter CUIMenu::s_TGFXDS1Param[AudioEffectDS1::Param::UNKNOWN] =
{
	{0, 1, 1, ToOnOff}, // BYPASS
	{1, 99, 1}, // DIST
	{1, 99, 1}, // TONE
	{1, 99, 1} // LEVEL
};

// must match AudioEffectBigMuff::Param
const CUIMenu::TParameter CUIMenu::s_TGFXBigMuffParam[AudioEffectBigMuff::Param::UNKNOWN] =
{
	{0, 1, 1, ToOnOff}, // BYPASS
	{0, 100, 1}, // SUSTAIN
	{0, 100, 1}, // TONE
	{0, 100, 1} // LEVEL
};

// must match AudioEffectTalReverb3::Param
const CUIMenu::TParameter CUIMenu::s_TGFXTalReverb3Param[AudioEffectTalReverb3::Param::UNKNOWN] =
{
	{0, 1, 1, ToOnOff}, // BYPASS
	{0, 100, 1}, // DRY
	{0, 100, 1}, // WET
	{0, 100, 1}, // DECAYTIME
	{0, 100, 1}, // PREDELAY
	{0, 100, 1}, // LOWSHELFGAIN
	{0, 100, 1}, // HIGHSHELFGAIN
	{0, 100, 1}, // STEREO
	{0, 1, 1, ToOnOff}, // REALSTEREOMODE
	{0, 1, 1, ToOnOff} // POWER
};

// must match AudioEffectPlateReverb::Param
const CUIMenu::TParameter CUIMenu::s_TGFXReverbParam[AudioEffectPlateReverb::Param::UNKNOWN] =
{
	{0,	1, 1, ToOnOff}, // BYPASS
	{0,	99,	1},	// SIZE
	{0,	99,	1}, // HIGH_DAMP
	{0,	99,	1},	// LOW_DAMP
	{0,	99,	1},	// LOW_PASS
	{0,	99, 1},	// DIFFUSION
	{0,	100, 1, ToMix}, // MIX
	{0,	99, 1},	// LEVEL
};

// must match AudioEffectMVerb::Param
const CUIMenu::TParameter CUIMenu::s_TGFXMVerbParam[AudioEffectMVerb::Param::UNKNOWN] =
{
	{0,	1, 1, ToOnOff}, // BYPASS
	{0,	100, 1},	// DAMPINGFREQ
	{0,	100, 1},	// DENSITY
	{0,	100, 1},	// BANDWIDTHFREQ
	{0,	100, 1},	// DECAY
	{0,	100, 1},	// PREDELAY
	{0,	100, 1},	// SIZE
	{0,	100, 1},	// GAIN
	{0,	100, 1, ToMix}, // MIX
	{0,	100, 1}, // EARLYMIX
};

// must match AudioEffect3BandEQ::Param
const CUIMenu::TParameter CUIMenu::s_TGFX3BandEQParam[AudioEffect3BandEQ::Param::UNKNOWN] =
{
	{0,	1, 1, ToOnOff}, // BYPASS
	{0, 100, 1},	// LOW
	{0, 100, 1},	// MID
	{0, 100, 1},	// HIGH
	{0, 100, 1},	// MASTER
	{0, 1000, 10},	// LOW_MID_FQ
	{1000,	20000, 10},	// MID_HIGH_FQ
};

// must match AudioEffectPhaser::Param
const CUIMenu::TParameter CUIMenu::s_TGFXPhaserParam[AudioEffectPhaser::Param::UNKNOWN] =
{
	{0,	1, 1, ToOnOff}, // BYPASS
	{0, 127, 1}, // WETDRY
	{0, 127, 1}, // PAN
	{1, 600, 1}, // PH_FREQ
	{0, 127, 1}, // PH_RND
	{0, 11, 1},	// TYPE
	{0,	127, 1}, // STDL
	{0,	127, 1}, // PH_DEPTH
	{0,	127, 1}, // FB
	{0,	12, 1}, // STAGES
	{0,	127, 1}, // LRCR
	{0,	1, 1, ToOnOff}, // SUB
	{0,	127, 1}, // PHASE
};

// must match AudioEffectAPhaser::Param
const CUIMenu::TParameter CUIMenu::s_TGFXAPhaserParam[AudioEffectAPhaser::Param::UNKNOWN] =
{
	{0,	1, 1, ToOnOff}, // BYPASS
	{0, 127, 1}, // WETDRY
	{0, 127, 1}, // DISTORTION
	{1, 600, 1}, // PH_FREQ
	{0, 127, 1}, // PH_RND
	{0, 11, 1},	// TYPE
	{0,	127, 1}, // STDL
	{0,	127, 1}, // WIDTH
	{0,	127, 1}, // FB
	{0,	12, 1}, // STAGES
	{0,	127, 1}, // OFFSET
	{0,	1, 1, ToOnOff}, // SUB
	{0,	127, 1}, // PH_DEPTH
	{0,	1, 1, ToOnOff}, // HYPER
};

// must match AudioEffectFlanger::Param
const CUIMenu::TParameter CUIMenu::s_TGFXFlangerParam[AudioEffectFlanger::Param::UNKNOWN] =
{
	{0,	1, 1, ToOnOff}, // BYPASS
	{0, 127, 1}, // MIX
	{0, 127, 1}, // PAN
	{1, 600, 1}, // FL_FREQ
	{0, 127, 1}, // FL_RND
	{0, 11, 1},	// TYPE
	{0,	127, 1}, // STDL
	{0,	127, 1}, // FL_DEPTH
	{0,	127, 1}, // DELAY
	{0,	127, 1}, // FB
	{0,	127, 1}, // LRCR
	{0,	1, 1, ToOnOff}, // MODE
	{0,	1, 1, ToOnOff}, // SUB
	{0,	1, 1, ToOnOff}, // AWESOME
};

// must match MidiArp::Param
const CUIMenu::TParameter CUIMenu::s_TGMidiFXArpParam[MidiArp::Param::UNKNOWN] =
{
	{0, 1, 1, ToOnOff}, // BYPASS
	{0, 1, 1, ToOnOff}, // LATCH
	{0, 2, 1}, // SYNC
	{0, 5, 1, ToArpMode}, // ARP_MODE
	{0, 12, 1, ToArpDivision}, // DIVISION
	{0, 100, 1}, // NOTE_LENGTH
	{0, 127, 1, ToArpVelocity}, // VELOCITY
	{1, 4, 1}, // OCTAVE_SPREAD
	{0, 4, 1, ToArpOctMode}, // OCTAVE_MODE
	{0, 1, 1, ToOnOff} // PANIC
};

// must match DexedVoiceParameters in Synth_Dexed
const CUIMenu::TParameter CUIMenu::s_VoiceParameter[] =
{
	{0,	99,	1},				// DEXED_PITCH_EG_R1
	{0,	99,	1},				// DEXED_PITCH_EG_R2
	{0,	99,	1},				// DEXED_PITCH_EG_R3
	{0,	99,	1},				// DEXED_PITCH_EG_R4
	{0,	99,	1},				// DEXED_PITCH_EG_L1
	{0,	99,	1},				// DEXED_PITCH_EG_L2
	{0,	99,	1},				// DEXED_PITCH_EG_L3
	{0,	99,	1},				// DEXED_PITCH_EG_L4
	{0,	31,	1,	ToAlgorithm},		// DEXED_ALGORITHM
	{0,	7,	1},				// DEXED_FEEDBACK
	{0,	1,	1,	ToOnOff},		// DEXED_OSC_KEY_SYNC
	{0,	99,	1},				// DEXED_LFO_SPEED
	{0,	99,	1},				// DEXED_LFO_DELAY
	{0,	99,	1},				// DEXED_LFO_PITCH_MOD_DEP
	{0,	99,	1},				// DEXED_LFO_AMP_MOD_DEP
	{0,	1,	1,	ToOnOff},		// DEXED_LFO_SYNC
	{0,	5,	1,	ToLFOWaveform},		// DEXED_LFO_WAVE
	{0,	7,	1},				// DEXED_LFO_PITCH_MOD_SENS
	{0,	48,	1,	ToTransposeNote},	// DEXED_TRANSPOSE
	{0,	1,	1}				// Voice Name - Dummy parameters for in case new item would be added in future 
};

// must match DexedVoiceOPParameters in Synth_Dexed
const CUIMenu::TParameter CUIMenu::s_OPParameter[] =
{
	{0,	99,	1},				// DEXED_OP_EG_R1
	{0,	99,	1},				// DEXED_OP_EG_R2
	{0,	99,	1},				// DEXED_OP_EG_R3
	{0,	99,	1},				// DEXED_OP_EG_R4
	{0,	99,	1},				// DEXED_OP_EG_L1
	{0,	99,	1},				// DEXED_OP_EG_L2
	{0,	99,	1},				// DEXED_OP_EG_L3
	{0,	99,	1},				// DEXED_OP_EG_L4
	{0,	99,	1,	ToBreakpointNote},	// DEXED_OP_LEV_SCL_BRK_PT
	{0,	99,	1},				// DEXED_OP_SCL_LEFT_DEPTH
	{0,	99,	1},				// DEXED_OP_SCL_RGHT_DEPTH
	{0,	3,	1,	ToKeyboardCurve},	// DEXED_OP_SCL_LEFT_CURVE
	{0,	3,	1,	ToKeyboardCurve},	// DEXED_OP_SCL_RGHT_CURVE
	{0,	7,	1},				// DEXED_OP_OSC_RATE_SCALE
	{0,	3,	1},				// DEXED_OP_AMP_MOD_SENS
	{0,	7,	1},				// DEXED_OP_KEY_VEL_SENS
	{0,	99,	1},				// DEXED_OP_OUTPUT_LEV
	{0,	1,	1,	ToOscillatorMode},	// DEXED_OP_OSC_MODE
	{0,	31,	1},				// DEXED_OP_FREQ_COARSE
	{0,	99,	1},				// DEXED_OP_FREQ_FINE
	{0,	14,	1,	ToOscillatorDetune},	// DEXED_OP_OSC_DETUNE
	{0, 1, 1, ToOnOff}		// DEXED_OP_ENABLE
};

const char CUIMenu::s_NoteName[100][5] =
{
"A-1", "A#-1", "B-1", "C0", "C#0", "D0", "D#0", "E0", "F0", "F#0", "G0", "G#0",
"A0", "A#0", "B0", "C1", "C#1", "D1", "D#1", "E1", "F1", "F#1", "G1", "G#1",
"A1", "A#1", "B1", "C2", "C#2", "D2", "D#2", "E2", "F2", "F#2", "G2", "G#2",
"A2", "A#2", "B2", "C3", "C#3", "D3", "D#3", "E3", "F3", "F#3", "G3", "G#3",
"A3", "A#3", "B3", "C4", "C#4", "D4", "D#4", "E4", "F4", "F#4", "G4", "G#4",
"A4", "A#4", "B4", "C5", "C#5", "D5", "D#5", "E5", "F5", "F#5", "G5", "G#5",
"A5", "A#5", "B5", "C6", "C#6", "D6", "D#6", "E6", "F6", "F#6", "G6", "G#6",
"A6", "A#6", "B6", "C7", "C#7", "D7", "D#7", "E7", "F7", "F#7", "G7", "G#7",
"A7", "A#7", "B7", "C8"
};

static const unsigned NoteC3 = 39;

const CUIMenu::TMenuItem CUIMenu::s_PerformanceMenu[] =
{
	{"Load",	PerformanceMenu, 0, 0}, 
	{"Save",	MenuHandler,	s_SaveMenu},
	{"Delete",	PerformanceMenu, 0, 1},
	{"Bank",	EditPerformanceBankNumber, 0, 0},
	{"PCCH",	EditGlobalParameter,	0,	CMiniDexed::ParameterPerformanceSelectChannel},
	{0}
};


CUIMenu::CUIMenu (CUserInterface *pUI, CMiniDexed *pMiniDexed, CConfig *pConfig)
:	m_pUI (pUI),
	m_pMiniDexed (pMiniDexed),
	m_pConfig (pConfig),
	m_pParentMenu (s_MenuRoot),
	m_pCurrentMenu (s_MainMenu),
	m_nCurrentMenuItem (0),
	m_nCurrentSelection (0),
	m_nCurrentParameter (0),
	m_nCurrentMenuDepth (0)
{
	assert (m_pConfig);
	m_nToneGenerators = m_pConfig->GetToneGenerators();

	if (m_nToneGenerators == 1)
	{
		// If there is just one core, then there is only a single
		// tone generator so start on the TG1 menu...
		m_pParentMenu = s_MainMenu;
		m_pCurrentMenu = s_TGMenu;
		m_nCurrentMenuItem = 0;
		m_nCurrentSelection = 0;
		m_nCurrentParameter = 0;
		m_nCurrentMenuDepth = 1;

		// Place the "root" menu at the top of the stack
		m_MenuStackParent[0] = s_MenuRoot;
		m_MenuStackMenu[0] = s_MainMenu;
		m_nMenuStackItem[0]	= 0;
		m_nMenuStackSelection[0] = 0;
		m_nMenuStackParameter[0] = 0;
	}
}

void CUIMenu::SetStepCount (unsigned StepCount)
{
	m_nStepCount = StepCount;
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
		if (m_nToneGenerators == 1)
		{
			// "Home" is the TG0 menu if only one TG active
			m_pParentMenu = s_MainMenu;
			m_pCurrentMenu = s_TGMenu;
			m_nCurrentMenuItem = 0;
			m_nCurrentSelection = 0;
			m_nCurrentParameter = 0;
			m_nCurrentMenuDepth = 1;
			// Place the "root" menu at the top of the stack
			m_MenuStackParent[0] = s_MenuRoot;
			m_MenuStackMenu[0] = s_MainMenu;
			m_nMenuStackItem[0] = 0;
			m_nMenuStackSelection[0] = 0;
			m_nMenuStackParameter[0] = 0;
		}
		else
		{
			m_pParentMenu = s_MenuRoot;
			m_pCurrentMenu = s_MainMenu;
			m_nCurrentMenuItem = 0;
			m_nCurrentSelection = 0;
			m_nCurrentParameter = 0;
			m_nCurrentMenuDepth = 0;
		}
		EventHandler (MenuEventUpdate);
		break;

	case MenuEventPgmUp:
	case MenuEventPgmDown:
		PgmUpDownHandler(Event);
		break;

	case MenuEventBankUp:
	case MenuEventBankDown:
		BankUpDownHandler(Event);
		break;

	case MenuEventTGUp:
	case MenuEventTGDown:
		TGUpDownHandler(Event);
		break;

	default:
		(*m_pParentMenu[m_nCurrentMenuItem].Handler) (this, Event);
		break;
	}
}

void CUIMenu::MainMenuHandler (CUIMenu *pUIMenu, TMenuEvent Event)
{
	bool menuChange = false;
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
		menuChange = true;
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
		string strTempo;
		if (menuChange || !pUIMenu->m_pConfig->GetFXEnabled())
		{
			strTempo = "";
		}
		else
		{
			strTempo = (pUIMenu->m_pMiniDexed->isPlaying() ? ">" : "#") +
				to_string(pUIMenu->m_pMiniDexed->getTempo());
		}

		pUIMenu->m_pUI->DisplayWrite (
			pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
			strTempo.c_str(),
			pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].Name,
			pUIMenu->m_nCurrentSelection > 0,
			!!pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection+1].Name);
	}
	else
	{
		pUIMenu->EventHandler (MenuEventUpdate);	// no, update parameter display
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
		if (pUIMenu->m_nCurrentSelection == 0)
		{
			// If in main mennu, wrap around
			if (pUIMenu->m_pCurrentMenu == s_MainMenu)
			{
				// Find last entry with a name
				while (pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection+1].Name)
				{
					pUIMenu->m_nCurrentSelection++;
				}
			}
		}
		else if (pUIMenu->m_nCurrentSelection > 0)
		{
			pUIMenu->m_nCurrentSelection--;
		}
		// Might need to trim menu if number of TGs is configured to be less than the maximum supported
		while ((pUIMenu->m_pCurrentMenu == s_MainMenu) && (pUIMenu->m_nCurrentSelection > 0) &&
			  	(	// Skip any unused menus
			   		(pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].MenuItem == s_TGMenu) &&
			   		(pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].Parameter >= pUIMenu->m_nToneGenerators) &&
			   		(pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].Parameter < CConfig::AllToneGenerators)
				)
			  )
		{
			pUIMenu->m_nCurrentSelection--;
		}
		break;

	case MenuEventStepUp:
		++pUIMenu->m_nCurrentSelection;
		if (!pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].Name)  // more entries?
		{
			if (pUIMenu->m_pCurrentMenu == s_MainMenu)
			{
				// If in main mennu, wrap around
				pUIMenu->m_nCurrentSelection = 0;
			}
			else
			{
				// Return to last known good item
				pUIMenu->m_nCurrentSelection--;
			}
		}
		// Might need to trim menu if number of TGs is configured to be less than the maximum supported
		while ((pUIMenu->m_pCurrentMenu == s_MainMenu) && (pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection+1].Name) &&
			   	(	// Skip any unused TG menus
					(pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].MenuItem == s_TGMenu) &&
					(pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].Parameter >= pUIMenu->m_nToneGenerators) &&
					(pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].Parameter < CConfig::AllToneGenerators)
				)
			  )
		{
			pUIMenu->m_nCurrentSelection++;
		}
		break;

	default:
		return;
	}

	if (pUIMenu->m_pCurrentMenu)				// if this is another menu?
	{
		bool bIsMainMenu = pUIMenu->m_pCurrentMenu == s_MainMenu;
		pUIMenu->m_pUI->DisplayWrite (
			pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
			"",
			pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].Name,
			pUIMenu->m_nCurrentSelection > 0 || bIsMainMenu,
			!!pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection+1].Name || bIsMainMenu);
	}
	else
	{
		pUIMenu->EventHandler (MenuEventUpdate);	// no, update parameter display
	}
}

void CUIMenu::MenuHandlerMidiFX (CUIMenu *pUIMenu, TMenuEvent Event)
{
	// Setup menu when it's open
	if (!pUIMenu->m_pConfig->GetFXEnabled()) {
		pUIMenu->m_pCurrentMenu = s_FXDisabled;
	}
	else if (!pUIMenu->m_pCurrentMenu)
	{
		pUIMenu->m_pCurrentMenu = s_MidiFX;
	}

	// Identify TG
	unsigned nTG = pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth-1];
	
	switch (Event)
	{
	case MenuEventUpdate:
		break;

	case MenuEventSelect:				// push menu
		assert (pUIMenu->m_nCurrentMenuDepth < MaxMenuDepth);

		// Check if TG is enabled
		if (nTG >= pUIMenu->m_pConfig->GetToneGenerators()) {
			break;
		}

		// Check if FXs are enabled
		if (!pUIMenu->m_pConfig->GetFXEnabled()) {
			break;
		}

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

	case MenuEventPressAndStepDown:
	case MenuEventPressAndStepUp:
		pUIMenu->TGShortcutHandler (Event);
		return;

	default:
		return;
	}

	if (pUIMenu->m_pCurrentMenu)				// if this is another menu?
	{
		// Create TG label
		string TG ("TG");
		TG += to_string (nTG+1);

		// Check if FXs are enabled
		std::string value;
		if (pUIMenu->m_pConfig->GetFXEnabled()) {
			// Get current FX type
			int nFxType = pUIMenu->m_pMiniDexed->GetTGParameter(CMiniDexed::TGParameterMidiFXType, nTG);

			// Create Paramter with type label
			value.append(pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].Name);
			value.append(getMidiFXTypeName(nFxType));
		} else {
			value.append(pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].Name);
		}

		pUIMenu->m_pUI->DisplayWrite (
			TG.c_str (),
			pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
			value.c_str(),
			pUIMenu->m_nCurrentSelection > 0,
			!!pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection+1].Name);
	}
	else
	{
		pUIMenu->EventHandler (MenuEventUpdate);	// no, update parameter display
	}
}

void CUIMenu::MenuHandlerInsertFX (CUIMenu *pUIMenu, TMenuEvent Event)
{
	// Setup menu when it's open
	if (!pUIMenu->m_pConfig->GetFXEnabled()) {
		pUIMenu->m_pCurrentMenu = s_FXDisabled;
	}
	else if (!pUIMenu->m_pCurrentMenu)
	{
		pUIMenu->m_pCurrentMenu = s_InsertFX;
	}

	// Identify TG
	unsigned nTG = pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth-1];
	
	switch (Event)
	{
	case MenuEventUpdate:
		break;

	case MenuEventSelect:				// push menu
		assert (pUIMenu->m_nCurrentMenuDepth < MaxMenuDepth);
		
		// Check if TG is enabled
		if (nTG >= pUIMenu->m_pConfig->GetToneGenerators()) {
			break;
		}

		// Check if FXs are enabled
		if (!pUIMenu->m_pConfig->GetFXEnabled()) {
			break;
		}

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

	case MenuEventPressAndStepDown:
	case MenuEventPressAndStepUp:
		pUIMenu->TGShortcutHandler (Event);
		return;

	default:
		return;
	}

	if (pUIMenu->m_pCurrentMenu)				// if this is another menu?
	{
		// Create TG label
		string TG ("TG");
		TG += to_string (nTG+1);

		// Check if FXs are enabled
		std::string value;
		if (pUIMenu->m_pConfig->GetFXEnabled()) {
			// Get current FX Name
			std::string fxName = pUIMenu->m_pMiniDexed->getInsertFXName(nTG);

			// Create Paramter with type label
			value.append(pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].Name);
			value.append(fxName);
		} else {
			value.append(pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].Name);
		}

		pUIMenu->m_pUI->DisplayWrite (
			TG.c_str (),
			pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
			value.c_str(),
			pUIMenu->m_nCurrentSelection > 0,
			!!pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection+1].Name);
	}
	else
	{
		pUIMenu->EventHandler (MenuEventUpdate);	// no, update parameter display
	}
}

void CUIMenu::MenuHandlerSendFX1 (CUIMenu *pUIMenu, TMenuEvent Event)
{
	// Setup menu when it's open
	if (!pUIMenu->m_pConfig->GetFXEnabled()) {
		pUIMenu->m_pCurrentMenu = s_FXDisabled;
	}
	else if (!pUIMenu->m_pCurrentMenu)
	{
		pUIMenu->m_pCurrentMenu = s_SendFX1Menu;
	}
	
	switch (Event)
	{
	case MenuEventUpdate:
		break;

	case MenuEventSelect:				// push menu
		assert (pUIMenu->m_nCurrentMenuDepth < MaxMenuDepth);

		// Check if FXs are enabled
		if (!pUIMenu->m_pConfig->GetFXEnabled()) {
			break;
		}

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

	case MenuEventPressAndStepDown:
	case MenuEventPressAndStepUp:
		pUIMenu->TGShortcutHandler (Event);
		return;

	default:
		return;
	}

	if (pUIMenu->m_pCurrentMenu)				// if this is another menu?
	{
		// Check if FXs are enabled
		std::string value;
		if (pUIMenu->m_pConfig->GetFXEnabled()) {
			// Get current FX Name
			std::string fxName = pUIMenu->m_pMiniDexed->getSendFX1Name();

			// Create Paramter with type label
			value.append(pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].Name);
			if (pUIMenu->m_nCurrentSelection < 2)
			{
				value.append(fxName);
			}
		} else {
			value.append(pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].Name);
		}

		pUIMenu->m_pUI->DisplayWrite (
			pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
			"",
			value.c_str(),
			pUIMenu->m_nCurrentSelection > 0,
			!!pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection+1].Name);
	}
	else
	{
		pUIMenu->EventHandler (MenuEventUpdate);	// no, update parameter display
	}
}

void CUIMenu::MenuHandlerSendFX2 (CUIMenu *pUIMenu, TMenuEvent Event)
{
	// Setup menu when it's open
	if (!pUIMenu->m_pConfig->GetFXEnabled()) {
		pUIMenu->m_pCurrentMenu = s_FXDisabled;
	}
	else if (!pUIMenu->m_pCurrentMenu)
	{
		pUIMenu->m_pCurrentMenu = s_SendFX2Menu;
	}
	
	switch (Event)
	{
	case MenuEventUpdate:
		break;

	case MenuEventSelect:				// push menu
		assert (pUIMenu->m_nCurrentMenuDepth < MaxMenuDepth);

		// Check if FXs are enabled
		if (!pUIMenu->m_pConfig->GetFXEnabled()) {
			break;
		}

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

	case MenuEventPressAndStepDown:
	case MenuEventPressAndStepUp:
		pUIMenu->TGShortcutHandler (Event);
		return;

	default:
		return;
	}

	if (pUIMenu->m_pCurrentMenu)				// if this is another menu?
	{
		// Check if FXs are enabled
		std::string value;
		if (pUIMenu->m_pConfig->GetFXEnabled()) {
			// Get current FX Name
			std::string fxName = pUIMenu->m_pMiniDexed->getSendFX2Name();

			// Create Paramter with type label
			value.append(pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].Name);
			if (pUIMenu->m_nCurrentSelection < 2)
			{
				value.append(fxName);
			}
		} else {
			value.append(pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].Name);
		}

		pUIMenu->m_pUI->DisplayWrite (
			pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
			"",
			value.c_str(),
			pUIMenu->m_nCurrentSelection > 0,
			!!pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection+1].Name);
	}
	else
	{
		pUIMenu->EventHandler (MenuEventUpdate);	// no, update parameter display
	}
}

void CUIMenu::MenuHandlerMasterFX (CUIMenu *pUIMenu, TMenuEvent Event)
{
	// Setup menu when it's open
	if (!pUIMenu->m_pConfig->GetFXEnabled()) {
		pUIMenu->m_pCurrentMenu = s_FXDisabled;
	}
	else if (!pUIMenu->m_pCurrentMenu)
	{
		pUIMenu->m_pCurrentMenu = s_MasterFXMenu;
	}
	
	switch (Event)
	{
	case MenuEventUpdate:
		break;

	case MenuEventSelect:				// push menu
		assert (pUIMenu->m_nCurrentMenuDepth < MaxMenuDepth);

		// Check if FXs are enabled
		if (!pUIMenu->m_pConfig->GetFXEnabled()) {
			break;
		}

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

	case MenuEventPressAndStepDown:
	case MenuEventPressAndStepUp:
		pUIMenu->TGShortcutHandler (Event);
		return;

	default:
		return;
	}

	if (pUIMenu->m_pCurrentMenu)				// if this is another menu?
	{
		// Check if FXs are enabled
		std::string value;
		if (pUIMenu->m_pConfig->GetFXEnabled()) {
			// Get current FX Name
			std::string fxName = pUIMenu->m_pMiniDexed->getMasterFXName();

			// Create Paramter with type label
			value.append(pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].Name);
			if (pUIMenu->m_nCurrentSelection < 2)
			{
				value.append(fxName);
			}
		} else {
			value.append(pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].Name);
		}

		pUIMenu->m_pUI->DisplayWrite (
			pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
			"",
			value.c_str(),
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

	int nValue = pUIMenu->m_pMiniDexed->GetTGParameter (CMiniDexed::TGParameterVoiceBank, nTG);

	switch (Event)
	{
	case MenuEventUpdate:
		break;

	case MenuEventStepDown:
		nValue = pUIMenu->m_pMiniDexed->GetSysExFileLoader ()->GetNextBankDown(nValue);
		pUIMenu->m_pMiniDexed->SetTGParameter (
			CMiniDexed::TGParameterVoiceBank, nValue, nTG);
		break;

	case MenuEventStepUp:
		nValue = pUIMenu->m_pMiniDexed->GetSysExFileLoader ()->GetNextBankUp(nValue);
		pUIMenu->m_pMiniDexed->SetTGParameter (
			CMiniDexed::TGParameterVoiceBank, nValue, nTG);
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

	int nValue = pUIMenu->m_pMiniDexed->GetTGParameter (CMiniDexed::TGParameterProgram, nTG);

	switch (Event)
	{
	case MenuEventUpdate:
		break;

	case MenuEventStepDown:
		if (--nValue < 0)
		{
			// Switch down a voice bank and set to the last voice
			nValue = CSysExFileLoader::VoicesPerBank-1;
			int nVB = pUIMenu->m_pMiniDexed->GetTGParameter(CMiniDexed::TGParameterVoiceBank, nTG);
			nVB = pUIMenu->m_pMiniDexed->GetSysExFileLoader ()->GetNextBankDown(nVB);
			pUIMenu->m_pMiniDexed->SetTGParameter (CMiniDexed::TGParameterVoiceBank, nVB, nTG);
		}
		pUIMenu->m_pMiniDexed->SetTGParameter (CMiniDexed::TGParameterProgram, nValue, nTG);
		break;

	case MenuEventStepUp:
		if (++nValue > (int) CSysExFileLoader::VoicesPerBank-1)
		{
			// Switch up a voice bank and reset to voice 0
			nValue = 0;
			int nVB = pUIMenu->m_pMiniDexed->GetTGParameter(CMiniDexed::TGParameterVoiceBank, nTG);
			nVB = pUIMenu->m_pMiniDexed->GetSysExFileLoader ()->GetNextBankUp(nVB);
			pUIMenu->m_pMiniDexed->SetTGParameter (CMiniDexed::TGParameterVoiceBank, nVB, nTG);
		}
		pUIMenu->m_pMiniDexed->SetTGParameter (CMiniDexed::TGParameterProgram, nValue, nTG);
		break;

	case MenuEventPressAndStepDown:
	case MenuEventPressAndStepUp:
		pUIMenu->TGShortcutHandler (Event);
		return;

	default:
		return;
	}

	// Skip empty voices.
	// Use same criteria in PgmUpDownHandler() too.
	string voiceName = pUIMenu->m_pMiniDexed->GetVoiceName (nTG);
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

void CUIMenu::EditMidiFX (CUIMenu *pUIMenu, TMenuEvent Event)
{
	unsigned nTG = pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth-2]; 

	int nFXType = pUIMenu->m_pMiniDexed->GetTGParameter(CMiniDexed::TGParameterMidiFXType, nTG);
	pUIMenu->m_pCurrentMenu = getMidiFXMenuItem(nFXType);

	switch (Event)
	{
	case MenuEventUpdate:
		break;

	case MenuEventSelect:				// push menu
		if (nFXType == MidiEffect::ID)
		{
			break;
		}
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
		string TG ("TG");
		TG += to_string (nTG+1);
		
		pUIMenu->m_pUI->DisplayWrite (
			TG.c_str (),
			getMidiFXTypeName(nFXType).c_str(),
			pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].Name,
			pUIMenu->m_nCurrentSelection > 0,
			!!pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection+1].Name);
	}
	else
	{
		pUIMenu->EventHandler (MenuEventUpdate);	// no, update parameter display
	}
}

void CUIMenu::EditTGMidiFXParameter (CUIMenu *pUIMenu, TMenuEvent Event)
{
	// Get TG
	unsigned nTG = pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth-3];

	// Get FX type
	int nFXType = pUIMenu->m_pMiniDexed->GetTGParameter(CMiniDexed::TGParameterMidiFXType, nTG);
	
	// Get Param
	unsigned nParam = pUIMenu->m_nCurrentParameter;
	TParameter pParam = getMidiFXParameter(nFXType, nParam);
	const TParameter &rParam = pParam;

	int nValue = pUIMenu->m_pMiniDexed->GetMidiFXParameter (nParam, nTG, nFXType);
	
	switch (Event)
	{
	case MenuEventUpdate:
		break;

	case MenuEventPressAndStepDown:
		nValue -= rParam.Increment * 9;
	case MenuEventStepDown:
		nValue -= rParam.Increment * pUIMenu->m_nStepCount;
		if (nValue < rParam.Minimum)
		{
			nValue = rParam.Minimum;
		}
		pUIMenu->m_pMiniDexed->SetMidiFXParameter (nParam, nValue, nTG, nFXType);
		break;

	case MenuEventPressAndStepUp:
		nValue += rParam.Increment * 9;
	case MenuEventStepUp:
		nValue += rParam.Increment * pUIMenu->m_nStepCount;
		if (nValue > rParam.Maximum)
		{
			nValue = rParam.Maximum;
		}
		pUIMenu->m_pMiniDexed->SetMidiFXParameter (nParam, nValue, nTG, nFXType);
		break;

	default:
		return;
	}

	string TG ("TG");
	TG += to_string (nTG+1);

	// Get value again after change
	nValue = pUIMenu->m_pMiniDexed->GetMidiFXParameter (nParam, nTG, nFXType);
	CUIMenu::TToString *pToString = rParam.ToString;
	string Value;
	if (pToString)
	{
		Value = (*pToString) (nValue);
	}
	else
	{
		Value = to_string (nValue);
	}

	pUIMenu->m_pUI->DisplayWrite (TG.c_str (),
				      pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
				      Value.c_str (),
				      nValue > rParam.Minimum, nValue < rParam.Maximum);
}

void CUIMenu::EditInsertFX (CUIMenu *pUIMenu, TMenuEvent Event)
{
	unsigned nTG = pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth-2]; 

	int nFXType = pUIMenu->m_pMiniDexed->GetTGParameter(CMiniDexed::TGParameterInsertFXType, nTG);
	pUIMenu->m_pCurrentMenu = getInsertFXMenuItem(nFXType);

	switch (Event)
	{
	case MenuEventUpdate:
		break;

	case MenuEventSelect:				// push menu
		if (nFXType == 0)
		{
			break;
		}
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
		string TG ("TG");
		TG += to_string (nTG+1);

		string fxName = pUIMenu->m_pMiniDexed->getInsertFXName(nTG);
		
		pUIMenu->m_pUI->DisplayWrite (
			TG.c_str (),
			fxName.c_str(),
			pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection].Name,
			pUIMenu->m_nCurrentSelection > 0,
			!!pUIMenu->m_pCurrentMenu[pUIMenu->m_nCurrentSelection+1].Name);
	}
	else
	{
		pUIMenu->EventHandler (MenuEventUpdate);	// no, update parameter display
	}
}

void CUIMenu::EditTGFXParameter (CUIMenu *pUIMenu, TMenuEvent Event)
{
	// Get TG
	unsigned nTG = pUIMenu->m_nMenuStackParameter[pUIMenu->m_nCurrentMenuDepth-3];

	// Get FX type
	int nFXType = pUIMenu->m_pMiniDexed->GetTGParameter(CMiniDexed::TGParameterInsertFXType, nTG);
	
	// Get Param
	unsigned nParam = pUIMenu->m_nCurrentParameter;
	TParameter pParam = getFXParameter(nFXType, nParam);
	const TParameter &rParam = pParam;

	int nValue = pUIMenu->m_pMiniDexed->GetTGFXParameter (nParam, nTG, nFXType);
	
	switch (Event)
	{
	case MenuEventUpdate:
		break;

	case MenuEventPressAndStepDown:
		nValue -= rParam.Increment * 9;
	case MenuEventStepDown:
		nValue -= rParam.Increment * pUIMenu->m_nStepCount;
		if (nValue < rParam.Minimum)
		{
			nValue = rParam.Minimum;
		}
		pUIMenu->m_pMiniDexed->SetTGFXParameter (nParam, nValue, nTG, nFXType);
		break;

	case MenuEventPressAndStepUp:
		nValue += rParam.Increment * 9;
	case MenuEventStepUp:
		nValue += rParam.Increment * pUIMenu->m_nStepCount;
		if (nValue > rParam.Maximum)
		{
			nValue = rParam.Maximum;
		}
		pUIMenu->m_pMiniDexed->SetTGFXParameter (nParam, nValue, nTG, nFXType);
		break;

	default:
		return;
	}

	string TG ("TG");
	TG += to_string (nTG+1);

	// Get value again after change
	nValue = pUIMenu->m_pMiniDexed->GetTGFXParameter (nParam, nTG, nFXType);
	CUIMenu::TToString *pToString = rParam.ToString;
	string Value;
	if (pToString)
	{
		Value = (*pToString) (nValue);
	}
	else
	{
		Value = to_string (nValue);
	}

	pUIMenu->m_pUI->DisplayWrite (TG.c_str (),
				      pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
				      Value.c_str (),
				      nValue > rParam.Minimum, nValue < rParam.Maximum);
}

void CUIMenu::EditSendFX1 (CUIMenu *pUIMenu, TMenuEvent Event)
{
	int nFXType = pUIMenu->m_pMiniDexed->GetParameter(CMiniDexed::ParameterSendFX1Type);
	pUIMenu->m_pCurrentMenu = getSendFX1MenuItem(nFXType);
	
	switch (Event)
	{
	case MenuEventUpdate:
		break;

	case MenuEventSelect:				// push menu
		if (nFXType == 0)
		{
			break;
		}
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
		string fxName = pUIMenu->m_pMiniDexed->getSendFX1Name();

		pUIMenu->m_pUI->DisplayWrite (
			fxName.c_str(),
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

void CUIMenu::EditSendFX2 (CUIMenu *pUIMenu, TMenuEvent Event)
{
	int nFXType = pUIMenu->m_pMiniDexed->GetParameter(CMiniDexed::ParameterSendFX2Type);
	pUIMenu->m_pCurrentMenu = getSendFX2MenuItem(nFXType);
	
	switch (Event)
	{
	case MenuEventUpdate:
		break;

	case MenuEventSelect:				// push menu
		if (nFXType == 0)
		{
			break;
		}
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
		string fxName = pUIMenu->m_pMiniDexed->getSendFX2Name();

		pUIMenu->m_pUI->DisplayWrite (
			fxName.c_str(),
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

void CUIMenu::EditSendFX1Parameter (CUIMenu *pUIMenu, TMenuEvent Event)
{
	// Get FX type
	int nFXType = pUIMenu->m_pMiniDexed->GetParameter(CMiniDexed::ParameterSendFX1Type);
	
	// Get Param
	unsigned nParam = pUIMenu->m_nCurrentParameter;
	TParameter pParam = getFXParameter(nFXType, nParam);
	const TParameter &rParam = pParam;

	int nValue = pUIMenu->m_pMiniDexed->GetSendFX1Parameter (nParam, nFXType);
	
	switch (Event)
	{
	case MenuEventUpdate:
		break;

	case MenuEventPressAndStepDown:
		nValue -= rParam.Increment * 9;
	case MenuEventStepDown:
		nValue -= rParam.Increment * pUIMenu->m_nStepCount;
		if (nValue < rParam.Minimum)
		{
			nValue = rParam.Minimum;
		}
		pUIMenu->m_pMiniDexed->SetSendFX1Parameter (nParam, nValue, nFXType);
		break;

	case MenuEventPressAndStepUp:
		nValue += rParam.Increment * 9;
	case MenuEventStepUp:
		nValue += rParam.Increment * pUIMenu->m_nStepCount;
		if (nValue > rParam.Maximum)
		{
			nValue = rParam.Maximum;
		}
		pUIMenu->m_pMiniDexed->SetSendFX1Parameter (nParam, nValue, nFXType);
		break;

	default:
		return;
	}

	// Get value again after change
	nValue = pUIMenu->m_pMiniDexed->GetSendFX1Parameter (nParam, nFXType);
	CUIMenu::TToString *pToString = rParam.ToString;
	string Value;
	if (pToString)
	{
		Value = (*pToString) (nValue);
	}
	else
	{
		Value = to_string (nValue);
	}

	pUIMenu->m_pUI->DisplayWrite (
				      pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
					  "",
				      Value.c_str (),
				      nValue > rParam.Minimum, nValue < rParam.Maximum);
}

void CUIMenu::EditSendFX2Parameter (CUIMenu *pUIMenu, TMenuEvent Event)
{
	// Get FX type
	int nFXType = pUIMenu->m_pMiniDexed->GetParameter(CMiniDexed::ParameterSendFX2Type);
	
	// Get Param
	unsigned nParam = pUIMenu->m_nCurrentParameter;
	TParameter pParam = getFXParameter(nFXType, nParam);
	const TParameter &rParam = pParam;

	int nValue = pUIMenu->m_pMiniDexed->GetSendFX2Parameter (nParam, nFXType);
	
	switch (Event)
	{
	case MenuEventUpdate:
		break;

	case MenuEventPressAndStepDown:
		nValue -= rParam.Increment * 9;
	case MenuEventStepDown:
		nValue -= rParam.Increment * pUIMenu->m_nStepCount;
		if (nValue < rParam.Minimum)
		{
			nValue = rParam.Minimum;
		}
		pUIMenu->m_pMiniDexed->SetSendFX2Parameter (nParam, nValue, nFXType);
		break;

	case MenuEventPressAndStepUp:
		nValue += rParam.Increment * 9;
	case MenuEventStepUp:
		nValue += rParam.Increment * pUIMenu->m_nStepCount;
		if (nValue > rParam.Maximum)
		{
			nValue = rParam.Maximum;
		}
		pUIMenu->m_pMiniDexed->SetSendFX2Parameter (nParam, nValue, nFXType);
		break;

	default:
		return;
	}

	// Get value again after change
	nValue = pUIMenu->m_pMiniDexed->GetSendFX2Parameter (nParam, nFXType);
	CUIMenu::TToString *pToString = rParam.ToString;
	string Value;
	if (pToString)
	{
		Value = (*pToString) (nValue);
	}
	else
	{
		Value = to_string (nValue);
	}

	pUIMenu->m_pUI->DisplayWrite (
				      pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
					  "",
				      Value.c_str (),
				      nValue > rParam.Minimum, nValue < rParam.Maximum);
}

void CUIMenu::EditMasterFX (CUIMenu *pUIMenu, TMenuEvent Event)
{
	int nFXType = pUIMenu->m_pMiniDexed->GetParameter(CMiniDexed::ParameterMasterFXType);
	pUIMenu->m_pCurrentMenu = getMasterFXMenuItem(nFXType);
	
	switch (Event)
	{
	case MenuEventUpdate:
		break;

	case MenuEventSelect:				// push menu
		if (nFXType == 0)
		{
			break;
		}
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
		string fxName = pUIMenu->m_pMiniDexed->getMasterFXName();

		pUIMenu->m_pUI->DisplayWrite (
			fxName.c_str(),
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

void CUIMenu::EditMasterFXParameter (CUIMenu *pUIMenu, TMenuEvent Event)
{
	// Get FX type
	int nFXType = pUIMenu->m_pMiniDexed->GetParameter(CMiniDexed::ParameterMasterFXType);
	
	// Get Param
	unsigned nParam = pUIMenu->m_nCurrentParameter;
	TParameter pParam = getFXParameter(nFXType, nParam);
	const TParameter &rParam = pParam;

	int nValue = pUIMenu->m_pMiniDexed->GetMasterFXParameter (nParam, nFXType);
	
	switch (Event)
	{
	case MenuEventUpdate:
		break;

	case MenuEventPressAndStepDown:
		nValue -= rParam.Increment * 9;
	case MenuEventStepDown:
		nValue -= rParam.Increment * pUIMenu->m_nStepCount;
		if (nValue < rParam.Minimum)
		{
			nValue = rParam.Minimum;
		}
		pUIMenu->m_pMiniDexed->SetMasterFXParameter (nParam, nValue, nFXType);
		break;

	case MenuEventPressAndStepUp:
		nValue += rParam.Increment * 9;
	case MenuEventStepUp:
		nValue += rParam.Increment * pUIMenu->m_nStepCount;
		if (nValue > rParam.Maximum)
		{
			nValue = rParam.Maximum;
		}
		pUIMenu->m_pMiniDexed->SetMasterFXParameter (nParam, nValue, nFXType);
		break;

	default:
		return;
	}

	// Get value again after change
	nValue = pUIMenu->m_pMiniDexed->GetMasterFXParameter (nParam, nFXType);
	CUIMenu::TToString *pToString = rParam.ToString;
	string Value;
	if (pToString)
	{
		Value = (*pToString) (nValue);
	}
	else
	{
		Value = to_string (nValue);
	}

	pUIMenu->m_pUI->DisplayWrite (
				      pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name,
					  "",
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
	assert (nTG < CConfig::AllToneGenerators);
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

	if (nTG < m_nToneGenerators)
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

void CUIMenu::PgmUpDownHandler (TMenuEvent Event)
{
	if (m_pMiniDexed->GetParameter (CMiniDexed::ParameterPerformanceSelectChannel) != CMIDIDevice::Disabled)
	{
		// Program Up/Down acts on performances
		unsigned nLastPerformance = m_pMiniDexed->GetLastPerformance();
		unsigned nPerformance = m_pMiniDexed->GetActualPerformanceID();
		unsigned nStart = nPerformance;
		//LOGNOTE("Performance actual=%d, last=%d", nPerformance, nLastPerformance);
		if (Event == MenuEventPgmDown)
		{
			do
			{
				if (nPerformance == 0)
				{
					// Wrap around
					nPerformance = nLastPerformance;
				}
				else if (nPerformance > 0)
				{
					--nPerformance;
				}
			} while ((m_pMiniDexed->IsValidPerformance(nPerformance) != true) && (nPerformance != nStart));
			m_nSelectedPerformanceID = nPerformance;
			m_pMiniDexed->SetNewPerformance(m_nSelectedPerformanceID);
			//LOGNOTE("Performance new=%d, last=%d", m_nSelectedPerformanceID, nLastPerformance);
		}
		else // MenuEventPgmUp
		{
			do
			{
				if (nPerformance == nLastPerformance)
				{
					// Wrap around
					nPerformance = 0;
				}
				else if (nPerformance < nLastPerformance)
				{
					++nPerformance;
				}
			} while ((m_pMiniDexed->IsValidPerformance(nPerformance) != true) && (nPerformance != nStart));
			m_nSelectedPerformanceID = nPerformance;
			m_pMiniDexed->SetNewPerformance(m_nSelectedPerformanceID);
			//LOGNOTE("Performance new=%d, last=%d", m_nSelectedPerformanceID, nLastPerformance);
		}
	}
	else
	{
		// Program Up/Down acts on voices within a TG.
	
		// If we're not in the root menu, then see if we are already in a TG menu,
		// then find the current TG number. Otherwise assume TG1 (nTG=0).
		unsigned nTG = 0;
		if (m_MenuStackMenu[0] == s_MainMenu && (m_pCurrentMenu == s_TGMenu) || (m_MenuStackMenu[1] == s_TGMenu)) {
			nTG = m_nMenuStackSelection[0];
		}
		assert (nTG < CConfig::AllToneGenerators);
		if (nTG < m_nToneGenerators)
		{
			int nPgm = m_pMiniDexed->GetTGParameter (CMiniDexed::TGParameterProgram, nTG);

			assert (Event == MenuEventPgmDown || Event == MenuEventPgmUp);
			if (Event == MenuEventPgmDown)
			{
				//LOGNOTE("PgmDown");
				if (--nPgm < 0)
				{
					// Switch down a voice bank and set to the last voice
					nPgm = CSysExFileLoader::VoicesPerBank-1;
					int nVB = m_pMiniDexed->GetTGParameter(CMiniDexed::TGParameterVoiceBank, nTG);
					nVB = m_pMiniDexed->GetSysExFileLoader ()->GetNextBankDown(nVB);
					m_pMiniDexed->SetTGParameter (CMiniDexed::TGParameterVoiceBank, nVB, nTG);
				}
				m_pMiniDexed->SetTGParameter (CMiniDexed::TGParameterProgram, nPgm, nTG);
			}
			else
			{
				//LOGNOTE("PgmUp");
				if (++nPgm > (int) CSysExFileLoader::VoicesPerBank-1)
				{
					// Switch up a voice bank and reset to voice 0
					nPgm = 0;
					int nVB = m_pMiniDexed->GetTGParameter(CMiniDexed::TGParameterVoiceBank, nTG);
					nVB = m_pMiniDexed->GetSysExFileLoader ()->GetNextBankUp(nVB);
					m_pMiniDexed->SetTGParameter (CMiniDexed::TGParameterVoiceBank, nVB, nTG);
				}
				m_pMiniDexed->SetTGParameter (CMiniDexed::TGParameterProgram, nPgm, nTG);
			}

			// Skip empty voices.
			// Use same criteria in EditProgramNumber () too.
			string voiceName = m_pMiniDexed->GetVoiceName (nTG);
			if (voiceName == "EMPTY     "
				|| voiceName == "          "
				|| voiceName == "----------"
				|| voiceName == "~~~~~~~~~~" )
			{
				if (Event == MenuEventPgmUp) {
					PgmUpDownHandler (MenuEventPgmUp);
				}
				if (Event == MenuEventPgmDown) {
					PgmUpDownHandler (MenuEventPgmDown);
				}
			}
		}
	}
}

void CUIMenu::BankUpDownHandler (TMenuEvent Event)
{
	if (m_pMiniDexed->GetParameter (CMiniDexed::ParameterPerformanceSelectChannel) != CMIDIDevice::Disabled)
	{
		// Bank Up/Down acts on performances
		unsigned nLastPerformanceBank = m_pMiniDexed->GetLastPerformanceBank();
		unsigned nPerformanceBank = m_nSelectedPerformanceBankID;
		unsigned nStartBank = nPerformanceBank;
		//LOGNOTE("Performance Bank actual=%d, last=%d", nPerformanceBank, nLastPerformanceBank);
		if (Event == MenuEventBankDown)
		{
			do
			{
				if (nPerformanceBank == 0)
				{
					// Wrap around
					nPerformanceBank = nLastPerformanceBank;
				}
				else if (nPerformanceBank > 0)
				{
					--nPerformanceBank;
				}
			} while ((m_pMiniDexed->IsValidPerformanceBank(nPerformanceBank) != true) && (nPerformanceBank != nStartBank));
			m_nSelectedPerformanceBankID = nPerformanceBank;
			// Switch to the new bank and select the first performance voice
			m_pMiniDexed->SetParameter (CMiniDexed::ParameterPerformanceBank, nPerformanceBank);
			m_pMiniDexed->SetFirstPerformance();
			//LOGNOTE("Performance Bank new=%d, last=%d", m_nSelectedPerformanceBankID, nLastPerformanceBank);
		}
		else // MenuEventBankUp
		{
			do
			{
				if (nPerformanceBank == nLastPerformanceBank)
				{
					// Wrap around
					nPerformanceBank = 0;
				}
				else if (nPerformanceBank < nLastPerformanceBank)
				{
					++nPerformanceBank;
				}
			} while ((m_pMiniDexed->IsValidPerformanceBank(nPerformanceBank) != true) && (nPerformanceBank != nStartBank));
			m_nSelectedPerformanceBankID = nPerformanceBank;
			m_pMiniDexed->SetParameter (CMiniDexed::ParameterPerformanceBank, nPerformanceBank);
			m_pMiniDexed->SetFirstPerformance();
			//LOGNOTE("Performance Bank new=%d, last=%d", m_nSelectedPerformanceBankID, nLastPerformanceBank);
		}
	}
	else
	{
		// Bank Up/Down acts on voices within a TG.
	
		// If we're not in the root menu, then see if we are already in a TG menu,
		// then find the current TG number. Otherwise assume TG1 (nTG=0).
		unsigned nTG = 0;
		if (m_MenuStackMenu[0] == s_MainMenu && (m_pCurrentMenu == s_TGMenu) || (m_MenuStackMenu[1] == s_TGMenu)) {
			nTG = m_nMenuStackSelection[0];
		}
		assert (nTG < CConfig::AllToneGenerators);
		if (nTG < m_nToneGenerators)
		{
			int nBank = m_pMiniDexed->GetTGParameter (CMiniDexed::TGParameterVoiceBank, nTG);

			assert (Event == MenuEventBankDown || Event == MenuEventBankUp);
			if (Event == MenuEventBankDown)
			{
				//LOGNOTE("BankDown");
				nBank = m_pMiniDexed->GetSysExFileLoader ()->GetNextBankDown(nBank);
				m_pMiniDexed->SetTGParameter (CMiniDexed::TGParameterVoiceBank, nBank, nTG);
			}
			else
			{
				//LOGNOTE("BankUp");
				nBank = m_pMiniDexed->GetSysExFileLoader ()->GetNextBankUp(nBank);
				m_pMiniDexed->SetTGParameter (CMiniDexed::TGParameterVoiceBank, nBank, nTG);
			}
		}
	}
}
	
void CUIMenu::TGUpDownHandler (TMenuEvent Event)
{
	// This will update the menus to position it for the next TG up or down
	unsigned nTG = 0;
	
	if (m_nToneGenerators <= 1) {
		// Nothing to do if only a single TG
		return;
	}

	// If we're not in the root menu, then see if we are already in a TG menu,
	// then find the current TG number. Otherwise assume TG1 (nTG=0).
	if (m_MenuStackMenu[0] == s_MainMenu && (m_pCurrentMenu == s_TGMenu) || (m_MenuStackMenu[1] == s_TGMenu)) {
		nTG = m_nMenuStackSelection[0];
	}

	assert (nTG < CConfig::AllToneGenerators);
	assert (Event == MenuEventTGDown || Event == MenuEventTGUp);
	if (Event == MenuEventTGDown)
	{
		//LOGNOTE("TGDown");
		if (nTG > 0) {
			nTG--;
		}
	}
	else
	{
		//LOGNOTE("TGUp");
		if (nTG < m_nToneGenerators - 1) {
			nTG++;
		}
	}

	// Set menu to the appropriate TG menu as follows:
	//  Top = Root
	//  Menu [0] = Main
	//  Menu [1] = TG Menu
	m_pParentMenu = s_MainMenu;
	m_pCurrentMenu = s_TGMenu;
	m_nCurrentMenuItem = nTG;
	m_nCurrentSelection = 0;
	m_nCurrentParameter = nTG;
	m_nCurrentMenuDepth = 1;

	// Place the main menu on the stack with Root as the parent
	m_MenuStackParent[0] = s_MenuRoot;
	m_MenuStackMenu[0] = s_MainMenu;
	m_nMenuStackItem[0] = 0;
	m_nMenuStackSelection[0] = nTG;
	m_nMenuStackParameter[0] = 0;

	EventHandler (MenuEventUpdate);
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
	unsigned nLastPerformance = pUIMenu->m_pMiniDexed->GetLastPerformance();
	unsigned nValue = pUIMenu->m_nSelectedPerformanceID;
	unsigned nStart = nValue;
	if (pUIMenu->m_pMiniDexed->IsValidPerformance(nValue) != true)
	{
		// A bank change has left the selected performance out of sync
		nValue = pUIMenu->m_pMiniDexed->GetActualPerformanceID();
		pUIMenu->m_nSelectedPerformanceID = nValue;
	}
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
			do
			{
				if (nValue == 0)
				{
					// Wrap around
					nValue = nLastPerformance;
				}
				else if (nValue > 0)
				{
					--nValue;
				}
			} while ((pUIMenu->m_pMiniDexed->IsValidPerformance(nValue) != true) && (nValue != nStart));
			pUIMenu->m_nSelectedPerformanceID = nValue;
			if (!bPerformanceSelectToLoad && pUIMenu->m_nCurrentParameter==0)
			{
				pUIMenu->m_pMiniDexed->SetNewPerformance(nValue);
			}
			break;

		case MenuEventStepUp:
			do
			{
				if (nValue == nLastPerformance)
				{
					// Wrap around
					nValue = 0;
				}
				else if (nValue < nLastPerformance)
				{
					++nValue;
				}
			} while ((pUIMenu->m_pMiniDexed->IsValidPerformance(nValue) != true) && (nValue != nStart));
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
				if (pUIMenu->m_pMiniDexed->IsValidPerformance(pUIMenu->m_nSelectedPerformanceID))
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
		unsigned nBankNum = pUIMenu->m_pMiniDexed->GetPerformanceBank();
		
		std::string nPSelected = "000";
		nPSelected += std::to_string(nBankNum+1);  // Convert to user-facing bank number rather than index
		nPSelected = nPSelected.substr(nPSelected.length()-3,3);
		std::string nPPerf = "000";
		nPPerf += std::to_string(nValue+1);  // Convert to user-facing performance number rather than index
		nPPerf = nPPerf.substr(nPPerf.length()-3,3);

		nPSelected += ":"+nPPerf;
		if(nValue == pUIMenu->m_pMiniDexed->GetActualPerformanceID())
		{
			nPSelected += " [L]";
		}
					
		pUIMenu->m_pUI->DisplayWrite (pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name, nPSelected.c_str(),
						  Value.c_str (), true, true);
//						 (int) nValue > 0, (int) nValue < (int) pUIMenu->m_pMiniDexed->GetLastPerformance());
	}
	else
	{
		pUIMenu->m_pUI->DisplayWrite ("", "Delete?", pUIMenu->m_bConfirmDeletePerformance ? "Yes" : "No", false, false);
	}
}

void CUIMenu::EditPerformanceBankNumber (CUIMenu *pUIMenu, TMenuEvent Event)
{
	bool bPerformanceSelectToLoad = pUIMenu->m_pMiniDexed->GetPerformanceSelectToLoad();
	unsigned nLastPerformanceBank = pUIMenu->m_pMiniDexed->GetLastPerformanceBank();
	unsigned nValue = pUIMenu->m_nSelectedPerformanceBankID;
	unsigned nStart = nValue;
	std::string Value;

	switch (Event)
	{
	case MenuEventUpdate:
		break;

	case MenuEventStepDown:
		do
		{
			if (nValue == 0)
			{
				// Wrap around
				nValue = nLastPerformanceBank;
			}
			else if (nValue > 0)
			{
				--nValue;
			}
		} while ((pUIMenu->m_pMiniDexed->IsValidPerformanceBank(nValue) != true) && (nValue != nStart));
		pUIMenu->m_nSelectedPerformanceBankID = nValue;
		if (!bPerformanceSelectToLoad)
		{
			// Switch to the new bank and select the first performance voice
			pUIMenu->m_pMiniDexed->SetParameter (CMiniDexed::ParameterPerformanceBank, nValue);
			pUIMenu->m_pMiniDexed->SetFirstPerformance();
		}
		break;

	case MenuEventStepUp:
		do
		{
			if (nValue == nLastPerformanceBank)
			{
				// Wrap around
				nValue = 0;
			}
			else if (nValue < nLastPerformanceBank)
			{
				++nValue;
			}
		} while ((pUIMenu->m_pMiniDexed->IsValidPerformanceBank(nValue) != true) && (nValue != nStart));
		pUIMenu->m_nSelectedPerformanceBankID = nValue;
		if (!bPerformanceSelectToLoad)
		{
			pUIMenu->m_pMiniDexed->SetParameter (CMiniDexed::ParameterPerformanceBank, nValue);
			pUIMenu->m_pMiniDexed->SetFirstPerformance();
		}
		break;

	case MenuEventSelect:	
		if (bPerformanceSelectToLoad)
		{
			pUIMenu->m_pMiniDexed->SetParameter (CMiniDexed::ParameterPerformanceBank, nValue);
			pUIMenu->m_pMiniDexed->SetFirstPerformance();
		}
		break;

	default:
		return;
	}

	Value = pUIMenu->m_pMiniDexed->GetPerformanceConfig ()->GetPerformanceBankName(nValue);
	std::string nPSelected = "000";
	nPSelected += std::to_string(nValue+1);  // Convert to user-facing number rather than index
	nPSelected = nPSelected.substr(nPSelected.length()-3,3);

	if(nValue == (unsigned)pUIMenu->m_pMiniDexed->GetParameter (CMiniDexed::ParameterPerformanceBank))
	{
		nPSelected += " [L]";
	}

	pUIMenu->m_pUI->DisplayWrite (pUIMenu->m_pParentMenu[pUIMenu->m_nCurrentMenuItem].Name, nPSelected.c_str(),
							Value.c_str (),
							nValue > 0,
							nValue < pUIMenu->m_pMiniDexed->GetLastPerformanceBank()-1);
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

CUIMenu::TMenuItem* CUIMenu::getFXMenuItem(unsigned type)
{
	CUIMenu::TMenuItem* menu;
	switch (type)
	{
	case AudioEffects::Types::CHORUS:
		menu = s_FXChorus;
		break;
	case AudioEffects::Types::DELAY:
		menu = s_FXDelay;
		break;
	case AudioEffects::Types::LPF:
		menu = s_FXLPFilter;
		break;
	case AudioEffects::Types::DS1:
		menu = s_FXDS1;
		break;
	case AudioEffects::Types::BIGMUFF:
		menu = s_FXBigMuff;
		break;
	case AudioEffects::Types::TALREVERB3:
		menu = s_FXTalReverb3;
		break;
	case AudioEffects::Types::REVERB:
		menu = s_FXReverb;
		break;
	case AudioEffects::Types::MVERB:
		menu = s_FXMVerb;
		break;
	case AudioEffects::Types::EQ3BAND:
		menu = s_FX3BandEQ;
		break;
	case AudioEffects::Types::PHASER:
		menu = s_FXPhaser;
		break;
	case AudioEffects::Types::APHASER:
		menu = s_FXAPhaser;
		break;
	case AudioEffects::Types::FLANGER:
		menu = s_FXFlanger;
		break;
	case AudioEffects::Types::NONE:
	default:
        menu = s_FXNone;
		break;
	}
	return menu;
}

CUIMenu::TMenuItem* CUIMenu::getInsertFXMenuItem(unsigned type)
{
	CUIMenu::TMenuItem* menu = getFXMenuItem(type);
	
	unsigned i = 0;
	while(menu[i].Name != NULL)
	{
		menu[i].Handler = EditTGFXParameter;
		i++;
	}
	
	return menu;
}

CUIMenu::TMenuItem* CUIMenu::getSendFX1MenuItem(unsigned type)
{
	CUIMenu::TMenuItem* menu = getFXMenuItem(type);
	
	unsigned i = 0;
	while(menu[i].Name != NULL)
	{
		menu[i].Handler = EditSendFX1Parameter;
		i++;
	}
	
	return menu;
}

CUIMenu::TMenuItem* CUIMenu::getSendFX2MenuItem(unsigned type)
{
	CUIMenu::TMenuItem* menu = getFXMenuItem(type);
	
	unsigned i = 0;
	while(menu[i].Name != NULL)
	{
		menu[i].Handler = EditSendFX2Parameter;
		i++;
	}
	
	return menu;
}

CUIMenu::TMenuItem* CUIMenu::getMasterFXMenuItem(unsigned type)
{
	CUIMenu::TMenuItem* menu = getFXMenuItem(type);
	
	unsigned i = 0;
	while(menu[i].Name != NULL)
	{
		menu[i].Handler = EditMasterFXParameter;
		i++;
	}
	
	return menu;
}

CUIMenu::TParameter CUIMenu::getFXParameter(unsigned type, unsigned nParam)
{
	TParameter pParam;
	switch (type)
	{
	case AudioEffects::Types::CHORUS:
		pParam = s_TGFXChorusParam[nParam];
		break;
	case AudioEffects::Types::DELAY:
		pParam = s_TGFXDelayParam[nParam];
		break;
	case AudioEffects::Types::LPF:
		pParam = s_TGFXLPFParam[nParam];
		break;
	case AudioEffects::Types::DS1:
		pParam = s_TGFXDS1Param[nParam];
		break;
	case AudioEffects::Types::BIGMUFF:
		pParam = s_TGFXBigMuffParam[nParam];
		break;
	case AudioEffects::Types::TALREVERB3:
		pParam = s_TGFXTalReverb3Param[nParam];
		break;
	case AudioEffects::Types::REVERB:
		pParam = s_TGFXReverbParam[nParam];
		break;
	case AudioEffects::Types::MVERB:
		pParam = s_TGFXMVerbParam[nParam];
		break;
	case AudioEffects::Types::EQ3BAND:
		pParam = s_TGFX3BandEQParam[nParam];
		break;
	case AudioEffects::Types::PHASER:
		pParam = s_TGFXPhaserParam[nParam];
		break;
	case AudioEffects::Types::APHASER:
		pParam = s_TGFXAPhaserParam[nParam];
		break;
	case AudioEffects::Types::FLANGER:
		pParam = s_TGFXFlangerParam[nParam];
		break;
	default:
		break;
	}
	return pParam;
}

CUIMenu::TMenuItem* CUIMenu::getMidiFXMenuItem(unsigned type)
{
	CUIMenu::TMenuItem* menu;
	switch (type)
	{
	case MidiEffects::Types::ARP:
		menu = s_MidiFXArp;
		break;
	case MidiEffects::Types::NONE:
	default:
        menu = s_MidiFXNone;
		break;
	}
	return menu;
}

CUIMenu::TParameter CUIMenu::getMidiFXParameter(unsigned type, unsigned nParam)
{
	TParameter pParam;
	switch (type)
	{
	case MidiEffects::Types::ARP:
		pParam = s_TGMidiFXArpParam[nParam];
	default:
		break;
	}
	return pParam;
}