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

static void ArturiaDisplayWrite (CMIDIKeyboard *pKeyboard, const u8 *pHdr, const unsigned nHdrSize, unsigned L1MaxLen, const char *pMenu, const char *pParam, const char *pValue)
{
	CString line1 (pParam);

	size_t nLen = strlen (pParam) + strlen (pMenu);
	if (nLen < L1MaxLen)
	{
		for (unsigned i = L1MaxLen - nLen; i > 0; i--)
		{
			line1.Append (" ");
		}
	}

	line1.Append (pMenu);

	int nLine1Len = strlen (line1);
	int nLine2Len = strlen (pValue);
	int nOffset = 0;

	uint8_t pLines[nHdrSize + nLine1Len + 2 + nLine2Len  + 2];

	memcpy (pLines, pHdr, nHdrSize);
	nOffset += nHdrSize;

	memcpy (&pLines[nOffset], line1, nLine1Len + 1);
	nOffset += nLine1Len + 1;

	pLines[nOffset] = 0x02;
	nOffset += 1;

	memcpy (&pLines[nOffset], pValue, nLine2Len + 1);
	nOffset += nLine2Len + 1;

	pLines[nOffset] = 0xf7;
	nOffset += 1;

	// block character (0xFF) is not supported, change to =
	for (unsigned i = 0; i < sizeof pLines; ++i)
		if (pLines[i] == 0xFF)
			pLines[i] = '=';

	pKeyboard->SendDebounce (pLines, nOffset, 0);
}

enum ControlType
{
	CT_KNOB = 3,
	CT_FADER,
	CT_PAD,
};

static void ArturiaDisplayInfoWrite (CMIDIKeyboard *pKeyboard, const uint8_t pDisplayHdr[3], ControlType Type, u8 uValue, const char *pName, const char *pValue)
{
	const uint8_t pHdr[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, pDisplayHdr[0], pDisplayHdr[1], pDisplayHdr[2], 0x1F, Type, 0x02, uValue, 0x00, 0x00, 0x01};

	int nLine1Len = strlen (pName);
	int nLine2Len = strlen (pValue);
	int nOffset = 0;

	uint8_t pLines[sizeof pHdr + nLine1Len + 2 + nLine2Len + 2];

	memcpy (pLines, pHdr, sizeof pHdr);
	nOffset += sizeof pHdr;

	memcpy (pLines + nOffset, pName, nLine1Len + 1);
	nOffset += nLine1Len + 1;

	pLines[nOffset] = 0x02;
	nOffset += 1;

	memcpy (pLines + nOffset, pValue, nLine2Len + 1);
	nOffset += nLine2Len + 1;

	pLines[nOffset] = 0xf7;
	nOffset += 1;

	pKeyboard->SendDebounce (pLines, nOffset, 0);
}

static void ArturiaShowNewCCValue (CMIDIKeyboard *pKeyboard, const uint8_t pDisplayHdr[3], u8 ucCh, u8 ucCC, u8 ucValue)
{
	char line1[LINELEN];
	char line2[LINELEN];

	switch (ucCC)
	{
	case MIDI_CC_PORTAMENTO_TIME:
		snprintf(line2, LINELEN, "%ld%%", maplong(ucValue, 0, 127, 0, 99));
		ArturiaDisplayInfoWrite (pKeyboard, pDisplayHdr, CT_KNOB, ucValue, "Portamento Time", line2);
		break;
	case MIDI_CC_VOLUME:
		snprintf(line1, LINELEN, "Volume Ch %d", ucCh + 1);
		snprintf(line2, LINELEN, "%ld%%", maplong(ucValue, 0, 127, 0, 100));
		ArturiaDisplayInfoWrite (pKeyboard, pDisplayHdr, CT_FADER, ucValue, line1, line2);
		break;
	case MIDI_CC_FREQUENCY_CUTOFF:
		snprintf(line2, LINELEN, "%ld%%", maplong(ucValue, 0, 127, 0, 99));
		ArturiaDisplayInfoWrite (pKeyboard, pDisplayHdr, CT_KNOB, ucValue, "Cutoff", line2);
		break;
	case MIDI_CC_RESONANCE:
		snprintf(line2, LINELEN, "%ld%%", maplong(ucValue, 0, 127, 0, 99));
		ArturiaDisplayInfoWrite (pKeyboard, pDisplayHdr, CT_KNOB, ucValue, "Resonance", line2);
		break;
	case MIDI_CC_REVERB_LEVEL:
		snprintf(line2, LINELEN, "%ld%%", maplong(ucValue, 0, 127, 0, 99));
		ArturiaDisplayInfoWrite (pKeyboard, pDisplayHdr, CT_KNOB, ucValue, "Reverb", line2);
		break;
	case MIDI_CC_DETUNE_LEVEL:
		snprintf(line2, LINELEN, "%ld", maplong(ucValue, 1, 127, -99, 99));
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
	case MIDI_CC_ALL_SOUND_OFF:
		ArturiaDisplayInfoWrite (pKeyboard, pDisplayHdr, CT_PAD, ucValue, "All Sound Off", "");
		break;
	}
}

class CDAWConnection
{
public:
	virtual void DisplayWrite (const char *pMenu, const char *pParam,
				   const char *pValue, bool bArrowDown, bool bArrowUp) = 0;
	virtual void UpdateState () = 0;
	virtual void ShowNewCCValue (u8 ucCh, u8 ucCC, u8 ucValue) = 0;
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
	void ShowNewCCValue (u8 ucCh, u8 ucCC, u8 ucValue) override;
private:
	enum TPadID {
		MonoPad = 0,
		PortamentoPad = 1,
		SostenutoPad = 2,
		SustainPad = 3,
		SoundOffPad = 4,
		TBDPad6 = 5,
		TBDPad7 = 6,
		ATPad = 7,
	};
	enum TBankID {
		BankA = 0x34,
		BankB = 0x44,
	};

	void UpdateEncoder (uint8_t ucEncID, uint8_t ucValue);
	void UpdateTGColor (uint8_t nTG);
	void UpdateMonoColor ();
	void UpdatePortamentoColor ();
	void UpdateATColor (u8 ucAT);
	void UpdateVolumeFaders ();

	void SetPadColor (TBankID BankID, TPadID PadID, u8 state);
	void SetPadColor (TBankID BankID, TPadID PadID, CColor color, u8 state);
	void SetPadColor (TBankID BankID, TPadID PadID, CColor color);

	static void s_SetChannelAT (void *data, u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2);
	void SetChannelAT (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2);
	static void s_SetVoice (void *data, u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2);
	void SetVoice (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2);
	static void s_SetAlgorithm (void *data, u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2);
	void SetAlgorithm (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2);
	static void s_SetMonoMode (void *data, u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2);
	void SetMonoMode (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2);
	static void s_EnableTG (void *data, u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2);
	void EnableTG (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2);

	CMiniDexed *m_pSynthesizer;
	CMIDIKeyboard *m_pKeyboard;
	const uint8_t m_pEncoder[3] = {0x04, 0x02, 0x60};
	TMIDIRouteMap m_pRouteMap[46] = {
		{0, 0, MIDI_CONTROL_CHANGE, 14, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader1
		{0, 0, MIDI_CONTROL_CHANGE, 15, 0xFF, 1, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader2
		{0, 0, MIDI_CONTROL_CHANGE, 30, 0xFF, 2, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader3
		{0, 0, MIDI_CONTROL_CHANGE, 31, 0xFF, 3, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader4
		{0, 0, MIDI_CONTROL_CHANGE, 86, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_FREQUENCY_CUTOFF, 0xFF}, // Knob1
		{0, 0, MIDI_CONTROL_CHANGE, 87, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_RESONANCE, 0xFF}, // Knob2
		{0, 0, MIDI_CONTROL_CHANGE, 89, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_REVERB_LEVEL, 0xFF}, // Knob3
		{0, 0, MIDI_CONTROL_CHANGE, 90, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_DETUNE_LEVEL, 0xFF}, // Knob4
		{0, 0, MIDI_CONTROL_CHANGE, 110, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_PORTAMENTO_TIME, 0xFF}, // Knob5
		{0, 0, MIDI_CONTROL_CHANGE, 111, 0xFF, 0, MIDI_CONTROL_CHANGE, 111, 0xFF, .bSkip=true, .pFun=s_SetVoice, .pData=this}, // Knob6
		{0, 0, MIDI_CONTROL_CHANGE, 116, 0xFF, 0, MIDI_CONTROL_CHANGE, 116, 0xFF, .bSkip=true, .pFun=s_SetAlgorithm, .pData=this}, // Knob7
		{0, 0, MIDI_CONTROL_CHANGE, 117, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_PAN_POSITION, 0xFF}, // Knob8
		{0, 9, MIDI_NOTE_ON, 36, 0xFF, 0, MIDI_NOTE_ON, 36, 0x7F, .bSkip=true, .pFun=s_SetMonoMode, .pData=this}, // BankA Pad1
		{0, 9, MIDI_NOTE_OFF, 36, 0xFF, 0, MIDI_NOTE_ON, 36, 0xFF, .bSkip=true}, // BankA Pad1
		{0, 9, MIDI_NOTE_ON, 37, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_PORTAMENTO, 0x7F, .bToggle=true}, // BankA Pad2
		{0, 9, MIDI_NOTE_OFF, 37, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_PORTAMENTO, 0x00, .bSkip=true}, // BankA Pad2
		{0, 9, MIDI_NOTE_ON, 38, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_SOSTENUTO, 0x7F, .bToggle=true}, // BankA Pad3
		{0, 9, MIDI_NOTE_OFF, 38, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_SOSTENUTO, 0x00, .bSkip=true}, // BankA Pad3
		{0, 9, MIDI_NOTE_ON, 39, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_BANK_SUSTAIN, 0x7F, .bToggle=true}, // BankA Pad4
		{0, 9, MIDI_NOTE_OFF, 39, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_BANK_SUSTAIN, 0x00, .bSkip=true}, // BankA Pad4
		{0, 9, MIDI_NOTE_ON, 40, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_ALL_SOUND_OFF, 0x7f}, // BankA Pad5
		{0, 9, MIDI_NOTE_OFF, 40, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_ALL_SOUND_OFF, 0x00}, // BankA Pad5
		{0, 9, MIDI_NOTE_ON, 41, 0xFF, 9, MIDI_NOTE_ON, 0xFF, 0xFF, .bSkip=true}, // BankA Pad6
		{0, 9, MIDI_NOTE_OFF, 41, 0xFF, 9, MIDI_NOTE_OFF, 0xFF, 0xFF, .bSkip=true}, // BankA Pad6
		{0, 9, MIDI_NOTE_ON, 42, 0xFF, 9, MIDI_NOTE_ON, 0xFF, 0xFF, .bSkip=true}, // BankA Pad7
		{0, 9, MIDI_NOTE_OFF, 42, 0xFF, 9, MIDI_NOTE_OFF, 0xFF, 0xFF, .bSkip=true}, // BankA Pad7
		{0, 9, MIDI_NOTE_ON, 43, 0xFF, 0, MIDI_NOTE_ON, 0xFF, 0xFF, .bSkip=true}, // BankA Pad8
		{0, 9, MIDI_NOTE_OFF, 43, 0xFF, 0, MIDI_CHANNEL_AFTERTOUCH, 0x00, 0xFF, .pFun=s_SetChannelAT, .pData=this}, // BankA Pad8 AT
		{0, 9, MIDI_AFTERTOUCH, 43, 0xFF, 0, MIDI_CHANNEL_AFTERTOUCH, 0X82, 0xFF, .pFun=s_SetChannelAT, .pData=this}, // BankA Pad8 AT 0x82=copy from p2
		{0, 9, MIDI_NOTE_ON, 44, 0xFF, 0, MIDI_NOTE_ON, 0, 0x7F, .bSkip=true, .pFun=s_EnableTG, .pData=this}, // BankB Pad1
		{0, 9, MIDI_NOTE_OFF, 44, 0xFF, 0, MIDI_NOTE_ON, 0, 0xFF, .bSkip=true}, // BankB Pad1
		{0, 9, MIDI_NOTE_ON, 45, 0xFF, 0, MIDI_NOTE_ON, 1, 0x7F, .bSkip=true, .pFun=s_EnableTG, .pData=this}, // BankB Pad2
		{0, 9, MIDI_NOTE_OFF, 45, 0xFF, 0, MIDI_NOTE_ON, 1, 0xFF, .bSkip=true}, // BankB Pad2
		{0, 9, MIDI_NOTE_ON, 46, 0xFF, 0, MIDI_NOTE_ON, 2, 0x7F, .bSkip=true, .pFun=s_EnableTG, .pData=this}, // BankB Pad3
		{0, 9, MIDI_NOTE_OFF, 46, 0xFF, 0, MIDI_NOTE_ON, 2, 0xFF, .bSkip=true}, // BankB Pad3
		{0, 9, MIDI_NOTE_ON, 47, 0xFF, 0, MIDI_NOTE_ON, 3, 0x7F, .bSkip=true, .pFun=s_EnableTG, .pData=this}, // BankB Pad4
		{0, 9, MIDI_NOTE_OFF, 47, 0xFF, 0, MIDI_NOTE_ON, 3, 0xFF, .bSkip=true}, // BankB Pad4
		{0, 9, MIDI_NOTE_ON, 48, 0xFF, 0, MIDI_NOTE_ON, 4, 0x7F, .bSkip=true, .pFun=s_EnableTG, .pData=this}, // BankB Pad5
		{0, 9, MIDI_NOTE_OFF, 48, 0xFF, 0, MIDI_NOTE_ON, 4, 0xFF, .bSkip=true}, // BankB Pad5
		{0, 9, MIDI_NOTE_ON, 49, 0xFF, 0, MIDI_NOTE_ON, 5, 0x7F, .bSkip=true, .pFun=s_EnableTG, .pData=this}, // BankB Pad6
		{0, 9, MIDI_NOTE_OFF, 49, 0xFF, 0, MIDI_NOTE_ON, 5, 0xFF, .bSkip=true}, // BankB Pad6
		{0, 9, MIDI_NOTE_ON, 50, 0xFF, 0, MIDI_NOTE_ON, 6, 0x7F, .bSkip=true, .pFun=s_EnableTG, .pData=this}, // BankB Pad7
		{0, 9, MIDI_NOTE_OFF, 50, 0xFF, 0, MIDI_NOTE_ON, 6, 0xFF, .bSkip=true}, // BankB Pad7
		{0, 9, MIDI_NOTE_ON, 51, 0xFF, 0, MIDI_NOTE_ON, 7, 0x7F, .bSkip=true, .pFun=s_EnableTG, .pData=this}, // BankB Pad8
		{0, 9, MIDI_NOTE_OFF, 51, 0xFF, 0, MIDI_NOTE_ON, 7, 0xFF, .bSkip=true}, // BankB Pad8
		{0xFF}, // Sentinel
	};
};

CMiniLab3DawConnection::CMiniLab3DawConnection (CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard, CConfig *pConfig, CUserInterface *pUI)
	:m_pSynthesizer (pSynthesizer), m_pKeyboard (pKeyboard)
{
	static const uint8_t pInit[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x02, 0x00, 0x40, 0x6A, 0x21, 0xF7};

	m_pKeyboard->SetMIDIRouteMap (m_pRouteMap);

	m_pKeyboard->Send (pInit, sizeof pInit, 0);
	DisplayWrite ("MiniDexed", "", "On MiniLab 3", 0, 0);

	SetPadColor (BankA, MonoPad, 0);
	SetPadColor (BankA, PortamentoPad, 0);
	SetPadColor (BankA, SostenutoPad, 0);
	SetPadColor (BankA, SustainPad, 0);
	SetPadColor (BankA, SoundOffPad, 0);
	SetPadColor (BankA, TBDPad6, 0);
	SetPadColor (BankA, TBDPad7, 0);
	UpdateATColor (0);

	UpdateState ();

	pConfig->SetMIDIButtonCh (1);
	pConfig->SetMIDIButtonPrev (28);
	pConfig->SetMIDIButtonActionPrev("dec");
	pConfig->SetMIDIButtonNext (28);
	pConfig->SetMIDIButtonActionNext("inc");
	pConfig->SetMIDIButtonBack (118);
	pConfig->SetMIDIButtonActionBack("longpress");
	pConfig->SetMIDIButtonSelect (118);
	pConfig->SetMIDIButtonActionSelect("click");
	pConfig->SetMIDIButtonHome (119);
	pConfig->SetMIDIButtonActionHome("click");

	pUI->InitButtonsWithConfig(pConfig);
}

void CMiniLab3DawConnection::DisplayWrite (const char *pMenu, const char *pParam, const char *pValue, bool bArrowDown, bool bArrowUp)
{
	static const uint8_t pHdr[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x04, 0x02, 0x60, 0x12, 0x01};
	ArturiaDisplayWrite (m_pKeyboard, pHdr, sizeof pHdr, 18, pMenu, pParam, pValue);
}

void CMiniLab3DawConnection::SetPadColor (TBankID BankID, TPadID PadID, u8 state)
{
	SetPadColor (BankID, PadID, padColors[PadID], state);
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
	SetPadColor (BankA, MonoPad, m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterMonoMode, 0));
}

void CMiniLab3DawConnection::UpdatePortamentoColor ()
{
	SetPadColor (BankA, PortamentoPad, m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterPortamentoMode, 0));
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

	for (unsigned i = 0; i < CConfig::AllToneGenerators; ++i)
	{
		int channel = m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterMIDIChannel, i);

		if (channel == CMIDIDevice::TChannel::ChannelUnknown || channel == CMIDIDevice::TChannel::Disabled)
			continue;

		if (channel == CMIDIDevice::TChannel::OmniMode)
			channel = 0;

		for (unsigned i = 0; i < sizeof chan_map; ++i)
		{
			if (chan_map[i] == channel)
				break;

			if (chan_map[i] == CMIDIDevice::TChannel::Disabled) {
				chan_map[i] = channel;
				break;
			}
		}
	}

	for (unsigned i = 0; i < sizeof chan_map; ++i)
	{
		if (chan_map[i] == CMIDIDevice::TChannel::Disabled)
			m_pRouteMap[i].bSkip = true;
		else {
			m_pRouteMap[i].bSkip = false;
			m_pRouteMap[i].ucDCh = chan_map[i];
		}
	}
}

void CMiniLab3DawConnection::UpdateState ()
{
	UpdateEncoder (0, maplong(m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterCutoff, 0), 0, 99, 0, 127));
	UpdateEncoder (1, maplong(m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterResonance, 0), 0, 99, 0, 127));
	UpdateEncoder (2, maplong(m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterReverbSend, 0), 0, 99, 0, 127));
	UpdateEncoder (3, maplong(m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterMasterTune, 0), -99, 99, 1, 127));
	UpdateEncoder (4, maplong(m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterPortamentoTime, 0), 0, 99, 0, 127));
	UpdateEncoder (5, m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterProgram, 0));
	UpdateEncoder (6, maplong(m_pSynthesizer->GetVoiceParameter (DEXED_ALGORITHM, CMiniDexed::NoOP, 0), 0, 31, 0, 127));
	UpdateEncoder (7, m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterPan, 0));

	UpdateMonoColor ();
	// TODO change the MIDIRouteMap's value also
	UpdatePortamentoColor ();

	for (unsigned i = 0; i < CConfig::AllToneGenerators; ++i)
		UpdateTGColor (i);

	UpdateVolumeFaders ();
}

void CMiniLab3DawConnection::ShowNewCCValue (u8 ucCh, u8 ucCC, u8 ucValue)
{
	ArturiaShowNewCCValue (m_pKeyboard, m_pEncoder, ucCh, ucCC, ucValue);

	switch (ucCC)
	{
	case MIDI_CC_PORTAMENTO:
		SetPadColor (BankA, PortamentoPad, ucValue);
		break;
	case MIDI_CC_SOSTENUTO:
		SetPadColor (BankA, SostenutoPad, ucValue);
		break;
	case MIDI_CC_BANK_SUSTAIN:
		SetPadColor (BankA, SustainPad, ucValue);
		break;
	case MIDI_CC_ALL_SOUND_OFF:
		SetPadColor (BankA, SoundOffPad, ucValue);
		break;
	}
}

void CMiniLab3DawConnection::s_SetChannelAT (void *data, u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2)
{
	static_cast<CMiniLab3DawConnection*>(data)->SetChannelAT (ucCable, ucChannel, ucType, ucP1, ucP2);
}

void CMiniLab3DawConnection::SetChannelAT (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2)
{
	char line2[LINELEN];
	snprintf(line2, LINELEN, "%d", ucP1);

	ArturiaDisplayInfoWrite (m_pKeyboard, m_pEncoder, CT_PAD, ucP1, "Channel AT", line2);
	
	UpdateATColor (ucP1);
}

void CMiniLab3DawConnection::s_SetVoice (void *data, u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2)
{
	static_cast<CMiniLab3DawConnection*>(data)->SetVoice (ucCable, ucChannel, ucType, ucP1, ucP2);
}

void CMiniLab3DawConnection::SetVoice (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2)
{
	std::string line2;
	for (unsigned i = 0; i < CConfig::AllToneGenerators; ++i)
	{
		if (m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterEnabled, i) == 0)
			continue;
		m_pSynthesizer->ProgramChange (ucP2, i);
		if (line2.length() == 0) {
			std::string sVoiceName = m_pSynthesizer->GetVoiceName (i);
			if (sVoiceName.length() > 0)
				line2 = std::to_string (ucP2 + 1) + "=" + sVoiceName;
		}
	}
	
	ArturiaDisplayInfoWrite (m_pKeyboard, m_pEncoder, CT_KNOB, ucP2, "Voice", line2.c_str());
}

void CMiniLab3DawConnection::s_SetAlgorithm (void *data, u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2)
{
	static_cast<CMiniLab3DawConnection*>(data)->SetAlgorithm (ucCable, ucChannel, ucType, ucP1, ucP2);
}

void CMiniLab3DawConnection::SetAlgorithm (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2)
{
	char line2[LINELEN];
	u8 algorithm = maplong (ucP2, 0, 127, 0, 31);

	for (unsigned i = 0; i < CConfig::AllToneGenerators; ++i)
	{
		if (m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterEnabled, i) == 0)
			continue;
		m_pSynthesizer->SetVoiceParameter (DEXED_ALGORITHM, algorithm, CMiniDexed::NoOP, i);
	}

	snprintf(line2, LINELEN, "%d", algorithm);
	ArturiaDisplayInfoWrite (m_pKeyboard, m_pEncoder, CT_KNOB, ucP2, "Algorithm", line2);
}

void CMiniLab3DawConnection::s_SetMonoMode (void *data, u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2)
{
	static_cast<CMiniLab3DawConnection*>(data)->SetMonoMode (ucCable, ucChannel, ucType, ucP1, ucP2);
}

void CMiniLab3DawConnection::SetMonoMode (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2)
{
	ucP2 = m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterMonoMode, 0) ? 0x00 : 0x7F;

	for (unsigned i = 0; i < CConfig::AllToneGenerators; ++i)
	{
		if (m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterEnabled, i) == 0)
			continue;
		m_pSynthesizer->setMonoMode (ucP2, i);
	}

	ArturiaDisplayInfoWrite (m_pKeyboard, m_pEncoder, CT_PAD, ucP2, "Mono Mode", ucP2 > 64 ? "On" : "Off");
	UpdateMonoColor ();
}


void CMiniLab3DawConnection::s_EnableTG (void *data, u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2)
{
	static_cast<CMiniLab3DawConnection*>(data)->EnableTG (ucCable, ucChannel, ucType, ucP1, ucP2);
}

void CMiniLab3DawConnection::EnableTG (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2)
{
	char line1[LINELEN];

	ucP2 = m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterEnabled, ucP1) ? 0x00 : 0x7F;

	m_pSynthesizer->setEnabled (ucP2, ucP1);
	m_pSynthesizer->panic (ucP2, ucP1);

	snprintf(line1, LINELEN, "TG %d", ucP1 + 1);
	ArturiaDisplayInfoWrite (m_pKeyboard, m_pEncoder, CT_PAD, ucP2, line1, ucP2 > 64 ? "On" : "Off");
	UpdateTGColor (ucP1);
}

class CKeyLabEs3DawConnection : public CDAWConnection
{
public:
	CKeyLabEs3DawConnection (CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard, CConfig *pConfig, CUserInterface *pUI);
	void DisplayWrite ( const char *pMenu, const char *pParam, const char *pValue, bool bArrowDown, bool bArrowUp) override;
	void UpdateState () override;
	void ShowNewCCValue (u8 ucCh, u8 ucCC, u8 ucValue) override;
private:
	void UpdateEncoder (uint8_t ucEncID, uint8_t ucValue);

	CMiniDexed *m_pSynthesizer;
	CMIDIKeyboard *m_pKeyboard;
	const uint8_t m_pEncoder[3] = {0x04, 0x01, 0x60};
	TMIDIRouteMap m_pRouteMap[16] = {
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
	:m_pSynthesizer (pSynthesizer), m_pKeyboard (pKeyboard) 
{
	static const uint8_t pInit[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x02, 0x00, 0x40, 0x6A, 0x21, 0xF7};

	m_pKeyboard->SetMIDIRouteMap (m_pRouteMap);

	m_pKeyboard->Send (pInit, sizeof pInit, 0);
	DisplayWrite ("MiniDexed", "", "On KeyLab 3 Essential", 0, 0);

	UpdateState ();

	pConfig->SetMIDIButtonCh (1);
	pConfig->SetMIDIButtonPrev (116);
	pConfig->SetMIDIButtonActionPrev("dec");
	pConfig->SetMIDIButtonNext (116);
	pConfig->SetMIDIButtonActionNext("inc");
	pConfig->SetMIDIButtonBack (117);
	pConfig->SetMIDIButtonActionBack("longpress");
	pConfig->SetMIDIButtonSelect (117);
	pConfig->SetMIDIButtonActionSelect("click");
	pConfig->SetMIDIButtonHome (44);
	pConfig->SetMIDIButtonActionHome("click");

	pUI->InitButtonsWithConfig(pConfig);
}

void CKeyLabEs3DawConnection::DisplayWrite (const char *pMenu, const char *pParam, const char *pValue, bool bArrowDown, bool bArrowUp)
{
	static const uint8_t pHdr[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x04, 0x01, 0x60, 0x12, 0x01};
	ArturiaDisplayWrite (m_pKeyboard, pHdr, sizeof pHdr, 18, pMenu, pParam, pValue);
}

void CKeyLabEs3DawConnection::UpdateEncoder (uint8_t ucEncID, uint8_t ucValue)
{
	uint8_t pUpdateEncoder[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x02, 0x0F, 0x40, ucEncID += 3, ucValue, 0xF7};
	m_pKeyboard->Send (pUpdateEncoder, sizeof pUpdateEncoder, 0);
} 

void CKeyLabEs3DawConnection::UpdateState ()
{
	UpdateEncoder (0, maplong(m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterCutoff, 0), 0, 99, 0, 127));
	UpdateEncoder (1, maplong(m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterResonance, 0), 0, 99, 0, 127));
	UpdateEncoder (2, maplong(m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterReverbSend, 0), 0, 99, 0, 127));
	UpdateEncoder (3, maplong(m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterMasterTune, 0), -99, 99, 1, 127));
	UpdateEncoder (4, m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterPan, 0));
	UpdateEncoder (5, maplong(m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterPortamentoTime, 0), 0, 99, 0, 127));
}

void CKeyLabEs3DawConnection::ShowNewCCValue (u8 ucCh, u8 ucCC, u8 ucValue)
{
	ArturiaShowNewCCValue (m_pKeyboard, m_pEncoder, ucCh, ucCC, ucValue);
}

class CKeyLab2DawConnection : public CDAWConnection
{
public:
	CKeyLab2DawConnection (CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard, CConfig *pConfig, CUserInterface *pUI);
	void DisplayWrite (const char *pMenu, const char *pParam, const char *pValue, bool bArrowDown, bool bArrowUp) override;
	void UpdateState () override;
	void ShowNewCCValue (u8 ucCh, u8 ucCC, u8 ucValue) override;
private:
	CMiniDexed *m_pSynthesizer;
	CMIDIKeyboard *m_pKeyboard;
	TMIDIRouteMap m_pRouteMap[13] = {
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
		{1, 0, MIDI_CONTROL_CHANGE, 0x3C, 0xFF, 0, MIDI_CONTROL_CHANGE, 60, 0xFF}, // Main rotate
		{1, 0, MIDI_NOTE_ON, 0x54, 0xFF, 0, MIDI_CONTROL_CHANGE, 63, 0xFF}, // Main click
		{1, 0xFF, 0xFF, 0xFF, 0xFF, .bSkip = true}, // skip other messages on DAW cable
		{0xFF}, // Sentinel
	};
};

CKeyLab2DawConnection::CKeyLab2DawConnection (CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard, CConfig *pConfig, CUserInterface *pUI)
	:m_pSynthesizer (pSynthesizer), m_pKeyboard (pKeyboard)
{
	static const uint8_t pInit[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x02, 0x00, 0x40, 0x52, 0x00, 0xF7};
	m_pKeyboard->SetMIDIRouteMap (m_pRouteMap);

	m_pKeyboard->Send (pInit, sizeof pInit, 0);
	DisplayWrite ("MiniDexed", "", "On KeyLab 2", 0, 0);

	UpdateState ();

	pConfig->SetMIDIButtonCh (1);
	pConfig->SetMIDIButtonPrev (60);
	pConfig->SetMIDIButtonActionPrev("inc");
	pConfig->SetMIDIButtonNext (60);
	pConfig->SetMIDIButtonActionNext("dec");
	pConfig->SetMIDIButtonBack (63);
	pConfig->SetMIDIButtonActionBack("longpress");
	pConfig->SetMIDIButtonSelect (63);
	pConfig->SetMIDIButtonActionSelect("click");
	pConfig->SetMIDIButtonHome (63);
	pConfig->SetMIDIButtonActionHome("doubleclick");

	pUI->InitButtonsWithConfig(pConfig);
}

void CKeyLab2DawConnection::DisplayWrite (const char *pMenu, const char *pParam, const char *pValue, bool bArrowDown, bool bArrowUp)
{
	static const uint8_t pHdr[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x04, 0x00, 0x60, 0x01};
	ArturiaDisplayWrite (m_pKeyboard, pHdr, sizeof pHdr, 16, pMenu, pParam, pValue);
}

void CKeyLab2DawConnection::UpdateState ()
{
}

void CKeyLab2DawConnection::ShowNewCCValue (u8 ucCh, u8 ucCC, u8 ucValue)
{
	//static const uint8_t pEncoder[] = {0x04, 0x01, 0x60};
	//ArturiaShowNewCCValue (pKeyboard, pEncoder, ucCh, ucCC, ucValue);
}

class CKeyLabEsDawConnection : public CDAWConnection
{
public:
	CKeyLabEsDawConnection (CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard, CConfig *pConfig, CUserInterface *pUI);
	void DisplayWrite (const char *pMenu, const char *pParam, const char *pValue, bool bArrowDown, bool bArrowUp) override;
	void UpdateState () override;
	void ShowNewCCValue (u8 ucCh, u8 ucCC, u8 ucValue) override;
private:
	CMiniDexed *m_pSynthesizer;
	CMIDIKeyboard *m_pKeyboard;
	TMIDIRouteMap m_pRouteMap[13] = {
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
		{1, 0, MIDI_CONTROL_CHANGE, 0x3C, 0xFF, 0, MIDI_CONTROL_CHANGE, 60, 0xFF}, // Main rotate
		{1, 0, MIDI_NOTE_ON, 0x54, 0xFF, 0, MIDI_CONTROL_CHANGE, 63, 0xFF}, // Main click
		{1, 0xFF, 0xFF, 0xFF, 0xFF, .bSkip = true}, // skip other messages on DAW cable
		{0xFF}, // Sentinel
	};
};

CKeyLabEsDawConnection::CKeyLabEsDawConnection (CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard, CConfig *pConfig, CUserInterface *pUI)
	:m_pSynthesizer (pSynthesizer), m_pKeyboard (pKeyboard)
{
	static const uint8_t pInit[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x02, 0x00, 0x40, 0x51, 0x00, 0xF7}; // init DAW to Mackie mode
	m_pKeyboard->SetMIDIRouteMap (m_pRouteMap);

	m_pKeyboard->Send (pInit, sizeof pInit, 0);
	DisplayWrite ("MiniDexed", "", "On KeyLab Essential", 0, 0);

	UpdateState ();

	pConfig->SetMIDIButtonCh (1);
	pConfig->SetMIDIButtonPrev (60);
	pConfig->SetMIDIButtonActionPrev("inc");
	pConfig->SetMIDIButtonNext (60);
	pConfig->SetMIDIButtonActionNext("dec");
	pConfig->SetMIDIButtonBack (63);
	pConfig->SetMIDIButtonActionBack("longpress");
	pConfig->SetMIDIButtonSelect (63);
	pConfig->SetMIDIButtonActionSelect("click");
	pConfig->SetMIDIButtonHome (63);
	pConfig->SetMIDIButtonActionHome("doubleclick");

	pUI->InitButtonsWithConfig(pConfig);
}

void CKeyLabEsDawConnection::DisplayWrite (const char *pMenu, const char *pParam, const char *pValue, bool bArrowDown, bool bArrowUp)
{
	static const uint8_t pHdr[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x04, 0x00, 0x60, 0x01};
	ArturiaDisplayWrite (m_pKeyboard, pHdr, sizeof pHdr, 16, pMenu, pParam, pValue);
}

void CKeyLabEsDawConnection::UpdateState ()
{
}

void CKeyLabEsDawConnection::ShowNewCCValue (u8 ucCh, u8 ucCC, u8 ucValue)
{
	//static const uint8_t pEncoder[] = {0x04, 0x01, 0x60};
	//ArturiaShowNewCCValue (pKeyboard, pEncoder, ucCh, ucCC, ucValue);
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

void CDAWController::ShowNewCCValue (u8 ucCh, u8 ucCC, u8 ucValue)
{
	if (m_pDAWConnection)
		m_pDAWConnection->ShowNewCCValue (ucCh, ucCC, ucValue);
}
