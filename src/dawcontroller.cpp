//
// dawdisplay.cpp
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
#include <circle/string.h>

#include "dawcontroller.h"
#include "midikeyboard.h"
#include "minidexed.h"

static void ArturiaDisplayWrite (CMIDIKeyboard *pKeyboard, const u8 *pHdr, const unsigned nHdrSize, const char *pMenu, const char *pParam, const char *pValue)
{
	static unsigned L1MaxLen = 18;

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
	#define LINELEN 18

	char line1[LINELEN];
	char line2[LINELEN];

	switch (ucCC)
	{
	case MIDI_CC_VOLUME:
		snprintf(line1, LINELEN, "Volume %d", ucCh + 1);
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
	case MIDI_CC_ALL_SOUND_OFF:
		ArturiaDisplayInfoWrite (pKeyboard, pDisplayHdr, CT_PAD, ucValue, "All Sound Off", "");
		break;
	}
}

class CDAWConnection
{
public:
	virtual void DisplayWrite (CMIDIKeyboard *pKeyboard, const char *pMenu, const char *pParam,
				   const char *pValue, bool bArrowDown, bool bArrowUp) = 0;
	virtual void UpdateEncoders (CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard) = 0;
	virtual void ShowNewCCValue (CMIDIKeyboard *pKeyboard, u8 ucCh, u8 ucCC, u8 ucValue) = 0;
	virtual ~CDAWConnection (void) = default;
};

class CMiniLab3DawConnection : public CDAWConnection
{
public:
	CMiniLab3DawConnection (CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard, CConfig *pConfig, CUserInterface *pUI);
	void DisplayWrite (CMIDIKeyboard *pKeyboard, const char *pMenu, const char *pParam, const char *pValue, bool bArrowDown, bool bArrowUp) override;
	void UpdateEncoders (CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard) override;
	void ShowNewCCValue (CMIDIKeyboard *pKeyboard, u8 ucCh, u8 ucCC, u8 ucValue) override;
private:
	enum TPadID {
		SustainPad = 3,
		SoundOffPad = 4,
	};
	enum TBankID {
		BankA = 0x34,
		BankB = 0x44,
	};

	void UpdateEncoder (CMIDIKeyboard *pKeyboard, uint8_t ucEncID, uint8_t ucValue);
	void SetPadColor (CMIDIKeyboard *pKeyBoard, TBankID BankID, TPadID PadID, uint8_t r, uint8_t g, uint8_t b);

	uint8_t m_pEncoderCache[8];
};

CMiniLab3DawConnection::CMiniLab3DawConnection (CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard, CConfig *pConfig, CUserInterface *pUI)
{
	static const uint8_t pInit[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x02, 0x00, 0x40, 0x6A, 0x21, 0xF7};
	static TMIDIRouteMap map[] = {
		{0, MIDI_CONTROL_CHANGE, 14, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader1
		{0, MIDI_CONTROL_CHANGE, 15, 0xFF, 1, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader2
		{0, MIDI_CONTROL_CHANGE, 30, 0xFF, 2, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader3
		{0, MIDI_CONTROL_CHANGE, 31, 0xFF, 3, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader4
		{0, MIDI_CONTROL_CHANGE, 86, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_FREQUENCY_CUTOFF, 0xFF}, // Knob1
		{0, MIDI_CONTROL_CHANGE, 87, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_RESONANCE, 0xFF}, // Knob2
		{0, MIDI_CONTROL_CHANGE, 89, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_REVERB_LEVEL, 0xFF}, // Knob3
		{0, MIDI_CONTROL_CHANGE, 90, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_DETUNE_LEVEL, 0xFF}, // Knob4
		// {0, MIDI_CONTROL_CHANGE, 110, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_DETUNE_LEVEL, 0xFF}, // Knob5
		// {0, MIDI_CONTROL_CHANGE, 111, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_DETUNE_LEVEL, 0xFF}, // Knob6
		// {0, MIDI_CONTROL_CHANGE, 116, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_DETUNE_LEVEL, 0xFF}, // Knob7
		{0, MIDI_CONTROL_CHANGE, 117, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_PAN_POSITION, 0xFF}, // Knob8
		{9, MIDI_NOTE_ON, 39, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_BANK_SUSTAIN, 0x7F, .bToggle=true}, // BankA Pad4
		{9, MIDI_NOTE_OFF, 39, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_BANK_SUSTAIN, 0x00, .bSkip=true}, // BankA Pad4
		{9, MIDI_NOTE_ON, 40, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_ALL_SOUND_OFF, 0x7f}, // BankA Pad5
		{9, MIDI_NOTE_OFF, 40, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_ALL_SOUND_OFF, 0x00}, // BankA Pad5
		{0xFF}, // Sentinel
	};

	memset (m_pEncoderCache, 128, sizeof m_pEncoderCache);

	pKeyboard->SetMIDIRouteMap (map);

	pKeyboard->Send (pInit, sizeof pInit, 0);
	DisplayWrite (pKeyboard, "MiniDexed", "", "On MiniLab 3", 0, 0);

	SetPadColor (pKeyboard, BankA, SustainPad, 0x11, 0x3f, 0x11);
	SetPadColor (pKeyboard, BankA, SoundOffPad, 0x3f, 0x11, 0x11);

	UpdateEncoders (pSynthesizer, pKeyboard);

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

void CMiniLab3DawConnection::DisplayWrite (CMIDIKeyboard *pKeyboard, const char *pMenu, const char *pParam, const char *pValue, bool bArrowDown, bool bArrowUp)
{
	static const uint8_t pHdr[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x04, 0x02, 0x60, 0x12, 0x01};
	ArturiaDisplayWrite (pKeyboard, pHdr, sizeof pHdr, pMenu, pParam, pValue);
}

void CMiniLab3DawConnection::SetPadColor (CMIDIKeyboard *pKeyboard, TBankID BankID, TPadID PadID, uint8_t r, uint8_t g, uint8_t b)
{
	const uint8_t pSetPadColor[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x02, 0x02, 0x16, (uint8_t)(PadID + BankID), r, g, b, 0xF7};
	pKeyboard->Send (pSetPadColor, sizeof pSetPadColor, 0);
}

void CMiniLab3DawConnection::UpdateEncoder (CMIDIKeyboard *pKeyboard, uint8_t ucEncID, uint8_t ucValue)
{
	if (m_pEncoderCache[ucEncID] == ucValue)
		return;

	m_pEncoderCache[ucEncID] = ucValue;

	uint8_t pUpdateEncoder[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x21, 0x10, 0x00, ucEncID+=7, 0x00, ucValue, 0xF7};
	pKeyboard->Send (pUpdateEncoder, sizeof pUpdateEncoder, 0);
} 

void CMiniLab3DawConnection::UpdateEncoders (CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard)
{
	UpdateEncoder (pKeyboard, 0, maplong(pSynthesizer->GetTGParameter (CMiniDexed::TGParameterCutoff, 0), 0, 99, 0, 127));
	UpdateEncoder (pKeyboard, 1, maplong(pSynthesizer->GetTGParameter (CMiniDexed::TGParameterResonance, 0), 0, 99, 0, 127));
	UpdateEncoder (pKeyboard, 2, maplong(pSynthesizer->GetTGParameter (CMiniDexed::TGParameterReverbSend, 0), 0, 99, 0, 127));
	UpdateEncoder (pKeyboard, 3, maplong(pSynthesizer->GetTGParameter (CMiniDexed::TGParameterMasterTune, 0), -99, 99, 1, 127));
	UpdateEncoder (pKeyboard, 7, pSynthesizer->GetTGParameter (CMiniDexed::TGParameterPan, 0));
}

void CMiniLab3DawConnection::ShowNewCCValue (CMIDIKeyboard *pKeyboard, u8 ucCh, u8 ucCC, u8 ucValue)
{
	static const uint8_t pEncoder[] = {0x04, 0x02, 0x60};
	ArturiaShowNewCCValue (pKeyboard, pEncoder, ucCh, ucCC, ucValue);

	switch (ucCC)
	{
	case MIDI_CC_BANK_SUSTAIN:
		SetPadColor (pKeyboard, BankA, SustainPad, 0x11, ucValue ? 0x7f : 0x3f, 0x11);
		break;
	case MIDI_CC_ALL_SOUND_OFF:
		SetPadColor (pKeyboard, BankA, SoundOffPad, ucValue ? 0x7f : 0x3f, 0x11, 0x11);
		break;
	}
}

class CKeyLabEs3DawConnection : public CDAWConnection
{
public:
	CKeyLabEs3DawConnection (CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard, CConfig *pConfig, CUserInterface *pUI);
	void DisplayWrite (CMIDIKeyboard *pKeyboard, const char *pMenu, const char *pParam, const char *pValue, bool bArrowDown, bool bArrowUp) override;
	void UpdateEncoders (CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard) override;
	void ShowNewCCValue (CMIDIKeyboard *pKeyboard, u8 ucCh, u8 ucCC, u8 ucValue) override;
private:
	void UpdateEncoder (CMIDIKeyboard *pKeyboard, uint8_t ucEncID, uint8_t ucValue);

	uint8_t m_pEncoderCache[8];
};

CKeyLabEs3DawConnection::CKeyLabEs3DawConnection (CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard, CConfig *pConfig, CUserInterface *pUI)
{
	static const uint8_t pInit[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x02, 0x00, 0x40, 0x6A, 0x21, 0xF7};
	static TMIDIRouteMap map[] = {
		{0, MIDI_CONTROL_CHANGE, 105, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader1
		{0, MIDI_CONTROL_CHANGE, 106, 0xFF, 1, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader2
		{0, MIDI_CONTROL_CHANGE, 107, 0xFF, 2, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader3
		{0, MIDI_CONTROL_CHANGE, 108, 0xFF, 3, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader4
		{0, MIDI_CONTROL_CHANGE, 109, 0xFF, 4, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader5
		{0, MIDI_CONTROL_CHANGE, 110, 0xFF, 5, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader6
		{0, MIDI_CONTROL_CHANGE, 111, 0xFF, 6, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader7
		{0, MIDI_CONTROL_CHANGE, 112, 0xFF, 7, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader8
		//{0, MIDI_CONTROL_CHANGE, 113, 0xFF, 8, MIDI_CONTROL_CHANGE, MIDI_CC_VOLUME, 0xFF}, // Fader9
		{0, MIDI_CONTROL_CHANGE, 96, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_FREQUENCY_CUTOFF, 0xFF}, // Knob1
		{0, MIDI_CONTROL_CHANGE, 97, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_RESONANCE, 0xFF}, // Knob2
		{0, MIDI_CONTROL_CHANGE, 98, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_REVERB_LEVEL, 0xFF}, // Knob3
		{0, MIDI_CONTROL_CHANGE, 99, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_DETUNE_LEVEL, 0xFF}, // Knob4
		{0, MIDI_CONTROL_CHANGE, 100, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_PAN_POSITION, 0xFF}, // Knob5
		// {0, MIDI_CONTROL_CHANGE, 101, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_DETUNE_LEVEL, 0xFF}, // Knob6
		// {0, MIDI_CONTROL_CHANGE, 102, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_DETUNE_LEVEL, 0xFF}, // Knob7
		// {0, MIDI_CONTROL_CHANGE, 103, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_PAN_POSITION, 0xFF}, // Knob8
		// {0, MIDI_CONTROL_CHANGE, 104, 0xFF, 0, MIDI_CONTROL_CHANGE, MIDI_CC_PAN_POSITION, 0xFF}, // Knob9
		{0xFF}, // Sentinel
	};

	memset (m_pEncoderCache, 128, sizeof m_pEncoderCache);

	pKeyboard->SetMIDIRouteMap (map);

	pKeyboard->Send (pInit, sizeof pInit, 0);
	DisplayWrite (pKeyboard, "MiniDexed", "", "On KeyLab 3 Essential", 0, 0);

	UpdateEncoders (pSynthesizer, pKeyboard);

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

void CKeyLabEs3DawConnection::DisplayWrite (CMIDIKeyboard *pKeyboard, const char *pMenu, const char *pParam, const char *pValue, bool bArrowDown, bool bArrowUp)
{
	static const uint8_t pHdr[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x04, 0x01, 0x60, 0x12, 0x01};
	ArturiaDisplayWrite (pKeyboard, pHdr, sizeof pHdr, pMenu, pParam, pValue);
}

void CKeyLabEs3DawConnection::UpdateEncoder (CMIDIKeyboard *pKeyboard, uint8_t ucEncID, uint8_t ucValue)
{
	if (m_pEncoderCache[ucEncID] == ucValue)
		return;

	m_pEncoderCache[ucEncID] = ucValue;

	uint8_t pUpdateEncoder[] = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, 0x02, 0x0F, 0x40, ucEncID += 3, ucValue, 0xF7};
	pKeyboard->Send (pUpdateEncoder, sizeof pUpdateEncoder, 0);
} 

void CKeyLabEs3DawConnection::UpdateEncoders (CMiniDexed *pSynthesizer, CMIDIKeyboard *pKeyboard)
{
	UpdateEncoder (pKeyboard, 0, maplong(pSynthesizer->GetTGParameter (CMiniDexed::TGParameterCutoff, 0), 0, 99, 0, 127));
	UpdateEncoder (pKeyboard, 1, maplong(pSynthesizer->GetTGParameter (CMiniDexed::TGParameterResonance, 0), 0, 99, 0, 127));
	UpdateEncoder (pKeyboard, 2, maplong(pSynthesizer->GetTGParameter (CMiniDexed::TGParameterReverbSend, 0), 0, 99, 0, 127));
	UpdateEncoder (pKeyboard, 3, maplong(pSynthesizer->GetTGParameter (CMiniDexed::TGParameterMasterTune, 0), -99, 99, 1, 127));
	UpdateEncoder (pKeyboard, 4, pSynthesizer->GetTGParameter (CMiniDexed::TGParameterPan, 0));
}

void CKeyLabEs3DawConnection::ShowNewCCValue (CMIDIKeyboard *pKeyboard, u8 ucCh, u8 ucCC, u8 ucValue)
{
	static const uint8_t pEncoder[] = {0x04, 0x01, 0x60};
	ArturiaShowNewCCValue (pKeyboard, pEncoder, ucCh, ucCC, ucValue);
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
	static const uint8_t pMiniLab3[] =	{0xF0, 0x7E, 0x7F, 0x06, 0x02, 0x00, 0x20, 0x6B, 0x02, 0x00, 0x04, 0x04};
	static const uint8_t pKeyLabEs3_49[] =	{0xF0, 0x7E, 0x7F, 0x06, 0x02, 0x00, 0x20, 0x6B, 0x02, 0x00, 0x05, 0x72};
	static const uint8_t pKeyLabEs3_61[] =	{0xF0, 0x7E, 0x7F, 0x06, 0x02, 0x00, 0x20, 0x6B, 0x02, 0x00, 0x05, 0x74};
	static const uint8_t pKeyLabEs3_88[] =	{0xF0, 0x7E, 0x7F, 0x06, 0x02, 0x00, 0x20, 0x6B, 0x02, 0x00, 0x05, 0x78};

	if (nLength > sizeof pMiniLab3 && memcmp (pPacket, pMiniLab3, sizeof pMiniLab3) == 0)
	{
		m_pDAWConnection = new CMiniLab3DawConnection (m_pSynthesizer, m_pKeyboard, m_pConfig, m_pUI);
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
		m_pDAWConnection->DisplayWrite (m_pKeyboard, pMenu, pParam, pValue, bArrowDown, bArrowUp);
}

void CDAWController::UpdateEncoders (void)
{
	if (m_pDAWConnection)
		m_pDAWConnection->UpdateEncoders (m_pSynthesizer, m_pKeyboard);
}

void CDAWController::ShowNewCCValue (u8 ucCh, u8 ucCC, u8 ucValue)
{
	if (m_pDAWConnection)
		m_pDAWConnection->ShowNewCCValue (m_pKeyboard, ucCh, ucCC, ucValue);
}
