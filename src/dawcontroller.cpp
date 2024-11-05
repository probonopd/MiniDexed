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
#define MIDI_DAW_ALGORITHM 2
#define MIDI_DAW_TOGGLE_MONO 3
#define MIDI_DAW_TOGGLE_PORTA_GLISS 4
#define MIDI_DAW_TOGGLE_TG 5
#define MIDI_DAW_SELECT_TG 6


static void ArturiaDisplayWrite (CMIDIKeyboard *pKeyboard, const u8 *pHdr, const unsigned nHdrSize,
	unsigned nLineMaxLen, const char *pMenu, const char *pParam, const char *pValue,
	const bool bArrowLeft, const bool bArrowRight)
{
	CString line1 (pParam);
	CString line2;

	if (strlen (pMenu))
	{
		size_t nLen = strlen (pParam) + strlen (pMenu);
		if (nLen < nLineMaxLen)
		{
			for (unsigned i = nLineMaxLen - nLen; i > 0; i--)
			{
				line1.Append (" ");
			}
		}

		line1.Append (pMenu);
	}

	if (bArrowLeft)
		line2.Append("< ");

	line2.Append(pValue);

	if (bArrowRight) {
		size_t nLen = strlen (line2) + 2;
		if (nLen < nLineMaxLen)
		{
			for (unsigned i = nLineMaxLen - nLen; i > 0; i--)
			{
				line2.Append (" ");
			}
		}

		line2.Append(" >");
	}

	int nLine1Len = strlen (line1);
	int nLine2Len = strlen (line2);
	int nOffset = 0;

	uint8_t pLines[nHdrSize + nLine1Len + 2 + nLine2Len  + 2];

	memcpy (pLines, pHdr, nHdrSize);
	nOffset += nHdrSize;

	memcpy (&pLines[nOffset], line1, nLine1Len);
	nOffset += nLine1Len;

	pLines[nOffset++] = 0x00;
	pLines[nOffset++] = 0x02;

	memcpy (&pLines[nOffset], line2, nLine2Len);
	nOffset += nLine2Len;

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
	{0x00, 0x00, 0x00},
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
	void MIDIListener (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2) override;
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
	void SetPadColor (TBankID BankID, TPadID PadID, u8 state, u8 state2);
	void SetPadColor (TBankID BankID, TPadID PadID, CColor color, u8 state);
	void SetPadColor (TBankID BankID, TPadID PadID, CColor color);

	void SetChannelAT (u8 ucValue);
	void SetVoice (u8 ucChannel, u8 ucVoice);
	void SetAlgorithm (u8 ucChannel, u8 ucAlgorithm);
	void ToggleMonoMode (u8 ucChannel);
	void TogglePortamentoGlisssando (u8 ucChannel);
	void ToggleTG (u8 ucTG);
	void SelectTG (u8 ucTG);

	CMiniDexed *m_pSynthesizer;
	CMIDIKeyboard *m_pKeyboard;
	CConfig *m_pConfig;

	u8 m_ucFirstTG = 0;
	
	const uint8_t m_pEncoder[3] = {0x04, 0x02, 0x60};
	TMIDIRoute m_pRouteMap[55] = {
		{0, 0, MIDI_CONTROL_CHANGE, 14, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader1
		{0, 0, MIDI_CONTROL_CHANGE, 15, 0xFF, 1, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader2
		{0, 0, MIDI_CONTROL_CHANGE, 30, 0xFF, 2, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader3
		{0, 0, MIDI_CONTROL_CHANGE, 31, 0xFF, 3, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader4

		{0, 0, MIDI_CONTROL_CHANGE, 86, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_FREQUENCY_CUTOFF, 0xFF}, // Knob1
		{0, 0, MIDI_CONTROL_CHANGE, 87, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_RESONANCE, 0xFF}, // Knob2
		{0, 0, MIDI_CONTROL_CHANGE, 89, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_REVERB_LEVEL, 0xFF}, // Knob3
		{0, 0, MIDI_CONTROL_CHANGE, 90, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_DETUNE_LEVEL, 0xFF}, // Knob4
		{0, 0, MIDI_CONTROL_CHANGE, 110, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_PORTAMENTO_TIME, 0xFF}, // Knob5
		{0, 0, MIDI_CONTROL_CHANGE, 111, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_VOICE, 0xFF}, // Knob6
		{0, 0, MIDI_CONTROL_CHANGE, 116, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_ALGORITHM, 0xFF}, // Knob7
		{0, 0, MIDI_CONTROL_CHANGE, 117, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_PAN_POSITION, 0xFF}, // Knob8

		{0, 9, MIDI_NOTE_ON, 36, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_TOGGLE_MONO, 0x7F}, // BankA Pad1
		{0, 9, MIDI_NOTE_OFF, 36, 0xFF, .bSkip=true},
	
		{0, 9, MIDI_NOTE_ON, 37, 0xFF, .bSkip=true}, // BankA Pad2
		{0, 9, MIDI_NOTE_OFF, 37, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_PORTAMENTO, 0x7F, .bToggle=true, .bSkipHandled=true},
		{0, 9, MIDI_AFTERTOUCH, 37, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_TOGGLE_PORTA_GLISS, 0x7F, .bSkipHandled=true},

		{0, 9, MIDI_NOTE_ON, 38, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_SOSTENUTO, 0x7F, .bToggle=true}, // BankA Pad3
		{0, 9, MIDI_NOTE_OFF, 38, 0xFF, .bSkip=true},

		{0, 9, MIDI_NOTE_ON, 39, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_BANK_SUSTAIN, 0x7F, .bToggle=true}, // BankA Pad4
		{0, 9, MIDI_NOTE_OFF, 39, 0xFF, .bSkip=true},

		{0, 9, MIDI_NOTE_ON, 40, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_ALL_SOUND_OFF, 0x7F}, // BankA Pad5
		{0, 9, MIDI_NOTE_OFF, 40, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_ALL_SOUND_OFF, 0x00},

		{0, 9, MIDI_NOTE_ON, 41, 0xFF, .bSkip=true}, // BankA Pad6
		{0, 9, MIDI_NOTE_OFF, 41, 0xFF, .bSkip=true},

		{0, 9, MIDI_NOTE_ON, 42, 0xFF, .bSkip=true}, // BankA Pad7
		{0, 9, MIDI_NOTE_OFF, 42, 0xFF, .bSkip=true},

		{0, 9, MIDI_NOTE_ON, 43, 0xFF, .bSkip=true}, // BankA Pad8
		{0, 9, MIDI_NOTE_OFF, 43, 0xFF, 0, MIDI_CHANNEL_AFTERTOUCH, 0x00, 0xFF},
		{0, 9, MIDI_AFTERTOUCH, 43, 0xFF, 0, MIDI_CHANNEL_AFTERTOUCH, TMIDIRoute::P2, 0xFF},

		{0, 9, MIDI_NOTE_ON, 44, 0xFF, .bSkip=true}, // BankB Pad1
		{0, 9, MIDI_NOTE_OFF, 44, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_TOGGLE_TG, 0, .bSkipHandled=true},
		{0, 9, MIDI_AFTERTOUCH, 44, 0x7F, 0, MIDI_DAW_CHANGE, MIDI_DAW_SELECT_TG, 0, .bSkipHandled=true},

		{0, 9, MIDI_NOTE_ON, 45, 0xFF, .bSkip=true}, // BankB Pad2
		{0, 9, MIDI_NOTE_OFF, 45, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_TOGGLE_TG, 1, .bSkipHandled=true},
		{0, 9, MIDI_AFTERTOUCH, 45, 0x7F, 0, MIDI_DAW_CHANGE, MIDI_DAW_SELECT_TG, 1, .bSkipHandled=true},

		{0, 9, MIDI_NOTE_ON, 46, 0xFF, .bSkip=true}, // BankB Pad3
		{0, 9, MIDI_NOTE_OFF, 46, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_TOGGLE_TG, 2, .bSkipHandled=true},
		{0, 9, MIDI_AFTERTOUCH, 46, 0x7F, 0, MIDI_DAW_CHANGE, MIDI_DAW_SELECT_TG, 2, .bSkipHandled=true},

		{0, 9, MIDI_NOTE_ON, 47, 0xFF, .bSkip=true}, // BankB Pad4
		{0, 9, MIDI_NOTE_OFF, 47, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_TOGGLE_TG, 3, .bSkipHandled=true},
		{0, 9, MIDI_AFTERTOUCH, 47, 0x7F, 0, MIDI_DAW_CHANGE, MIDI_DAW_SELECT_TG, 3, .bSkipHandled=true},

		{0, 9, MIDI_NOTE_ON, 48, 0xFF, .bSkip=true}, // BankB Pad5
		{0, 9, MIDI_NOTE_OFF, 48, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_TOGGLE_TG, 4, .bSkipHandled=true},
		{0, 9, MIDI_AFTERTOUCH, 48, 0x7F, 0, MIDI_DAW_CHANGE, MIDI_DAW_SELECT_TG, 4, .bSkipHandled=true},

		{0, 9, MIDI_NOTE_ON, 49, 0xFF, .bSkip=true}, // BankB Pad6
		{0, 9, MIDI_NOTE_OFF, 49, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_TOGGLE_TG, 5, .bSkipHandled=true},
		{0, 9, MIDI_AFTERTOUCH, 49, 0x7F, 0, MIDI_DAW_CHANGE, MIDI_DAW_SELECT_TG, 5, .bSkipHandled=true},

		{0, 9, MIDI_NOTE_ON, 50, 0xFF, .bSkip=true}, // BankB Pad7
		{0, 9, MIDI_NOTE_OFF, 50, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_TOGGLE_TG, 6, .bSkipHandled=true},
		{0, 9, MIDI_AFTERTOUCH, 50, 0x7F, 0, MIDI_DAW_CHANGE, MIDI_DAW_SELECT_TG, 6, .bSkipHandled=true},

		{0, 9, MIDI_NOTE_ON, 51, 0xFF, .bSkip=true}, // BankB Pad8
		{0, 9, MIDI_NOTE_OFF, 51, 0xFF, 0, MIDI_DAW_CHANGE, MIDI_DAW_TOGGLE_TG, 7, .bSkipHandled=true},
		{0, 9, MIDI_AFTERTOUCH, 51, 0x7F, 0, MIDI_DAW_CHANGE, MIDI_DAW_SELECT_TG, 7, .bSkipHandled=true},

		{0xFF}, // Sentinel
	};
};

CMiniLab3DawConnection::CMiniLab3DawConnection (CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard, CConfig *pConfig, CUserInterface *pUI)
	:m_pSynthesizer (pSynthesizer), m_pKeyboard (pKeyboard), m_pConfig (pConfig)
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
	static const uint8_t pHdr[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x04, 0x02, 0x60, 0x1f, 0x07, 0x01, 0x00, 0x00, 0x01, 0x00, 0x01};
	ArturiaDisplayWrite (m_pKeyboard, pHdr, sizeof pHdr, 18, pMenu, pParam, pValue, bArrowDown, bArrowUp);
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

	UpdateEncoder (0, maplong(m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterCutoff, m_ucFirstTG), 0, 99, 0, 127));
	UpdateEncoder (1, maplong(m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterResonance, m_ucFirstTG), 0, 99, 0, 127));
	UpdateEncoder (2, maplong(m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterReverbSend, m_ucFirstTG), 0, 99, 0, 127));
	UpdateEncoder (3, maplong(m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterMasterTune, m_ucFirstTG), -99, 99, 1, 127));
	UpdateEncoder (4, maplong(m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterPortamentoTime, m_ucFirstTG), 0, 99, 0, 127));
	UpdateEncoder (5, m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterProgram, m_ucFirstTG));
	UpdateEncoder (6, maplong(m_pSynthesizer->GetVoiceParameter (DEXED_ALGORITHM, CMiniDexed::NoOP, m_ucFirstTG), 0, 31, 0, 127));
	UpdateEncoder (7, m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterPan, m_ucFirstTG));

	UpdateMonoColor ();
	// TODO change the MIDIRouteMap's value also
	UpdatePortamentoColor ();

	for (unsigned i = 0; i < m_pConfig->GetToneGenerators(); ++i)
		UpdateTGColor (i);

	UpdateVolumeFaders ();
}

void CMiniLab3DawConnection::MIDIListener (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2)
{
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
		case MIDI_CC_BANK_SUSTAIN:
			SetPadColor (BankA, SustainPad, ucP2);
			break;
		case MIDI_CC_ALL_SOUND_OFF:
			SetPadColor (BankA, SoundOffPad, ucP2);
			break;
		}
		break;
	case MIDI_DAW_CHANGE:
		switch (ucP1)
		{
		case MIDI_DAW_VOICE:
			SetVoice (ucChannel, ucP2);
			break;
		case MIDI_DAW_ALGORITHM:
			SetAlgorithm (ucChannel, ucP2);
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
		}
		break;
	case MIDI_CHANNEL_AFTERTOUCH:
		SetChannelAT (ucP1);
		break;
	}
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

void CMiniLab3DawConnection::SetAlgorithm (u8 ucChannel, u8 ucAlgorithm)
{
	char line2[LINELEN];
	u8 algorithm = maplong (ucAlgorithm, 0, 127, 0, 31);

	for (unsigned i = 0; i < m_pConfig->GetToneGenerators(); ++i)
	{
		if (m_pSynthesizer->GetTGParameter (CMiniDexed::TGParameterEnabled, i) == 0 ||
			(m_pKeyboard->GetChannel (i) != ucChannel && m_pKeyboard->GetChannel (i) != CMIDIDevice::OmniMode))
			continue;
		m_pSynthesizer->SetVoiceParameter (DEXED_ALGORITHM, algorithm, CMiniDexed::NoOP, i);
	}

	snprintf(line2, LINELEN, "%d", algorithm + 1);
	ArturiaDisplayInfoWrite (m_pKeyboard, m_pEncoder, CT_KNOB, ucAlgorithm, "Algorithm", line2);
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
	UpdateTGColor (ucTG);
	UpdateState();
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
			UpdateTGColor (i);
		}
		ArturiaDisplayInfoWrite (m_pKeyboard, m_pEncoder, CT_PAD, 0x7F, "TG All", "On");
	} else {
		for (unsigned i = 0; i < m_pConfig->GetToneGenerators(); ++i) {
			if (i == ucTG) {
				m_pSynthesizer->setEnabled (true, i);
			} else {
				m_pSynthesizer->setEnabled (false, i);
				m_pSynthesizer->panic (false, ucTG);
			}
			UpdateTGColor (i);
		}
		snprintf(line1, LINELEN, "TG %d", ucTG + 1);
		ArturiaDisplayInfoWrite (m_pKeyboard, m_pEncoder, CT_PAD, 0x7F, line1, "Selected");
	}
	UpdateState();
}

class CKeyLabEs3DawConnection : public CDAWConnection
{
public:
	CKeyLabEs3DawConnection (CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard, CConfig *pConfig, CUserInterface *pUI);
	void DisplayWrite ( const char *pMenu, const char *pParam, const char *pValue, bool bArrowDown, bool bArrowUp) override;
	void UpdateState () override;
	void MIDIListener (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2) override;
private:
	void UpdateEncoder (uint8_t ucEncID, uint8_t ucValue);

	CMiniDexed *m_pSynthesizer;
	CMIDIKeyboard *m_pKeyboard;
	const uint8_t m_pEncoder[3] = {0x04, 0x01, 0x60};
	TMIDIRoute m_pRouteMap[16] = {
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

	m_pKeyboard->SetRouteMap (m_pRouteMap);

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
	ArturiaDisplayWrite (m_pKeyboard, pHdr, sizeof pHdr, 18, pMenu, pParam, pValue, bArrowDown, bArrowUp);
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

void CKeyLabEs3DawConnection::MIDIListener (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2)
{
	switch (ucType)
	{
	case MIDI_CONTROL_CHANGE:
		ArturiaShowNewCCValue (m_pKeyboard, m_pEncoder, ucChannel, ucP1, ucP2);
	}
}

class CKeyLab2DawConnection : public CDAWConnection
{
public:
	CKeyLab2DawConnection (CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard, CConfig *pConfig, CUserInterface *pUI);
	void DisplayWrite (const char *pMenu, const char *pParam, const char *pValue, bool bArrowDown, bool bArrowUp) override;
	void UpdateState () override;
	void MIDIListener (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2) override;
private:
	CMiniDexed *m_pSynthesizer;
	CMIDIKeyboard *m_pKeyboard;
	TMIDIRoute m_pRouteMap[13] = {
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
	m_pKeyboard->SetRouteMap (m_pRouteMap);

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
	ArturiaDisplayWrite (m_pKeyboard, pHdr, sizeof pHdr, 16, pMenu, pParam, pValue, bArrowDown, bArrowUp);
}

void CKeyLab2DawConnection::UpdateState ()
{
}

void CKeyLab2DawConnection::MIDIListener (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2)
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
	void MIDIListener (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2) override;
private:
	CMiniDexed *m_pSynthesizer;
	CMIDIKeyboard *m_pKeyboard;
	TMIDIRoute m_pRouteMap[13] = {
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
	m_pKeyboard->SetRouteMap (m_pRouteMap);

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
	ArturiaDisplayWrite (m_pKeyboard, pHdr, sizeof pHdr, 16, pMenu, pParam, pValue, bArrowDown, bArrowUp);
}

void CKeyLabEsDawConnection::UpdateState ()
{
}

void CKeyLabEsDawConnection::MIDIListener (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2)
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

void CDAWController::MIDIListener (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2)
{
	if (m_pDAWConnection)
		m_pDAWConnection->MIDIListener (ucCable, ucChannel, ucType, ucP1, ucP2);
}
