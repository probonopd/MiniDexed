// MiniDexed - Dexed FM synthesizer for bare metal Raspberry Pi
// Copyright (C) 2024  The MiniDexed Team
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
#include <circle/string.h>

#include "dawcontroller.h"
#include "midikeyboard.h"
#include "minidexed.h"

#define LINELEN 18

#define MIDI_DAW_CHANGE 0b10000
#define MIDI_DAW_VOICE 1
#define MIDI_DAW_TOGGLE_MONO 3
#define MIDI_DAW_TOGGLE_PORTA_GLISS 4
#define MIDI_DAW_TOGGLE_TG 5
#define MIDI_DAW_SELECT_TG 6
#define MIDI_DAW_SELECT_CHAN_TG 7
#define MIDI_DAW_MENU_SELECT 8
#define MIDI_DAW_MENU_BACK 9
#define MIDI_DAW_MENU_PREV 10
#define MIDI_DAW_MENU_NEXT 11
#define MIDI_DAW_MENU_PRESS_PREV 12
#define MIDI_DAW_MENU_PRESS_NEXT 13
#define MIDI_DAW_MENU_HOME 14
#define MIDI_DAW_DISPLAY_MODE_TOGGLE 17
#define MIDI_DAW_ENC_VALUES_TOGGLE 18
#define MIDI_DAW_ENC_0 20
#define MIDI_DAW_ENC_1 21
#define MIDI_DAW_ENC_2 22
#define MIDI_DAW_ENC_3 23
#define MIDI_DAW_ENC_4 24
#define MIDI_DAW_ENC_5 25
#define MIDI_DAW_ENC_6 26
#define MIDI_DAW_ENC_7 27


static void ArturiaDisplayWrite (CMIDIKeyboard *pKeyboard, const u8 *pHdr, const unsigned nHdrSize,
	size_t nLineMaxLen, const bool bFill1, const bool bFill2, const char *pMenu,
	const char *pParam, const char *pValue,
	const bool bArrowLeft, const bool bArrowRight)
{
	size_t nParamLen = std::min (nLineMaxLen, strlen (pParam));
	size_t nMenuLen = strlen (pMenu);
	size_t nFill1Len = bFill1 && nLineMaxLen > nParamLen + nMenuLen ?
		nLineMaxLen - nParamLen - nMenuLen : 1;

	nFill1Len = std:: min (nLineMaxLen - nParamLen, nFill1Len);
	nMenuLen = std::min (nLineMaxLen - nParamLen - nFill1Len, nMenuLen);

	size_t nLine1Len = nParamLen + nFill1Len + nMenuLen;

	size_t nValueLen = std::min (nLineMaxLen - 2, strlen (pValue));
	size_t nFill2Len = bFill2 ? nLineMaxLen - 2 - nValueLen : 0;
	size_t nLine2Len = 1 + nValueLen + nFill2Len + 1;

	size_t nOffset = 0;

	uint8_t pLines[nHdrSize + nLine1Len + 2 + nLine2Len  + 2];

	memcpy (pLines, pHdr, nHdrSize);
	nOffset += nHdrSize;

	memcpy (&pLines[nOffset], pParam, nParamLen);
	nOffset += nParamLen;

	memset (&pLines[nOffset], ' ', nFill1Len);
	nOffset += nFill1Len;

	memcpy (&pLines[nOffset], pMenu, nMenuLen);
	nOffset += nMenuLen;

	pLines[nOffset++] = 0x00;
	pLines[nOffset++] = 0x02;

	pLines[nOffset++] = bArrowLeft ? '<' : ' ';

	memcpy (&pLines[nOffset], pValue, nValueLen);
	nOffset += nValueLen;

	memset (&pLines[nOffset], ' ', nFill2Len);
	nOffset += nFill2Len;

	pLines[nOffset++] = bArrowRight ? '>' : ' ';

	pLines[nOffset++] = 0x00;
	pLines[nOffset++] = 0xF7;

	// block character (0xFF) is not supported over MIDI, change to 0x7f
	for (unsigned i = 0; i < sizeof pLines; ++i)
		if (pLines[i] == 0xFF)
			pLines[i] = 0x7F;

	pKeyboard->SendDebounce (pLines, nOffset, 0);
}

enum ControlType
{
	CT_KNOB = 3,
	CT_FADER,
	CT_PAD,
};

enum HideAfter
{
	HA_NO = 0,
	HA_YES = 2,
};

static std::string to_percent (int nValue)
{
	return std::to_string(mapfloatr (nValue, 0, 127, 0, 100)) + "%";
}

std::string to_midi_channel (int nValue)
{
	switch (nValue)
	{
	case CMIDIDevice::OmniMode:	return "Omni";
	case CMIDIDevice::Disabled:	return "Off";
	default: return std::to_string (nValue + 1);
	}
}

static void ArturiaDisplayInfoWrite (CMIDIKeyboard *pKeyboard, const uint8_t pDisplayHdr[3], ControlType Type, u8 uValue, const char *pName, const char *pValue)
{
	const uint8_t pHdr[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, pDisplayHdr[0], pDisplayHdr[1], pDisplayHdr[2], 0x1F, Type, HA_NO, uValue, 0x00, 0x00, 0x01};

	int nLine1Len = strlen (pName);
	int nLine2Len = strlen (pValue);
	int nOffset = 0;

	uint8_t pLines[sizeof pHdr + nLine1Len + 2 + nLine2Len + 2];

	memcpy (pLines, pHdr, sizeof pHdr);
	nOffset += sizeof pHdr;

	memcpy (pLines + nOffset, pName, nLine1Len);
	nOffset += nLine1Len;

	pLines[nOffset++] = 0x00;
	pLines[nOffset++] = 0x02;

	memcpy (pLines + nOffset, pValue, nLine2Len);
	nOffset += nLine2Len;

	pLines[nOffset++] = 0x00;
	pLines[nOffset++] = 0xf7;

	pKeyboard->SendDebounce (pLines, nOffset, 0);
}

static void ArturiaShowNewCCValue (CMIDIKeyboard *pKeyboard, const uint8_t pDisplayHdr[3], u8 ucCh, u8 ucCC, u8 ucValue)
{
	char line1[LINELEN];
	char line2[LINELEN];

	switch (ucCC)
	{
	case MIDI_CC_PORTAMENTO_TIME:
		snprintf(line2, LINELEN, "%ld%%", mapfloatr (ucValue, 0, 127, 0, 99));
		ArturiaDisplayInfoWrite (pKeyboard, pDisplayHdr, CT_KNOB, ucValue, "Portamento Time", line2);
		break;
	case MIDI_CC_VOLUME:
		snprintf(line1, LINELEN, "Volume Ch %d", ucCh + 1);
		snprintf(line2, LINELEN, "%ld%%", mapfloatr (ucValue, 0, 127, 0, 100));
		ArturiaDisplayInfoWrite (pKeyboard, pDisplayHdr, CT_FADER, ucValue, line1, line2);
		break;
	case MIDI_CC_FREQUENCY_CUTOFF:
		snprintf(line2, LINELEN, "%ld%%", mapfloatr (ucValue, 0, 127, 0, 99));
		ArturiaDisplayInfoWrite (pKeyboard, pDisplayHdr, CT_KNOB, ucValue, "Cutoff", line2);
		break;
	case MIDI_CC_RESONANCE:
		snprintf(line2, LINELEN, "%ld%%", mapfloatr (ucValue, 0, 127, 0, 99));
		ArturiaDisplayInfoWrite (pKeyboard, pDisplayHdr, CT_KNOB, ucValue, "Resonance", line2);
		break;
	case MIDI_CC_REVERB_LEVEL:
		snprintf(line2, LINELEN, "%ld%%", mapfloatr (ucValue, 0, 127, 0, 99));
		ArturiaDisplayInfoWrite (pKeyboard, pDisplayHdr, CT_KNOB, ucValue, "Reverb", line2);
		break;
	case MIDI_CC_DETUNE_LEVEL:
		snprintf(line2, LINELEN, "%ld", mapfloatr (ucValue, 1, 127, -99, 99));
		ArturiaDisplayInfoWrite (pKeyboard, pDisplayHdr, CT_KNOB, ucValue, "Detune", line2);
		break;
	case MIDI_CC_PAN_POSITION:
		snprintf(line2, LINELEN, "%d", ucValue);
		ArturiaDisplayInfoWrite (pKeyboard, pDisplayHdr, CT_KNOB, ucValue, "Pan", line2);
		break;
	case MIDI_CC_BANK_SUSTAIN:
		ArturiaDisplayInfoWrite (pKeyboard, pDisplayHdr, CT_PAD, ucValue, "Sustain", ucValue > 64 ? "On" : "Off");
		break;
	case MIDI_CC_PORTAMENTO:
		ArturiaDisplayInfoWrite (pKeyboard, pDisplayHdr, CT_PAD, ucValue, "Portamento", ucValue > 64 ? "On" : "Off");
		break;
	case MIDI_CC_SOSTENUTO:
		ArturiaDisplayInfoWrite (pKeyboard, pDisplayHdr, CT_PAD, ucValue, "Sostenuto", ucValue > 64 ? "On" : "Off");
		break;
	case MIDI_CC_HOLD2:
		ArturiaDisplayInfoWrite (pKeyboard, pDisplayHdr, CT_PAD, ucValue, "Hold", ucValue > 64 ? "On" : "Off");
		break;
	case MIDI_CC_ALL_SOUND_OFF:
		ArturiaDisplayInfoWrite (pKeyboard, pDisplayHdr, CT_PAD, ucValue, "All Sound Off", "");
		break;
	}
}

static void HandleMenuEvents (CUserInterface *pUI, u8 ucDC)
{
	switch (ucDC)
	{
	case MIDI_DAW_MENU_SELECT:
		pUI->MIDIEventHandler (CUIMenu::MenuEventSelect);
		break;
	case MIDI_DAW_MENU_BACK:
		pUI->MIDIEventHandler (CUIMenu::MenuEventBack);
		break;
	case MIDI_DAW_MENU_PREV:
		pUI->MIDIEventHandler (CUIMenu::MenuEventStepDown);
		break;
	case MIDI_DAW_MENU_NEXT:
		pUI->MIDIEventHandler (CUIMenu::MenuEventStepUp);
		break;
	case MIDI_DAW_MENU_PRESS_PREV:
		pUI->MIDIEventHandler (CUIMenu::MenuEventPressAndStepDown);
		break;
	case MIDI_DAW_MENU_PRESS_NEXT:
		pUI->MIDIEventHandler (CUIMenu::MenuEventPressAndStepUp);
		break;
	case MIDI_DAW_MENU_HOME:
		pUI->MIDIEventHandler (CUIMenu::MenuEventHome);
		break;
	}
}

class CDAWConnection
{
public:
	virtual void DisplayWrite (const char *pMenu, const char *pParam,
				   const char *pValue, bool bArrowDown, bool bArrowUp) = 0;
	virtual void UpdateState () = 0;
	virtual void UpdateMenu (CUIMenu::TCPageType Type, s8 ucPage, u8 ucOP, u8 ucTG) = 0;
	virtual void MIDIListener (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2) = 0;
	virtual ~CDAWConnection (void) = default;
};

struct CColor
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

static CColor padColors[8] = {
	{0x3F, 0x3F, 0x11},
	{0x11, 0x11, 0x3F},
	{0x3F, 0x11, 0x3F},
	{0x11, 0x3F, 0x11},
	{0x3F, 0x11, 0x11},
	{0x11, 0x3F, 0x3F},
	{0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00},
};

static CColor altPadColors[8] = {
	{0x3F, 0x3F, 0x11},
	{0x11, 0x21, 0x3F},
	{0x3F, 0x11, 0x3F},
	{0x11, 0x3F, 0x11},
	{0x3F, 0x11, 0x11},
	{0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00},
};

static CColor chColors[CMIDIDevice::TChannel::Disabled + 1] = {
	{0x7F, 0x00, 0x00}, // 1
	{0x7F, 0x40, 0x00}, // 2
	{0x7F, 0x40, 0x40}, // 3
	{0x7F, 0x40, 0x7F}, // 4
	{0x7F, 0x7F, 0x00}, // 5
	{0x7F, 0x7F, 0x40}, // 6
	{0x7F, 0x7F, 0x7F}, // 7
	{0x40, 0x00, 0x40}, // 8
	{0x40, 0x40, 0x00}, // 9
	{0x40, 0x40, 0x40}, // 10
	{0x40, 0x40, 0x7F}, // 11
	{0x40, 0x7F, 0x00}, // 12
	{0x40, 0x7F, 0x40}, // 13
	{0x40, 0x7F, 0x7F}, // 14
	{0x00, 0x00, 0x40}, // 15
	{0x00, 0x40, 0x00}, // 16
	{0x7F, 0x7F, 0x7F}, // Omni
	{0x00, 0x00, 0x00}, // Disabled
};

class CMiniLab3DawConnection : public CDAWConnection
{
public:
	CMiniLab3DawConnection (CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard, CConfig *pConfig, CUserInterface *pUI);
	void DisplayWrite (const char *pMenu, const char *pParam, const char *pValue, bool bArrowDown, bool bArrowUp) override;
	void UpdateState () override;
	void UpdateMenu (CUIMenu::TCPageType Type, s8 ucPage, u8 ucOP, u8 ucTG) override;
	void MIDIListener (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2) override;
private:
	enum TPadID {
		MonoPad = 0,
		PortamentoPad = 1,
		SostenutoPad = 2,
		SustainPad = 3,
		SoundOffPad = 4,
		HoldPad = 5,
		TBDPad7 = 6,
		ATPad = 7,
	};
	enum TBankID {
		BankA = 0x34,
		BankB = 0x44,
	};

	static const u8 AllOP = 8;
	static const unsigned nDefaultDisplayUpdateDelay = 2000;

	static void s_UpdateDisplay (TKernelTimerHandle hTimer, void *pParam, void *pContext);
	void QueueUpdateDisplay (unsigned msec);
	void UpdateDisplay ();
	void ShowEncoderDisplay ();
	void ShowValueDisplay ();

	void UpdateEncoder (uint8_t ucEncID, uint8_t ucValue);
	void UpdateTGColor (uint8_t nTG);
	void UpdateMonoColor ();
	void UpdatePortamentoColor ();
	void UpdateATColor (u8 ucAT);
	void UpdateVolumeFaders ();

	void SetPadColor (TBankID BankID, TPadID PadID, u8 state);
	void SetPadColor (TBankID BankID, TPadID PadID, u8 state, u8 state2);
	void SetPadColor (TBankID BankID, TPadID PadID, CColor color, u8 state);
	void SetPadColor (TBankID BankID, TPadID PadID, CColor color);

	void SetChannelAT (u8 ucValue);
	void SetVoice (u8 ucChannel, u8 ucVoice);
	void SetAlgorithm (u8 ucChannel, u8 ucAlgorithm);
	void SetEncoder (u8 ucChannel, u8 ucEncId, u8 ucVoice);
	void ToggleMonoMode (u8 ucChannel);
	void TogglePortamentoGlisssando (u8 ucChannel);
	void ToggleTG (u8 ucTG);
	void SelectTG (u8 ucTG);
	void SelectChanTG (u8 ucTG);

	CMiniDexed *m_pSynthesizer;
	CMIDIKeyboard *m_pKeyboard;
	CConfig *m_pConfig;
	CUserInterface *m_pUI;

	bool m_bDisableEncoderUpdate = false;

	CUIMenu::TCPageType m_encoderPageType = CUIMenu::PageMain;
	u8 m_ucEncoderPage = 0;
	u8 m_ucEncoderOP = 0;
	u8 m_ucEncoderTG = 0;
	CUIMenu::TCParameterInfo *m_pEncoders;

	enum DisplayState {
		DisplayMenu,
		DisplayEncoder,
		DisplayValues,
	};
	DisplayState m_DisplayState = DisplayMenu;

	CUIMenu::TCParameterInfo m_pTGEncoders[4 + 3 + 22][8] = {
		{
			{CUIMenu::ParameterTG, CMiniDexed::TGParameterCutoff},
			{CUIMenu::ParameterTG, CMiniDexed::TGParameterResonance},
			{CUIMenu::ParameterTG, CMiniDexed::TGParameterReverbSend},
			{CUIMenu::ParameterTG, CMiniDexed::TGParameterMasterTune},
			{CUIMenu::ParameterTG, CMiniDexed::TGParameterPortamentoTime},
			{CUIMenu::ParameterTG, CMiniDexed::TGParameterProgram},
			{CUIMenu::ParameterNone},
			{CUIMenu::ParameterTG, CMiniDexed::TGParameterPan, .ToString=std::to_string},
		},
		{
			{CUIMenu::ParameterTG, CMiniDexed::TGParameterMIDIChannel},
			{CUIMenu::ParameterTG, CMiniDexed::TGParameterVolume, .ToString=to_percent},
			{CUIMenu::ParameterNone},
			{CUIMenu::ParameterTG, CMiniDexed::TGParameterPitchBendRange},
			{CUIMenu::ParameterTG, CMiniDexed::TGParameterPortamentoGlissando},
			{CUIMenu::ParameterTG, CMiniDexed::TGParameterMonoMode},
			{CUIMenu::ParameterNone},
			{CUIMenu::ParameterTG, CMiniDexed::TGParameterPitchBendStep},
		},
		{
			{CUIMenu::ParameterTG, CMiniDexed::TGParameterMWRange},
			{CUIMenu::ParameterTG, CMiniDexed::TGParameterMWPitch, "MW Pitch", "MWPi"},
			{CUIMenu::ParameterTG, CMiniDexed::TGParameterFCRange},
			{CUIMenu::ParameterTG, CMiniDexed::TGParameterFCPitch, "FC Pitch", "FCPi"},
			{CUIMenu::ParameterTG, CMiniDexed::TGParameterMWEGBias, "MW EG Bias", "MWEGB"},
			{CUIMenu::ParameterTG, CMiniDexed::TGParameterMWAmplitude, "MW Amp", "MWA"},
			{CUIMenu::ParameterTG, CMiniDexed::TGParameterFCEGBias, "FC EG Bias", "FCEGB"},
			{CUIMenu::ParameterTG, CMiniDexed::TGParameterFCAmplitude, "FC Amp", "FCA"},

		},
		{
			{CUIMenu::ParameterTG, CMiniDexed::TGParameterBCRange},
			{CUIMenu::ParameterTG, CMiniDexed::TGParameterBCPitch, "BC Pitch", "BCPi"},
			{CUIMenu::ParameterTG, CMiniDexed::TGParameterATRange},
			{CUIMenu::ParameterTG, CMiniDexed::TGParameterATPitch, "AT Pitch", "ATPi"},
			{CUIMenu::ParameterTG, CMiniDexed::TGParameterBCEGBias, "BC EG Bias", "BCEGB"},
			{CUIMenu::ParameterTG, CMiniDexed::TGParameterBCAmplitude, "BC Amp", "BCA"},
			{CUIMenu::ParameterTG, CMiniDexed::TGParameterATEGBias, "AT EG Bias", "ATEGB"},
			{CUIMenu::ParameterTG, CMiniDexed::TGParameterATAmplitude, "AT Amp", "ATA"},
		},
	};

	CUIMenu::TCParameterInfo m_pEffectEncoders[1][8] = {
		{
			{CUIMenu::ParameterGlobal, CMiniDexed::ParameterCompressorEnable},
			{CUIMenu::ParameterGlobal, CMiniDexed::ParameterReverbEnable, "Reverb"},
			{CUIMenu::ParameterGlobal, CMiniDexed::ParameterReverbSize, "Rev Size"},
			{CUIMenu::ParameterGlobal, CMiniDexed::ParameterReverbHighDamp, "Rev High Damp"},
			{CUIMenu::ParameterGlobal, CMiniDexed::ParameterReverbLowDamp, "Rev Low Damp"},
			{CUIMenu::ParameterGlobal, CMiniDexed::ParameterReverbLowPass, "Rev Low Pass"},
			{CUIMenu::ParameterGlobal, CMiniDexed::ParameterReverbDiffusion, "Rev Diffusion"},
			{CUIMenu::ParameterGlobal, CMiniDexed::ParameterReverbLevel, "Rev Level"},
		},
	};

	CUIMenu::TCParameterInfo m_pVoiceEncoders[3 + 22][8] = {
		{
			{CUIMenu::ParameterVoice, DEXED_ALGORITHM},
			{CUIMenu::ParameterVoice, DEXED_FEEDBACK},
			{CUIMenu::ParameterVoice, DEXED_TRANSPOSE},
		},
		{
			{CUIMenu::ParameterVoice, DEXED_PITCH_EG_R1},
			{CUIMenu::ParameterVoice, DEXED_PITCH_EG_R2},
			{CUIMenu::ParameterVoice, DEXED_PITCH_EG_R3},
			{CUIMenu::ParameterVoice, DEXED_PITCH_EG_R4},
			{CUIMenu::ParameterVoice, DEXED_PITCH_EG_L1},
			{CUIMenu::ParameterVoice, DEXED_PITCH_EG_L2},
			{CUIMenu::ParameterVoice, DEXED_PITCH_EG_L3},
			{CUIMenu::ParameterVoice, DEXED_PITCH_EG_L4},
		},
		{
			{CUIMenu::ParameterVoice, DEXED_OSC_KEY_SYNC},
			{CUIMenu::ParameterVoice, DEXED_LFO_SPEED},
			{CUIMenu::ParameterVoice, DEXED_LFO_DELAY},
			{CUIMenu::ParameterVoice, DEXED_LFO_PITCH_MOD_DEP},
			{CUIMenu::ParameterVoice, DEXED_LFO_SYNC},
			{CUIMenu::ParameterVoice, DEXED_LFO_WAVE},
			{CUIMenu::ParameterVoice, DEXED_LFO_PITCH_MOD_SENS},
			{CUIMenu::ParameterVoice, DEXED_LFO_AMP_MOD_DEP},
		},
	};

	CUIMenu::TCParameterInfo m_pOPEncoders[3][8] = {
		{
			{CUIMenu::ParameterOP, DEXED_OP_OUTPUT_LEV},
			{CUIMenu::ParameterOP, DEXED_OP_FREQ_COARSE},
			{CUIMenu::ParameterOP, DEXED_OP_FREQ_FINE},
			{CUIMenu::ParameterOP, DEXED_OP_OSC_DETUNE},
			{CUIMenu::ParameterOP, DEXED_OP_OSC_MODE},
			{CUIMenu::ParameterOP, DEXED_OP_ENABLE},
		},
		{
			{CUIMenu::ParameterOP, DEXED_OP_EG_R1},
			{CUIMenu::ParameterOP, DEXED_OP_EG_R2},
			{CUIMenu::ParameterOP, DEXED_OP_EG_R3},
			{CUIMenu::ParameterOP, DEXED_OP_EG_R4},
			{CUIMenu::ParameterOP, DEXED_OP_EG_L1},
			{CUIMenu::ParameterOP, DEXED_OP_EG_L2},
			{CUIMenu::ParameterOP, DEXED_OP_EG_L3},
			{CUIMenu::ParameterOP, DEXED_OP_EG_L4},
		},
		{
			{CUIMenu::ParameterOP, DEXED_OP_LEV_SCL_BRK_PT},
			{CUIMenu::ParameterOP, DEXED_OP_SCL_LEFT_DEPTH},
			{CUIMenu::ParameterOP, DEXED_OP_SCL_RGHT_DEPTH},
			{CUIMenu::ParameterOP, DEXED_OP_AMP_MOD_SENS},
			{CUIMenu::ParameterOP, DEXED_OP_OSC_RATE_SCALE},
			{CUIMenu::ParameterOP, DEXED_OP_SCL_LEFT_CURVE},
			{CUIMenu::ParameterOP, DEXED_OP_SCL_RGHT_CURVE},
			{CUIMenu::ParameterOP, DEXED_OP_KEY_VEL_SENS},
		},
	};

	TKernelTimerHandle m_DisplayTimer = 0;

	u8 m_ucFirstTG = 0;
	
	const uint8_t m_pEncoder[3] = {0x04, 0x02, 0x60};
	TMIDIRoute m_pRouteMap[75] = {
		{0, 0, MIDI_CONTROL_CHANGE, 14, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader1
		{0, 0, MIDI_CONTROL_CHANGE, 15, 0xFF, 1, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader2
		{0, 0, MIDI_CONTROL_CHANGE, 30, 0xFF, 2, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader3
		{0, 0, MIDI_CONTROL_CHANGE, 31, 0xFF, 3, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader4

		{0, 0, MIDI_CONTROL_CHANGE, 118, 0x7F, .ucTimerTarget=2, .usTimerExpire=m_pConfig->GetLongPressTimeout (), .bSkip=true}, // Main knob click
		{0, 0, MIDI_CONTROL_CHANGE, 118, 0x00, 0, MIDI_DAW_CHANGE, MIDI_DAW_MENU_SELECT, 0, .bGroup=true},
		{0, 0, MIDI_CONTROL_CHANGE, 118, 0x00, 0, MIDI_DAW_CHANGE, MIDI_DAW_MENU_BACK, 0, .bGroup=true},
		{0, 0, MIDI_CONTROL_CHANGE, 28, TMIDIRoute::LtCenter, 0, MIDI_DAW_CHANGE, MIDI_DAW_MENU_PRESS_PREV, 0xFF, .bGroup=true, .bGroupHold=true}, // Main knob click + rotate
		{0, 0, MIDI_CONTROL_CHANGE, 28, TMIDIRoute::GtCenter, 0, MIDI_DAW_CHANGE, MIDI_DAW_MENU_PRESS_NEXT, 0xFF, .bGroup=true, .bGroupHold=true},
		
		{0, 0, MIDI_CONTROL_CHANGE, 119, 0x7F, .bSkip=true}, // Shift + Main knob click
		{0, 0, MIDI_CONTROL_CHANGE, 119, 0x00, 0, MIDI_DAW_CHANGE, MIDI_DAW_MENU_HOME, 0},

		{0, 0, MIDI_CONTROL_CHANGE, 28, TMIDIRoute::LtCenter, 0, MIDI_DAW_CHANGE, MIDI_DAW_MENU_PREV, 0xFF}, // Main knob
		{0, 0, MIDI_CONTROL_CHANGE, 28, TMIDIRoute::GtCenter, 0, MIDI_DAW_CHANGE, MIDI_DAW_MENU_NEXT, 0xFF},

		{0, 0, MIDI_CONTROL_CHANGE, 27, 0x7F, .ucTimerTarget=2, .usTimerExpire=m_pConfig->GetLongPressTimeout (), .bSkip=true}, // Shift
		{0, 0, MIDI_CONTROL_CHANGE, 27, 0x00, 0, MIDI_DAW_CHANGE, MIDI_DAW_ENC_VALUES_TOGGLE, 0xFF, .bGroup=true},
		{0, 0, MIDI_CONTROL_CHANGE, 27, 0x00, 0, MIDI_DAW_CHANGE, MIDI_DAW_DISPLAY_MODE_TOGGLE, 0xFF, .bGroup=true},

		{0, 0, MIDI_CONTROL_CHANGE, 86, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_ENC_0, 0xFF}, // Knob1
		{0, 0, MIDI_CONTROL_CHANGE, 87, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_ENC_1, 0xFF}, // Knob2
		{0, 0, MIDI_CONTROL_CHANGE, 89, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_ENC_2, 0xFF}, // Knob3
		{0, 0, MIDI_CONTROL_CHANGE, 90, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_ENC_3, 0xFF}, // Knob4
		{0, 0, MIDI_CONTROL_CHANGE, 110, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_ENC_4, 0xFF}, // Knob5
		{0, 0, MIDI_CONTROL_CHANGE, 111, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_ENC_5, 0xFF}, // Knob6
		{0, 0, MIDI_CONTROL_CHANGE, 116, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_ENC_6, 0xFF}, // Knob7
		{0, 0, MIDI_CONTROL_CHANGE, 117, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_ENC_7, 0xFF}, // Knob8

		{0, 9, MIDI_NOTE_ON, 36, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_TOGGLE_MONO, 0x7F}, // BankA Pad1
		{0, 9, MIDI_NOTE_OFF, 36, 0xFF, .bSkip=true},
	
		{0, 9, MIDI_NOTE_ON, 37, 0xFF, .bSkip=true, .bGroupHead=true}, // BankA Pad2
		{0, 9, MIDI_NOTE_OFF, 37, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_PORTAMENTO, 0x7F, .bToggle=true, .bGroup=true},
		{0, 9, MIDI_AFTERTOUCH, 37, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_TOGGLE_PORTA_GLISS, 0x7F, .bGroup=true},

		{0, 9, MIDI_NOTE_ON, 38, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_SOSTENUTO, 0x7F, .bToggle=true}, // BankA Pad3
		{0, 9, MIDI_NOTE_OFF, 38, 0xFF, .bSkip=true},

		{0, 9, MIDI_NOTE_ON, 39, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_BANK_SUSTAIN, 0x7F, .bToggle=true}, // BankA Pad4
		{0, 9, MIDI_NOTE_OFF, 39, 0xFF, .bSkip=true},

		{0, 9, MIDI_NOTE_ON, 40, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_ALL_SOUND_OFF, 0x7F}, // BankA Pad5
		{0, 9, MIDI_NOTE_OFF, 40, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_ALL_SOUND_OFF, 0x00},

		{0, 9, MIDI_NOTE_ON, 41, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_HOLD2, 0x7F, .bToggle=true}, // BankA Pad6
		{0, 9, MIDI_NOTE_OFF, 41, 0xFF, .bSkip=true},

		{0, 9, MIDI_NOTE_ON, 42, 0xFF, .bSkip=true}, // BankA Pad7
		{0, 9, MIDI_NOTE_OFF, 42, 0xFF, .bSkip=true},

		{0, 9, MIDI_NOTE_ON, 43, 0xFF, .bSkip=true}, // BankA Pad8
		{0, 9, MIDI_NOTE_OFF, 43, 0xFF, 0, MIDI_CHANNEL_AFTERTOUCH, 0x00, 0xFF},
		{0, 9, MIDI_AFTERTOUCH, 43, 0xFF, 0, MIDI_CHANNEL_AFTERTOUCH, TMIDIRoute::P2, 0xFF},

		{0, 9, MIDI_NOTE_ON, 44, 0xFF,  .ucTimerTarget=2, .usTimerExpire=m_pConfig->GetLongPressTimeout (), .bSkip=true}, // BankB Pad1
		{0, 9, MIDI_NOTE_OFF, 44, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_TOGGLE_TG, 0, .bGroup=true},
		{0, 9, MIDI_NOTE_OFF, 44, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_SELECT_CHAN_TG, 0, .bGroup=true},
		{0, 9, MIDI_AFTERTOUCH, 44, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_SELECT_TG, 0, .bGroup=true},

		{0, 9, MIDI_NOTE_ON, 45, 0xFF, .ucTimerTarget=2, .usTimerExpire=m_pConfig->GetLongPressTimeout (), .bSkip=true}, // BankB Pad2
		{0, 9, MIDI_NOTE_OFF, 45, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_TOGGLE_TG, 1, .bGroup=true},
		{0, 9, MIDI_NOTE_OFF, 45, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_SELECT_CHAN_TG, 1, .bGroup=true},
		{0, 9, MIDI_AFTERTOUCH, 45, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_SELECT_TG, 1, .bGroup=true},

		{0, 9, MIDI_NOTE_ON, 46, 0xFF, .ucTimerTarget=2, .usTimerExpire=m_pConfig->GetLongPressTimeout (), .bSkip=true}, // BankB Pad3
		{0, 9, MIDI_NOTE_OFF, 46, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_TOGGLE_TG, 2, .bGroup=true},
		{0, 9, MIDI_NOTE_OFF, 46, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_SELECT_CHAN_TG, 2, .bGroup=true},
		{0, 9, MIDI_AFTERTOUCH, 46, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_SELECT_TG, 2, .bGroup=true},

		{0, 9, MIDI_NOTE_ON, 47, 0xFF, .ucTimerTarget=2, .usTimerExpire=m_pConfig->GetLongPressTimeout (), .bSkip=true}, // BankB Pad4
		{0, 9, MIDI_NOTE_OFF, 47, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_TOGGLE_TG, 3, .bGroup=true},
		{0, 9, MIDI_NOTE_OFF, 47, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_SELECT_CHAN_TG, 3, .bGroup=true},
		{0, 9, MIDI_AFTERTOUCH, 47, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_SELECT_TG, 3, .bGroup=true},

		{0, 9, MIDI_NOTE_ON, 48, 0xFF, .ucTimerTarget=2, .usTimerExpire=m_pConfig->GetLongPressTimeout (), .bSkip=true}, // BankB Pad5
		{0, 9, MIDI_NOTE_OFF, 48, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_TOGGLE_TG, 4, .bGroup=true},
		{0, 9, MIDI_NOTE_OFF, 48, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_SELECT_CHAN_TG, 4, .bGroup=true},
		{0, 9, MIDI_AFTERTOUCH, 48, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_SELECT_TG, 4, .bGroup=true},

		{0, 9, MIDI_NOTE_ON, 49, 0xFF, .ucTimerTarget=2, .usTimerExpire=m_pConfig->GetLongPressTimeout (), .bSkip=true}, // BankB Pad6
		{0, 9, MIDI_NOTE_OFF, 49, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_TOGGLE_TG, 5, .bGroup=true},
		{0, 9, MIDI_NOTE_OFF, 49, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_SELECT_CHAN_TG, 5, .bGroup=true},
		{0, 9, MIDI_AFTERTOUCH, 49, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_SELECT_TG, 5, .bGroup=true},

		{0, 9, MIDI_NOTE_ON, 50, 0xFF, .ucTimerTarget=2, .usTimerExpire=m_pConfig->GetLongPressTimeout (), .bSkip=true}, // BankB Pad7
		{0, 9, MIDI_NOTE_OFF, 50, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_TOGGLE_TG, 6, .bGroup=true},
		{0, 9, MIDI_NOTE_OFF, 50, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_SELECT_CHAN_TG, 6, .bGroup=true},
		{0, 9, MIDI_AFTERTOUCH, 50, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_SELECT_TG, 6, .bGroup=true},

		{0, 9, MIDI_NOTE_ON, 51, 0xFF, .ucTimerTarget=2, .usTimerExpire=m_pConfig->GetLongPressTimeout (), .bSkip=true}, // BankB Pad8
		{0, 9, MIDI_NOTE_OFF, 51, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_TOGGLE_TG, 7, .bGroup=true},
		{0, 9, MIDI_NOTE_OFF, 51, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_SELECT_CHAN_TG, 7, .bGroup=true},
		{0, 9, MIDI_AFTERTOUCH, 51, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_SELECT_TG, 7, .bGroup=true},

		{0xFF}, // Sentinel
	};
};

CMiniLab3DawConnection::CMiniLab3DawConnection (CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard, CConfig *pConfig, CUserInterface *pUI)
	:m_pSynthesizer (pSynthesizer), m_pKeyboard (pKeyboard), m_pConfig (pConfig), m_pUI (pUI)
{
	static const uint8_t pInit[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x02, 0x00, 0x40, 0x6A, 0x21, 0xF7};

	m_pKeyboard->SetRouteMap (m_pRouteMap);

	m_pKeyboard->Send (pInit, sizeof pInit, 0);
	DisplayWrite ("MiniDexed", "", "On MiniLab 3", 0, 0);

	SetPadColor (BankA, MonoPad, 0);
	SetPadColor (BankA, PortamentoPad, 0);
	SetPadColor (BankA, SostenutoPad, 0);
	SetPadColor (BankA, SustainPad, 0);
	SetPadColor (BankA, SoundOffPad, 0);
	SetPadColor (BankA, HoldPad, 0);
	SetPadColor (BankA, TBDPad7, 0);
	UpdateATColor (0);

	for (unsigned vIdx = 3,i = 0; i < ARRAY_LENGTH (m_pOPEncoders); ++i)
		for (unsigned j = 0; j < ARRAY_LENGTH (*m_pOPEncoders); ++j)
		{
			CUIMenu::TCParameterInfo *pInfo = &m_pOPEncoders[i][j];
			if (!pInfo->Type)
				continue;

			for (unsigned k = 0; k < 6; ++k)
				m_pVoiceEncoders[vIdx][k] = {pInfo->Type, pInfo->Parameter, .OP=static_cast<u8>(k+1)};
			
			m_pVoiceEncoders[vIdx][7] = {pInfo->Type, pInfo->Parameter, .OP=AllOP};

			vIdx++;
		}

	assert ((sizeof m_pTGEncoders - sizeof *m_pTGEncoders * 4 ) == sizeof m_pVoiceEncoders);
	memcpy (m_pTGEncoders + 4, m_pVoiceEncoders, sizeof m_pVoiceEncoders);


	for (unsigned i = 0; i < ARRAY_LENGTH (m_pTGEncoders); ++i)
		m_pUI->GetParameterInfos (m_pTGEncoders[i]);
	for (unsigned i = 0; i < ARRAY_LENGTH (m_pEffectEncoders); ++i)
		m_pUI->GetParameterInfos (m_pEffectEncoders[i]);
	for (unsigned i = 0; i < ARRAY_LENGTH (m_pVoiceEncoders); ++i)
		m_pUI->GetParameterInfos (m_pVoiceEncoders[i]);
	for (unsigned i = 0; i < ARRAY_LENGTH (m_pOPEncoders); ++i)
		m_pUI->GetParameterInfos (m_pOPEncoders[i]);

	UpdateMenu (m_encoderPageType, m_ucEncoderPage, m_ucEncoderOP, m_ucEncoderTG);
	QueueUpdateDisplay (nDefaultDisplayUpdateDelay);
}

void CMiniLab3DawConnection::DisplayWrite (const char *pMenu, const char *pParam, const char *pValue, bool bArrowDown, bool bArrowUp)
{
	const uint8_t page = bArrowDown == bArrowUp ? 0x11 : bArrowDown ? 0x10 : 0x00;
	const uint8_t pHdr[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x04, 0x02, 0x60, 0x1f, 0x06, 0x00, 0x00, page, 0x00, 0x11, 0x00, 0x01};
	ArturiaDisplayWrite (m_pKeyboard, pHdr, sizeof pHdr, 18, true, false, pMenu, pParam, pValue, false, false);
}

void CMiniLab3DawConnection::QueueUpdateDisplay (unsigned msec)
{
	if (m_DisplayTimer)
		CTimer::Get ()->CancelKernelTimer (m_DisplayTimer);
	m_DisplayTimer = CTimer::Get ()->StartKernelTimer (MSEC2HZ (msec), s_UpdateDisplay, this, NULL);
}

void CMiniLab3DawConnection::s_UpdateDisplay (TKernelTimerHandle hTimer, void *pParam, void *pContext)
{
	assert (pParam != NULL);
	static_cast<CMiniLab3DawConnection*>(pParam)->UpdateDisplay ();
}

void CMiniLab3DawConnection::UpdateDisplay ()
{
	switch (m_DisplayState)
	{
		case DisplayMenu:
			m_pUI->MIDIEventHandler (CUIMenu::MenuEventUpdate);
		break;
		case DisplayEncoder:
			ShowEncoderDisplay ();
		break;
		case DisplayValues:
			ShowValueDisplay ();
		break;
	}
}

void CMiniLab3DawConnection::ShowEncoderDisplay ()
{
	static const uint8_t pHdr[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x04, 0x02, 0x60, 0x1f, 0x07, 0x01, 0x00, 0x00, 0x01, 0x00, 0x01};
	std::string pParam = "";
	std::string pValue = "";
	for (unsigned i = 0; i < 8; ++i)
	{
		const char *pShort = m_pEncoders[i].Short ?: "...";
		if (i < 4)
			pParam = pParam + pShort + " ";
		else
			pValue = pValue + pShort + " ";
	}

	ArturiaDisplayWrite (m_pKeyboard, pHdr, sizeof pHdr, 18, false, false, "", pParam.c_str(), pValue.c_str(), false, false);
}

static uint8_t GetParameterValue (CMiniDexed *pSynthesizer, CUIMenu::TCParameterInfo *pInfo, uint8_t ucOP, uint8_t ucTG)
{
	switch (pInfo->Type)
	{
	case CUIMenu::ParameterGlobal:
		return pSynthesizer->GetParameter (static_cast<CMiniDexed::TParameter>(pInfo->Parameter));
		break;
	case CUIMenu::ParameterTG:
		return pSynthesizer->GetTGParameter (static_cast<CMiniDexed::TTGParameter>(pInfo->Parameter), ucTG);
		break;
	case CUIMenu::ParameterVoice:
		return pSynthesizer->GetVoiceParameter (pInfo->Parameter, CMiniDexed::NoOP, ucTG);
		break;
	case CUIMenu::ParameterOP:
		return pSynthesizer->GetVoiceParameter (pInfo->Parameter, ucOP, ucTG);
		break;
	default:
		return 0;
		break;
	}
}

static std::string GetParameterValueStr (CMiniDexed *pSynthesizer, CUIMenu::TCParameterInfo *pInfo, uint8_t ucOP, uint8_t ucTG)
{
	if (pInfo->Type == CUIMenu::ParameterNone)
		return "...";
	uint8_t value = GetParameterValue (pSynthesizer, pInfo, ucOP, ucTG);
	if (pInfo->ToString)
		return pInfo->ToString (value);
	return std::to_string (value);
}


void CMiniLab3DawConnection::ShowValueDisplay ()
{
	static const uint8_t pHdr[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x04, 0x02, 0x60, 0x1f, 0x07, 0x01, 0x02, 0x02, 0x01, 0x00, 0x01};
	std::string pParam = "";
	std::string pValue = "";

	for (unsigned i = 0; i < 8; ++i)
	{
		CUIMenu::TCParameterInfo *enc = &m_pEncoders[i];
		uint8_t ucOP = enc->OP ? enc->OP - 1 : m_ucEncoderOP;
		if (enc->OP == AllOP)
			ucOP = 0;
		uint8_t ucTG = m_ucEncoderTG ? m_ucEncoderTG - 1 : m_ucFirstTG;

		std::string sValue = GetParameterValueStr (m_pSynthesizer, enc, ucOP, ucTG);

		if (i < 4)
			pParam = pParam + sValue + " ";
		else
			pValue = pValue + sValue + " ";
	}

	ArturiaDisplayWrite (m_pKeyboard, pHdr, sizeof pHdr, 18, false, false, "", pParam.c_str(), pValue.c_str(), false, false);
}

void CMiniLab3DawConnection::SetPadColor (TBankID BankID, TPadID PadID, u8 state)
{
	SetPadColor (BankID, PadID, padColors[PadID], state);
}

void CMiniLab3DawConnection::SetPadColor (TBankID BankID, TPadID PadID, u8 state, u8 state2)
{
	SetPadColor (BankID, PadID, state2 ? altPadColors[PadID] : padColors[PadID], state);
}

void CMiniLab3DawConnection::SetPadColor (TBankID BankID, TPadID PadID, CColor color, u8 state)
{
	if (state == 0)
	{
		color.r /= 32;
		color.g /= 32;
		color.b /= 32;
	}
	SetPadColor (BankID, PadID, color);
}

void CMiniLab3DawConnection::SetPadColor (TBankID BankID, TPadID PadID, CColor color)
{
	const uint8_t pSetPadColor[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x02, 0x02, 0x16, (uint8_t)(PadID + BankID), color.r, color.g, color.b, 0xF7};
	m_pKeyboard->Send (pSetPadColor, sizeof pSetPadColor, 0);
}

void CMiniLab3DawConnection::UpdateEncoder (uint8_t ucEncID, uint8_t ucValue)
{
	uint8_t pUpdateEncoder[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x21, 0x10, 0x00, ucEncID+=7, 0x00, ucValue, 0xF7};
	m_pKeyboard->Send (pUpdateEncoder, sizeof pUpdateEncoder, 0);
} 

void CMiniLab3DawConnection::UpdateTGColor (uint8_t nTG)
{
	u8 ch = m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterMIDIChannel, nTG);
	u8 enabled = m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterEnabled, nTG);
	SetPadColor (BankB, (TPadID)nTG, chColors[ch], enabled);
}

void CMiniLab3DawConnection::UpdateMonoColor ()
{
	SetPadColor (BankA, MonoPad, m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterMonoMode, m_ucFirstTG));
}

void CMiniLab3DawConnection::UpdatePortamentoColor ()
{
	u8 mode = m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterPortamentoMode, m_ucFirstTG);
	u8 mode2 = m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterPortamentoGlissando, m_ucFirstTG);
	SetPadColor (BankA, PortamentoPad, mode, mode2);
}

void CMiniLab3DawConnection::UpdateATColor (u8 ucAT)
{
	u8 c = ucAT ?: 1;
	SetPadColor (BankA, ATPad, CColor {c, c, c});
}


void CMiniLab3DawConnection::UpdateVolumeFaders ()
{
	u8 chan_map[4] = {
		CMIDIDevice::TChannel::Disabled,
		CMIDIDevice::TChannel::Disabled,
		CMIDIDevice::TChannel::Disabled,
		CMIDIDevice::TChannel::Disabled,
	};

	for (unsigned i = 0; i < m_pConfig->GetToneGenerators(); ++i)
	{
		int channel = m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterMIDIChannel, i);

		if (channel == CMIDIDevice::ChannelUnknown || channel == CMIDIDevice::Disabled)
			continue;

		if (channel == CMIDIDevice::OmniMode)
			channel = 0;

		for (unsigned i = 0; i < sizeof chan_map; ++i)
		{
			if (chan_map[i] == channel)
				break;

			if (chan_map[i] == CMIDIDevice::Disabled) {
				chan_map[i] = channel;
				break;
			}
		}
	}

	for (unsigned i = 0; i < sizeof chan_map; ++i)
	{
		if (chan_map[i] == CMIDIDevice::Disabled)
			m_pRouteMap[i].bSkip = true;
		else {
			m_pRouteMap[i].bSkip = false;
			m_pRouteMap[i].ucDCh = chan_map[i];
		}
	}
}

void CMiniLab3DawConnection::UpdateState ()
{
	for (unsigned i = 0; i < m_pConfig->GetToneGenerators(); ++i)
		if (m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterEnabled, i)) {
			u8 ucChannel = m_pKeyboard->GetChannel (i);

			if (ucChannel == CMIDIDevice::ChannelUnknown || ucChannel == CMIDIDevice::Disabled)
				continue;

			if (ucChannel == CMIDIDevice::OmniMode)
				ucChannel = 0;

			for (TMIDIRoute *r = m_pRouteMap; r->ucSCable != 0xFF; r++)
					r->ucDCh = ucChannel;

			m_ucFirstTG = i;

			break;
		}

	for (unsigned i = 0; i < 8; ++i)
	{
		if (m_bDisableEncoderUpdate)
			continue;

		CUIMenu::TCParameterInfo *enc = &m_pEncoders[i];
		u8 ucTG = m_ucEncoderTG ? m_ucEncoderTG - 1 :  m_ucFirstTG;
		u8 ucOP = enc->OP ? enc->OP - 1 : m_ucEncoderOP;
		if (enc->OP == AllOP)
			ucOP = 0;
		int value;

		switch (enc->Type)
		{
		case CUIMenu::ParameterGlobal:
			value = m_pSynthesizer->GetParameter (static_cast<CMiniDexed::TParameter>(enc->Parameter));
			break;
		case CUIMenu::ParameterTG:
			value = m_pSynthesizer->GetTGParameter (static_cast<CMiniDexed::TTGParameter>(enc->Parameter), ucTG);
			break;
		case CUIMenu::ParameterVoice:
			value = m_pSynthesizer->GetVoiceParameter (enc->Parameter, CMiniDexed::NoOP, ucTG);
			break;
		case CUIMenu::ParameterOP:
			value = m_pSynthesizer->GetVoiceParameter (enc->Parameter, ucOP, ucTG);
			break;
		default:
			continue;
		}
		UpdateEncoder (i, mapfloatr (value, enc->Min, enc->Max, 0, 127));
	}

	UpdateMonoColor ();
	// TODO change the MIDIRouteMap's value also
	UpdatePortamentoColor ();

	for (unsigned i = 0; i < m_pConfig->GetToneGenerators(); ++i)
	{
		if (m_bDisableEncoderUpdate)
			continue;

		UpdateTGColor (i);
	}

	UpdateVolumeFaders ();
}

void CMiniLab3DawConnection::UpdateMenu (CUIMenu::TCPageType Type, s8 ucPage, u8 ucOP, u8 ucTG)
{
	m_encoderPageType = Type;
	m_ucEncoderOP = ucOP;
	m_ucEncoderTG = ucTG;

	switch (Type)
	{
	case CUIMenu::PageMain:
		m_ucEncoderPage = constrain (ucPage, 0, (s8)ARRAY_LENGTH (m_pTGEncoders) - 1);
		m_pEncoders = m_pTGEncoders[m_ucEncoderPage];
		m_ucEncoderTG = 0; // 0 -> first active TG
		break;
	case CUIMenu::PageTG:
		m_ucEncoderPage = constrain (ucPage, 0, (s8)ARRAY_LENGTH (m_pTGEncoders) - 1);
		m_pEncoders = m_pTGEncoders[m_ucEncoderPage];
		break;
	case CUIMenu::PageEffect:
		m_ucEncoderPage = constrain (ucPage, 0, (s8)ARRAY_LENGTH (m_pEffectEncoders) - 1);
		m_pEncoders = m_pEffectEncoders[m_ucEncoderPage];
		break;
	case CUIMenu::PageVoice:
		m_ucEncoderPage = constrain (ucPage, 0, (s8)ARRAY_LENGTH (m_pVoiceEncoders) - 1);
		m_pEncoders = m_pVoiceEncoders[m_ucEncoderPage];
		break;
	case CUIMenu::PageOP:
		m_ucEncoderPage = constrain (ucPage, 0, (s8)ARRAY_LENGTH (m_pOPEncoders) - 1);
		m_pEncoders = m_pOPEncoders[m_ucEncoderPage];
		break;
	default:
		assert (false);
	}

	UpdateState ();
}

void CMiniLab3DawConnection::MIDIListener (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2)
{
	unsigned nDisplayUpdateDelay = nDefaultDisplayUpdateDelay;
	switch (ucType)
	{
	case MIDI_CONTROL_CHANGE:
		ArturiaShowNewCCValue (m_pKeyboard, m_pEncoder, ucChannel, ucP1, ucP2);

		switch (ucP1)
		{
		case MIDI_CC_PORTAMENTO:
			UpdatePortamentoColor ();
			break;
		case MIDI_CC_SOSTENUTO:
			SetPadColor (BankA, SostenutoPad, ucP2);
			break;
		case MIDI_CC_HOLD2:
			SetPadColor (BankA, HoldPad, ucP2);
			break;
		case MIDI_CC_BANK_SUSTAIN:
			SetPadColor (BankA, SustainPad, ucP2);
			break;
		case MIDI_CC_ALL_SOUND_OFF:
			SetPadColor (BankA, SoundOffPad, ucP2);
			break;
		}
		break;
	case MIDI_DAW_CHANGE:
		switch (m_DisplayState)
		{
		case DisplayMenu:
			HandleMenuEvents (m_pUI, ucP1);
			break;
		case DisplayEncoder:
			switch (ucP1)
			{
			case MIDI_DAW_MENU_PREV:
				UpdateMenu (m_encoderPageType, m_ucEncoderPage - 1, m_ucEncoderOP, m_ucEncoderTG);
				ShowEncoderDisplay ();
				break;
			case MIDI_DAW_MENU_NEXT:
				UpdateMenu (m_encoderPageType, m_ucEncoderPage + 1, m_ucEncoderOP, m_ucEncoderTG);
				ShowEncoderDisplay ();
				break;
			case MIDI_DAW_ENC_VALUES_TOGGLE:
				m_DisplayState = DisplayValues;
				UpdateDisplay ();
				break;
			}
			break;
		case DisplayValues:
			switch (ucP1)
			{
			case MIDI_DAW_MENU_PREV:
				UpdateMenu (m_encoderPageType, m_ucEncoderPage - 1, m_ucEncoderOP, m_ucEncoderTG);
				ShowEncoderDisplay ();
				nDisplayUpdateDelay = 500;
				break;
			case MIDI_DAW_MENU_NEXT:
				UpdateMenu (m_encoderPageType, m_ucEncoderPage + 1, m_ucEncoderOP, m_ucEncoderTG);
				ShowEncoderDisplay ();
				nDisplayUpdateDelay = 500;
				break;
			case MIDI_DAW_ENC_VALUES_TOGGLE:
				m_DisplayState = DisplayEncoder;
				UpdateDisplay ();
				break;
			}
			break;
		}

		switch (ucP1)
		{
		case MIDI_DAW_VOICE:
			SetVoice (ucChannel, ucP2);
			break;
		case MIDI_DAW_TOGGLE_MONO:
			ToggleMonoMode (ucChannel);
			break;
		case MIDI_DAW_TOGGLE_PORTA_GLISS:
			TogglePortamentoGlisssando (ucChannel);
			break;
		case MIDI_DAW_TOGGLE_TG:
			ToggleTG (ucP2);
			break;
		case MIDI_DAW_SELECT_TG:
			SelectTG (ucP2);
			break;
		case MIDI_DAW_SELECT_CHAN_TG:
			SelectChanTG (ucP2);
			break;
		case MIDI_DAW_DISPLAY_MODE_TOGGLE:
			m_DisplayState = m_DisplayState != DisplayMenu ? DisplayMenu : DisplayEncoder;
			UpdateDisplay ();
			break;
		case MIDI_DAW_ENC_0:
		case MIDI_DAW_ENC_1:
		case MIDI_DAW_ENC_2:
		case MIDI_DAW_ENC_3:
		case MIDI_DAW_ENC_4:
		case MIDI_DAW_ENC_5:
		case MIDI_DAW_ENC_6:
		case MIDI_DAW_ENC_7:
			SetEncoder (ucChannel, ucP1 - MIDI_DAW_ENC_0, ucP2);
			break;
		}
		break;
	case MIDI_CHANNEL_AFTERTOUCH:
		SetChannelAT (ucP1);
		break;
	}
	QueueUpdateDisplay (nDisplayUpdateDelay);
}

void CMiniLab3DawConnection::SetChannelAT (u8 ucValue)
{
	char line2[LINELEN];
	snprintf(line2, LINELEN, "%d", ucValue);

	ArturiaDisplayInfoWrite (m_pKeyboard, m_pEncoder, CT_PAD, ucValue, "Channel AT", line2);
	
	UpdateATColor (ucValue);
}

void CMiniLab3DawConnection::SetVoice (u8 ucChannel, u8 ucVoice)
{
	std::string line2;
	for (unsigned i = 0; i < m_pConfig->GetToneGenerators(); ++i)
	{
		if (m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterEnabled, i) == 0 ||
			(m_pKeyboard->GetChannel (i) != ucChannel && m_pKeyboard->GetChannel (i) != CMIDIDevice::OmniMode))
			continue;
		m_pSynthesizer->ProgramChange (ucVoice, i);
		if (line2.length() == 0) {
			std::string sVoiceName = m_pSynthesizer->GetVoiceName (i);
			if (sVoiceName.length() > 0)
				line2 = std::to_string (ucVoice + 1) + "=" + sVoiceName;
		}
	}
	
	ArturiaDisplayInfoWrite (m_pKeyboard, m_pEncoder, CT_KNOB, ucVoice, "Voice", line2.c_str());
}

void CMiniLab3DawConnection::SetEncoder (u8 ucChannel, u8 ucEncId, u8 ucValue)
{
	char line2[LINELEN];

	CUIMenu::TCParameterInfo *encoder = &m_pEncoders[ucEncId];

	if (encoder->Type == CUIMenu::ParameterNone)
		return;

	int value = mapfloatr (ucValue, 0, 127, encoder->Min, encoder->Max);
	u8 ucOP = encoder->OP ? encoder->OP - 1 : m_ucEncoderOP;

	// If we update the encoders during setup, we will get rounding problems, so disable it.
	m_bDisableEncoderUpdate = true;

	if (encoder->Type == CUIMenu::ParameterGlobal)
		m_pSynthesizer->SetParameter (static_cast<CMiniDexed::TParameter>(encoder->Parameter), value);
	else
		for (unsigned i = 0; i < m_pConfig->GetToneGenerators(); ++i)
		{
			if (m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterEnabled, i) == 0)
				continue;

			if (m_ucEncoderTG && m_ucEncoderTG - 1u != i)
				continue;

			if (m_ucEncoderTG == 0 &&
				(m_pKeyboard->GetChannel (i) != ucChannel && m_pKeyboard->GetChannel (i) != CMIDIDevice::OmniMode))
				continue;

			switch (encoder->Type)
			{
			case CUIMenu::ParameterTG:
				m_pSynthesizer->SetTGParameter (static_cast<CMiniDexed::TTGParameter>(encoder->Parameter), value, i);
				break;
			case CUIMenu::ParameterVoice:
				m_pSynthesizer->SetVoiceParameter (encoder->Parameter, value, CMiniDexed::NoOP, i);
				break;
			case CUIMenu::ParameterOP:
				for (unsigned j = 0; j < 6; ++j)
				{
					if (encoder->OP != AllOP && j != ucOP)
						continue;

					m_pSynthesizer->SetVoiceParameter (encoder->Parameter, value, j, i);
				}
				break;
			default:
				break;
			}
		}
	
	m_bDisableEncoderUpdate = false;

	if (encoder->ToString) {
		std::string sValue = encoder->ToString(value);
		snprintf(line2, LINELEN, "%s", sValue.c_str());
	}
	else
		snprintf(line2, LINELEN, "%d", value);

	ArturiaDisplayInfoWrite (m_pKeyboard, m_pEncoder, CT_KNOB, ucValue, encoder->Name, line2);
}

void CMiniLab3DawConnection::ToggleMonoMode (u8 ucChannel)
{
	u8 ucValue = m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterMonoMode, m_ucFirstTG) ? 0x00 : 0x7F;

	for (unsigned i = 0; i < m_pConfig->GetToneGenerators(); ++i)
	{
		if (m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterEnabled, i) == 0 ||
			(m_pKeyboard->GetChannel (i) != ucChannel && m_pKeyboard->GetChannel (i) != CMIDIDevice::OmniMode))
			continue;
		m_pSynthesizer->setMonoMode (ucValue, i);
	}

	ArturiaDisplayInfoWrite (m_pKeyboard, m_pEncoder, CT_PAD, ucValue, "Mono Mode", ucValue > 64 ? "On" : "Off");
	UpdateMonoColor ();
}

void CMiniLab3DawConnection::TogglePortamentoGlisssando (u8 ucChannel)
{
	u8 ucValue = m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterPortamentoGlissando, m_ucFirstTG) ? 0x00 : 0x7F;

	for (unsigned i = 0; i < m_pConfig->GetToneGenerators(); ++i)
	{
		if (m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterEnabled, i) == 0 &&
			(m_pKeyboard->GetChannel (i) != ucChannel && m_pKeyboard->GetChannel (i) != CMIDIDevice::OmniMode))
			continue;
		m_pSynthesizer->setPortamentoGlissando (ucValue, i);
	}

	ArturiaDisplayInfoWrite (m_pKeyboard, m_pEncoder, CT_PAD, ucValue, "Porta Gliss", ucValue > 64 ? "On" : "Off");
	UpdatePortamentoColor ();
}

void CMiniLab3DawConnection::ToggleTG (u8 ucTG)
{
	char line1[LINELEN];

	u8 value = m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterEnabled, ucTG) ? 0x00 : 0x7F;

	m_pSynthesizer->setEnabled (value, ucTG);
	m_pSynthesizer->panic (value, ucTG);

	snprintf(line1, LINELEN, "TG %d", ucTG + 1);
	ArturiaDisplayInfoWrite (m_pKeyboard, m_pEncoder, CT_PAD, value, line1, value > 64 ? "On" : "Off");
}

void CMiniLab3DawConnection::SelectTG (u8 ucTG)
{
	char line1[LINELEN];

	u8 enabledOne = true;
	for (unsigned i = 0; i < m_pConfig->GetToneGenerators(); ++i)
	{
		if (i == ucTG)
			continue;

		if (m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterEnabled, i)) {
			enabledOne = false;
			break;
		}
	}

	if (enabledOne) {
		for (unsigned i = 0; i < m_pConfig->GetToneGenerators(); ++i) {
			m_pSynthesizer->setEnabled (true, i);
		}
		ArturiaDisplayInfoWrite (m_pKeyboard, m_pEncoder, CT_PAD, 0x7F, "TG All", "On");
	} else {
		for (unsigned i = 0; i < m_pConfig->GetToneGenerators(); ++i) {
			if (i == ucTG) {
				m_pSynthesizer->setEnabled (true, i);
			} else {
				m_pSynthesizer->setEnabled (false, i);
				m_pSynthesizer->panic (false, i);
			}
		}
		snprintf(line1, LINELEN, "TG %d", ucTG + 1);
		ArturiaDisplayInfoWrite (m_pKeyboard, m_pEncoder, CT_PAD, 0x7F, line1, "Selected");
	}
}


void CMiniLab3DawConnection::SelectChanTG (u8 ucTG)
{
	char line1[LINELEN];

	u8 enabled = m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterEnabled, ucTG);
	u8 channel = m_pKeyboard->GetChannel (ucTG);

	for (unsigned i = 0; i < m_pConfig->GetToneGenerators(); ++i) {
		if (m_pKeyboard->GetChannel (i) == channel) {
			if (enabled) {
				m_pSynthesizer->setEnabled (false, i);
				m_pSynthesizer->panic (false, i);
			} else {
				m_pSynthesizer->setEnabled (true, i);
			}
		}
	}

	snprintf(line1, LINELEN, "TGs on Ch %s", to_midi_channel(channel).c_str());

	// this doesn't work well with Minilab 3 firmware 1.2.0
	ArturiaDisplayInfoWrite (m_pKeyboard, m_pEncoder, CT_PAD, 0x7F, line1, enabled ? "Off" : "On");
}

class CKeyLabEs3DawConnection : public CDAWConnection
{
public:
	CKeyLabEs3DawConnection (CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard, CConfig *pConfig, CUserInterface *pUI);
	void DisplayWrite ( const char *pMenu, const char *pParam, const char *pValue, bool bArrowDown, bool bArrowUp) override;
	void UpdateState () override;
	void UpdateMenu (CUIMenu::TCPageType Type, s8 ucPage, u8 ucOP, u8 ucTG) override;
	void MIDIListener (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2) override;
private:
	void UpdateEncoder (uint8_t ucEncID, uint8_t ucValue);

	CMiniDexed *m_pSynthesizer;
	CMIDIKeyboard *m_pKeyboard;
	CConfig *m_pConfig;
	CUserInterface *m_pUI;

	const uint8_t m_pEncoder[3] = {0x04, 0x01, 0x60};
	TMIDIRoute m_pRouteMap[25] = {
		{0, 0, MIDI_CONTROL_CHANGE, 117, 0x7F, .ucTimerTarget=2, .usTimerExpire=m_pConfig->GetLongPressTimeout (), .bSkip=true}, // Main knob click
		{0, 0, MIDI_CONTROL_CHANGE, 117, 0x00, 0, MIDI_DAW_CHANGE, MIDI_DAW_MENU_SELECT, 0, .bGroup=true},
		{0, 0, MIDI_CONTROL_CHANGE, 117, 0x00, 0, MIDI_DAW_CHANGE, MIDI_DAW_MENU_BACK, 0, .bGroup=true},
		{0, 0, MIDI_CONTROL_CHANGE, 116, TMIDIRoute::LtCenter, 0, MIDI_DAW_CHANGE, MIDI_DAW_MENU_PRESS_PREV, 0xFF, .bGroup=true, .bGroupHold=true}, // Main knob click + rotate
		{0, 0, MIDI_CONTROL_CHANGE, 116, TMIDIRoute::GtCenter, 0, MIDI_DAW_CHANGE, MIDI_DAW_MENU_PRESS_NEXT, 0xFF, .bGroup=true, .bGroupHold=true},

		{0, 0, MIDI_CONTROL_CHANGE, 44, 0x7F, .bSkip=true}, // Home
		{0, 0, MIDI_CONTROL_CHANGE, 44, 0x00, 0, MIDI_DAW_CHANGE, MIDI_DAW_MENU_HOME, 0},

		{0, 0, MIDI_CONTROL_CHANGE, 116, TMIDIRoute::LtCenter, 0, MIDI_DAW_CHANGE, MIDI_DAW_MENU_PREV, 0xFF}, // Main knob
		{0, 0, MIDI_CONTROL_CHANGE, 116, TMIDIRoute::GtCenter, 0, MIDI_DAW_CHANGE, MIDI_DAW_MENU_NEXT, 0xFF},

		{0, 0, MIDI_CONTROL_CHANGE, 105, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader1
		{0, 0, MIDI_CONTROL_CHANGE, 106, 0xFF, 1, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader2
		{0, 0, MIDI_CONTROL_CHANGE, 107, 0xFF, 2, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader3
		{0, 0, MIDI_CONTROL_CHANGE, 108, 0xFF, 3, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader4
		{0, 0, MIDI_CONTROL_CHANGE, 109, 0xFF, 4, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader5
		{0, 0, MIDI_CONTROL_CHANGE, 110, 0xFF, 5, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader6
		{0, 0, MIDI_CONTROL_CHANGE, 111, 0xFF, 6, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader7
		{0, 0, MIDI_CONTROL_CHANGE, 112, 0xFF, 7, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader8
		{0, 0, MIDI_CONTROL_CHANGE, 113, 0xFF, 8, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader9

		{0, 0, MIDI_CONTROL_CHANGE, 96, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_FREQUENCY_CUTOFF, 0xFF}, // Knob1
		{0, 0, MIDI_CONTROL_CHANGE, 97, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_RESONANCE, 0xFF}, // Knob2
		{0, 0, MIDI_CONTROL_CHANGE, 98, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_REVERB_LEVEL, 0xFF}, // Knob3
		{0, 0, MIDI_CONTROL_CHANGE, 99, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_DETUNE_LEVEL, 0xFF}, // Knob4
		{0, 0, MIDI_CONTROL_CHANGE, 100, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_PAN_POSITION, 0xFF}, // Knob5
		{0, 0, MIDI_CONTROL_CHANGE, 101, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_PORTAMENTO_TIME, 0xFF}, // Knob6
		// {0, 0, MIDI_CONTROL_CHANGE, 102, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_DETUNE_LEVEL, 0xFF}, // Knob7
		// {0, 0, MIDI_CONTROL_CHANGE, 103, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_PAN_POSITION, 0xFF}, // Knob8
		// {0, 0, MIDI_CONTROL_CHANGE, 104, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_PAN_POSITION, 0xFF}, // Knob9
		{0xFF}, // Sentinel
	};
};

CKeyLabEs3DawConnection::CKeyLabEs3DawConnection (CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard, CConfig *pConfig, CUserInterface *pUI)
	:m_pSynthesizer (pSynthesizer), m_pKeyboard (pKeyboard), m_pConfig (pConfig), m_pUI (pUI)
{
	static const uint8_t pInit[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x02, 0x00, 0x40, 0x6A, 0x21, 0xF7};

	m_pKeyboard->SetRouteMap (m_pRouteMap);

	m_pKeyboard->Send (pInit, sizeof pInit, 0);
	DisplayWrite ("MiniDexed", "", "On KeyLab 3 Essential", 0, 0);

	UpdateState ();
}

void CKeyLabEs3DawConnection::DisplayWrite (const char *pMenu, const char *pParam, const char *pValue, bool bArrowDown, bool bArrowUp)
{
	static const uint8_t pHdr[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x04, 0x01, 0x60, 0x12, 0x01};
	ArturiaDisplayWrite (m_pKeyboard, pHdr, sizeof pHdr, 18, true, true, pMenu, pParam, pValue, bArrowDown, bArrowUp);
}

void CKeyLabEs3DawConnection::UpdateEncoder (uint8_t ucEncID, uint8_t ucValue)
{
	uint8_t pUpdateEncoder[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x02, 0x0F, 0x40, ucEncID += 3, ucValue, 0xF7};
	m_pKeyboard->Send (pUpdateEncoder, sizeof pUpdateEncoder, 0);
} 

void CKeyLabEs3DawConnection::UpdateState ()
{
	UpdateEncoder (0, mapfloatr (m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterCutoff, 0), 0, 99, 0, 127));
	UpdateEncoder (1, mapfloatr (m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterResonance, 0), 0, 99, 0, 127));
	UpdateEncoder (2, mapfloatr (m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterReverbSend, 0), 0, 99, 0, 127));
	UpdateEncoder (3, mapfloatr (m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterMasterTune, 0), -99, 99, 1, 127));
	UpdateEncoder (4, m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterPan, 0));
	UpdateEncoder (5, mapfloatr (m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterPortamentoTime, 0), 0, 99, 0, 127));
}

void CKeyLabEs3DawConnection::UpdateMenu (CUIMenu::TCPageType Type, s8 ucPage, u8 ucOP, u8 ucTG)
{

}

void CKeyLabEs3DawConnection::MIDIListener (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2)
{
	switch (ucType)
	{
	case MIDI_CONTROL_CHANGE:
		ArturiaShowNewCCValue (m_pKeyboard, m_pEncoder, ucChannel, ucP1, ucP2);
		break;
	case MIDI_DAW_CHANGE:
		HandleMenuEvents (m_pUI, ucP1);
		break;
	}
}

class CKeyLab2DawConnection : public CDAWConnection
{
public:
	CKeyLab2DawConnection (CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard, CConfig *pConfig, CUserInterface *pUI);
	void DisplayWrite (const char *pMenu, const char *pParam, const char *pValue, bool bArrowDown, bool bArrowUp) override;
	void UpdateState () override;
	void UpdateMenu (CUIMenu::TCPageType Type, s8 ucPage, u8 ucOP, u8 ucTG) override;
	void MIDIListener (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2) override;
private:
	CMiniDexed *m_pSynthesizer;
	CMIDIKeyboard *m_pKeyboard;
	CConfig *m_pConfig;
	CUserInterface *m_pUI;

	TMIDIRoute m_pRouteMap[18] = {
		{1, 0, MIDI_NOTE_ON, 0x54, 0x7F, .ucTimerTarget=2, .usTimerExpire=m_pConfig->GetLongPressTimeout (), .bSkip=true}, // Main knob click
		{1, 0, MIDI_NOTE_ON, 0x54, 0x00, 0, MIDI_DAW_CHANGE, MIDI_DAW_MENU_SELECT, 0, .bGroup=true},
		{1, 0, MIDI_NOTE_ON, 0x54, 0x00, 0, MIDI_DAW_CHANGE, MIDI_DAW_MENU_BACK, 0, .bGroup=true},
		{0, 0, MIDI_CONTROL_CHANGE, 0x3C, TMIDIRoute::LtCenter, 0, MIDI_DAW_CHANGE, MIDI_DAW_MENU_PRESS_PREV, 0xFF, .bGroup=true, .bGroupHold=true}, // Main knob click + rotate
		{0, 0, MIDI_CONTROL_CHANGE, 0x3C, TMIDIRoute::GtCenter, 0, MIDI_DAW_CHANGE, MIDI_DAW_MENU_PRESS_NEXT, 0xFF, .bGroup=true, .bGroupHold=true},

		{1, 0, MIDI_CONTROL_CHANGE, 0x3C, TMIDIRoute::GtCenter, 0, MIDI_DAW_CHANGE, MIDI_DAW_MENU_PREV, 0xFF}, // Main knob
		{1, 0, MIDI_CONTROL_CHANGE, 0x3C, TMIDIRoute::LtCenter, 0, MIDI_DAW_CHANGE, MIDI_DAW_MENU_NEXT, 0xFF},

		{1, 0, MIDI_PITCH_BEND, 0xFF, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader1
		{1, 1, MIDI_PITCH_BEND, 0xFF, 0xFF, 1, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader2
		{1, 2, MIDI_PITCH_BEND, 0xFF, 0xFF, 2, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader3
		{1, 3, MIDI_PITCH_BEND, 0xFF, 0xFF, 3, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader4
		{1, 4, MIDI_PITCH_BEND, 0xFF, 0xFF, 4, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader5
		{1, 5, MIDI_PITCH_BEND, 0xFF, 0xFF, 5, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader6
		{1, 6, MIDI_PITCH_BEND, 0xFF, 0xFF, 6, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader7
		{1, 7, MIDI_PITCH_BEND, 0xFF, 0xFF, 7, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader8
		{1, 8, MIDI_PITCH_BEND, 0xFF, 0xFF, 8, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader9

		/*{0, 0, MIDI_CONTROL_CHANGE, 96, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_FREQUENCY_CUTOFF, 0xFF}, // Knob1
		{0, 0, MIDI_CONTROL_CHANGE, 97, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_RESONANCE, 0xFF}, // Knob2
		{0, 0, MIDI_CONTROL_CHANGE, 98, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_REVERB_LEVEL, 0xFF}, // Knob3
		{0, 0, MIDI_CONTROL_CHANGE, 99, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_DETUNE_LEVEL, 0xFF}, // Knob4
		{0, 0, MIDI_CONTROL_CHANGE, 100, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_PAN_POSITION, 0xFF}, // Knob5
		{0, 0, MIDI_CONTROL_CHANGE, 101, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_PORTAMENTO_TIME, 0xFF}, // Knob6
		// {0, 0, MIDI_CONTROL_CHANGE, 102, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_DETUNE_LEVEL, 0xFF}, // Knob7
		// {0, 0, MIDI_CONTROL_CHANGE, 103, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_PAN_POSITION, 0xFF}, // Knob8
		// {0, 0, MIDI_CONTROL_CHANGE, 104, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_PAN_POSITION, 0xFF}, // Knob9*/
		{1, 0xFF, 0xFF, 0xFF, 0xFF, .bSkip = true}, // skip other messages on DAW cable
		{0xFF}, // Sentinel
	};
};

CKeyLab2DawConnection::CKeyLab2DawConnection (CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard, CConfig *pConfig, CUserInterface *pUI)
	:m_pSynthesizer (pSynthesizer), m_pKeyboard (pKeyboard), m_pConfig (pConfig), m_pUI (pUI)
{
	static const uint8_t pInit[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x02, 0x00, 0x40, 0x52, 0x00, 0xF7};
	m_pKeyboard->SetRouteMap (m_pRouteMap);

	m_pKeyboard->Send (pInit, sizeof pInit, 0);
	DisplayWrite ("MiniDexed", "", "On KeyLab 2", 0, 0);

	UpdateState ();
}

void CKeyLab2DawConnection::DisplayWrite (const char *pMenu, const char *pParam, const char *pValue, bool bArrowDown, bool bArrowUp)
{
	static const uint8_t pHdr[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x04, 0x00, 0x60, 0x01};
	ArturiaDisplayWrite (m_pKeyboard, pHdr, sizeof pHdr, 16, true, true, pMenu, pParam, pValue, bArrowDown, bArrowUp);
}

void CKeyLab2DawConnection::UpdateState ()
{
}

void CKeyLab2DawConnection::UpdateMenu (CUIMenu::TCPageType Type, s8 ucPage, u8 ucOP, u8 ucTG)
{
}

void CKeyLab2DawConnection::MIDIListener (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2)
{
	//static const uint8_t pEncoder[] = {0x04, 0x01, 0x60};
	//ArturiaShowNewCCValue (pKeyboard, pEncoder, ucCh, ucCC, ucValue);
	switch (ucType)
	{
	case MIDI_DAW_CHANGE:
		HandleMenuEvents (m_pUI, ucP1);
		break;
	}
}

class CKeyLabEsDawConnection : public CDAWConnection
{
public:
	CKeyLabEsDawConnection (CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard, CConfig *pConfig, CUserInterface *pUI);
	void DisplayWrite (const char *pMenu, const char *pParam, const char *pValue, bool bArrowDown, bool bArrowUp) override;
	void UpdateState () override;
	void UpdateMenu (CUIMenu::TCPageType Type, s8 ucPage, u8 ucOP, u8 ucTG) override;
	void MIDIListener (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2) override;
private:
	CMiniDexed *m_pSynthesizer;
	CMIDIKeyboard *m_pKeyboard;
	CConfig *m_pConfig;
	CUserInterface *m_pUI;

	TMIDIRoute m_pRouteMap[18] = {
		{1, 0, MIDI_NOTE_ON, 0x54, 0x7F, .ucTimerTarget=2, .usTimerExpire=m_pConfig->GetLongPressTimeout (), .bSkip=true}, // Main knob click
		{1, 0, MIDI_NOTE_ON, 0x54, 0x00, 0, MIDI_DAW_CHANGE, MIDI_DAW_MENU_SELECT, 0, .bGroup=true},
		{1, 0, MIDI_NOTE_ON, 0x54, 0x00, 0, MIDI_DAW_CHANGE, MIDI_DAW_MENU_BACK, 0, .bGroup=true},
		{0, 0, MIDI_CONTROL_CHANGE, 0x3C, TMIDIRoute::LtCenter, 0, MIDI_DAW_CHANGE, MIDI_DAW_MENU_PRESS_PREV, 0xFF, .bGroup=true, .bGroupHold=true}, // Main knob click + rotate
		{0, 0, MIDI_CONTROL_CHANGE, 0x3C, TMIDIRoute::GtCenter, 0, MIDI_DAW_CHANGE, MIDI_DAW_MENU_PRESS_NEXT, 0xFF, .bGroup=true, .bGroupHold=true},

		{1, 0, MIDI_CONTROL_CHANGE, 0x3C, TMIDIRoute::GtCenter, 0, MIDI_DAW_CHANGE, MIDI_DAW_MENU_PREV, 0xFF}, // Main knob
		{1, 0, MIDI_CONTROL_CHANGE, 0x3C, TMIDIRoute::LtCenter, 0, MIDI_DAW_CHANGE, MIDI_DAW_MENU_NEXT, 0xFF},

		{1, 0, MIDI_PITCH_BEND, 0xFF, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader1
		{1, 1, MIDI_PITCH_BEND, 0xFF, 0xFF, 1, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader2
		{1, 2, MIDI_PITCH_BEND, 0xFF, 0xFF, 2, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader3
		{1, 3, MIDI_PITCH_BEND, 0xFF, 0xFF, 3, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader4
		{1, 4, MIDI_PITCH_BEND, 0xFF, 0xFF, 4, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader5
		{1, 5, MIDI_PITCH_BEND, 0xFF, 0xFF, 5, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader6
		{1, 6, MIDI_PITCH_BEND, 0xFF, 0xFF, 6, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader7
		{1, 7, MIDI_PITCH_BEND, 0xFF, 0xFF, 7, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader8
		{1, 8, MIDI_PITCH_BEND, 0xFF, 0xFF, 8, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader9
		/*{0, 0, MIDI_CONTROL_CHANGE, 96, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_FREQUENCY_CUTOFF, 0xFF}, // Knob1
		{0, 0, MIDI_CONTROL_CHANGE, 97, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_RESONANCE, 0xFF}, // Knob2
		{0, 0, MIDI_CONTROL_CHANGE, 98, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_REVERB_LEVEL, 0xFF}, // Knob3
		{0, 0, MIDI_CONTROL_CHANGE, 99, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_DETUNE_LEVEL, 0xFF}, // Knob4
		{0, 0, MIDI_CONTROL_CHANGE, 100, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_PAN_POSITION, 0xFF}, // Knob5
		{0, 0, MIDI_CONTROL_CHANGE, 101, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_PORTAMENTO_TIME, 0xFF}, // Knob6
		// {0, 0, MIDI_CONTROL_CHANGE, 102, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_DETUNE_LEVEL, 0xFF}, // Knob7
		// {0, 0, MIDI_CONTROL_CHANGE, 103, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_PAN_POSITION, 0xFF}, // Knob8
		// {0, 0, MIDI_CONTROL_CHANGE, 104, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_PAN_POSITION, 0xFF}, // Knob9*/
		{1, 0xFF, 0xFF, 0xFF, 0xFF, .bSkip = true}, // skip other messages on DAW cable
		{0xFF}, // Sentinel
	};
};

CKeyLabEsDawConnection::CKeyLabEsDawConnection (CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard, CConfig *pConfig, CUserInterface *pUI)
	:m_pSynthesizer (pSynthesizer), m_pKeyboard (pKeyboard), m_pConfig (pConfig), m_pUI (pUI)
{
	static const uint8_t pInit[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x02, 0x00, 0x40, 0x51, 0x00, 0xF7}; // init DAW to Mackie mode
	m_pKeyboard->SetRouteMap (m_pRouteMap);

	m_pKeyboard->Send (pInit, sizeof pInit, 0);
	DisplayWrite ("MiniDexed", "", "On KeyLab Essential", 0, 0);

	UpdateState ();
}

void CKeyLabEsDawConnection::DisplayWrite (const char *pMenu, const char *pParam, const char *pValue, bool bArrowDown, bool bArrowUp)
{
	static const uint8_t pHdr[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x04, 0x00, 0x60, 0x01};
	ArturiaDisplayWrite (m_pKeyboard, pHdr, sizeof pHdr, 16, true, true, pMenu, pParam, pValue, bArrowDown, bArrowUp);
}

void CKeyLabEsDawConnection::UpdateState ()
{
}

void CKeyLabEsDawConnection::UpdateMenu (CUIMenu::TCPageType Type, s8 ucPage, u8 ucOP, u8 ucTG)
{
}

void CKeyLabEsDawConnection::MIDIListener (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2)
{
	//static const uint8_t pEncoder[] = {0x04, 0x01, 0x60};
	//ArturiaShowNewCCValue (pKeyboard, pEncoder, ucCh, ucCC, ucValue);
	switch (ucType)
	{
	case MIDI_DAW_CHANGE:
		HandleMenuEvents (m_pUI, ucP1);
		break;
	}
}

CDAWController::CDAWController (CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard, CConfig *pConfig, CUserInterface *pUI)
:	m_pSynthesizer (pSynthesizer),
	m_pKeyboard (pKeyboard),
	m_pConfig (pConfig),
	m_pUI (pUI),
	m_pDAWConnection (0)
{
}

CDAWController::~CDAWController (void)
{
	delete m_pDAWConnection;
}

void CDAWController::OnConnect (void)
{
	static const uint8_t inquiry[] = {0xF0, 0x7E, 0x7F, 0x06, 0x01, 0xF7};

	delete m_pDAWConnection;
	m_pDAWConnection = 0;

	m_pKeyboard->Send (inquiry, sizeof inquiry, 0);
}

void CDAWController::MIDISysexHandler (u8 *pPacket, unsigned nLength, unsigned nCable)
{
	static const uint8_t pMiniLab3[] =		{0xF0, 0x7E, 0x7F, 0x06, 0x02, 0x00, 0x20, 0x6B, 0x02, 0x00, 0x04, 0x04};
	static const uint8_t pKeyLabEs_49[] = 	{0xF0, 0x7E, 0x7F, 0x06, 0x02, 0x00, 0x20, 0x6B, 0x02, 0x00, 0x05, 0x52};
	static const uint8_t pKeyLabEs_61[] = 	{0xF0, 0x7E, 0x7F, 0x06, 0x02, 0x00, 0x20, 0x6B, 0x02, 0x00, 0x05, 0x54};
	static const uint8_t pKeyLabEs_88[] = 	{0xF0, 0x7E, 0x7F, 0x06, 0x02, 0x00, 0x20, 0x6B, 0x02, 0x00, 0x05, 0x58};
	static const uint8_t pKeyLab2_49[] = 	{0xF0, 0x7E, 0x7F, 0x06, 0x02, 0x00, 0x20, 0x6B, 0x02, 0x00, 0x05, 0x62};
	static const uint8_t pKeyLab2_61[] = 	{0xF0, 0x7E, 0x7F, 0x06, 0x02, 0x00, 0x20, 0x6B, 0x02, 0x00, 0x05, 0x64};
	static const uint8_t pKeyLab2_88[] = 	{0xF0, 0x7E, 0x7F, 0x06, 0x02, 0x00, 0x20, 0x6B, 0x02, 0x00, 0x05, 0x68};
	static const uint8_t pKeyLabEs3_49[] =	{0xF0, 0x7E, 0x7F, 0x06, 0x02, 0x00, 0x20, 0x6B, 0x02, 0x00, 0x05, 0x72};
	static const uint8_t pKeyLabEs3_61[] =	{0xF0, 0x7E, 0x7F, 0x06, 0x02, 0x00, 0x20, 0x6B, 0x02, 0x00, 0x05, 0x74};
	static const uint8_t pKeyLabEs3_88[] =	{0xF0, 0x7E, 0x7F, 0x06, 0x02, 0x00, 0x20, 0x6B, 0x02, 0x00, 0x05, 0x78};

	if (nLength > sizeof pMiniLab3 && memcmp (pPacket, pMiniLab3, sizeof pMiniLab3) == 0)
	{
		m_pDAWConnection = new CMiniLab3DawConnection (m_pSynthesizer, m_pKeyboard, m_pConfig, m_pUI);
	}
	else if (nLength > sizeof pKeyLabEs_49 && (
		memcmp (pPacket, pKeyLabEs_49, sizeof pKeyLabEs_49) == 0 ||
		memcmp (pPacket, pKeyLabEs_61, sizeof pKeyLabEs_61) == 0 ||
		memcmp (pPacket, pKeyLabEs_88, sizeof pKeyLabEs_88) == 0))
	{
		m_pDAWConnection = new CKeyLabEsDawConnection (m_pSynthesizer, m_pKeyboard, m_pConfig, m_pUI);
	}
	else if (nLength > sizeof pKeyLab2_61 && (
		memcmp (pPacket, pKeyLab2_49, sizeof pKeyLab2_49) == 0 ||
		memcmp (pPacket, pKeyLab2_61, sizeof pKeyLab2_61) == 0 ||
		memcmp (pPacket, pKeyLab2_88, sizeof pKeyLab2_88) == 0))
	{
		m_pDAWConnection = new CKeyLab2DawConnection (m_pSynthesizer, m_pKeyboard, m_pConfig, m_pUI);
	}
	else if (nLength > sizeof pKeyLabEs3_49 && (
		memcmp (pPacket, pKeyLabEs3_49, sizeof pKeyLabEs3_49) == 0 ||
		memcmp (pPacket, pKeyLabEs3_61, sizeof pKeyLabEs3_61) == 0 ||
		memcmp (pPacket, pKeyLabEs3_88, sizeof pKeyLabEs3_88) == 0))
	{
		m_pDAWConnection = new CKeyLabEs3DawConnection (m_pSynthesizer, m_pKeyboard, m_pConfig, m_pUI);
	}
}

void CDAWController::DisplayWrite (const char *pMenu, const char *pParam, const char *pValue,
				bool bArrowDown, bool bArrowUp)
{
	if (m_pDAWConnection)
		m_pDAWConnection->DisplayWrite (pMenu, pParam, pValue, bArrowDown, bArrowUp);
}

void CDAWController::UpdateState (void)
{
	if (m_pDAWConnection)
		m_pDAWConnection->UpdateState ();
}

void CDAWController::UpdateMenu (CUIMenu::TCPageType Type, s8 ucPage, u8 ucOP, u8 ucTG)
{
	if (m_pDAWConnection)
		m_pDAWConnection->UpdateMenu (Type, ucPage, ucOP, ucTG);
}

void CDAWController::MIDIListener (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2)
{
	if (m_pDAWConnection)
		m_pDAWConnection->MIDIListener (ucCable, ucChannel, ucType, ucP1, ucP2);
}
