//
// userinterface.cpp
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
#include "userinterface.h"
#include "minidexed.h"
#include <circle/logger.h>
#include <circle/string.h>
#include <circle/startup.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <assert.h>

LOGMODULE ("ui");

CUserInterface::CUserInterface (CMiniDexed *pMiniDexed, CGPIOManager *pGPIOManager, CConfig *pConfig)
:	m_pMiniDexed (pMiniDexed),
	m_pGPIOManager (pGPIOManager),
	m_pConfig (pConfig),
	m_pLCD (0),
	m_pLCDBuffered (0),
	m_pRotaryEncoder (0),
	m_UIMode (UIModeVoiceSelect),
	m_nTG (0)
{
	for (unsigned nTG = 0; nTG < CConfig::ToneGenerators; nTG++)
	{
		m_nBank[nTG] = 0;
		m_nProgram[nTG] = 0;
		m_nVolume[nTG] = 0;
		m_nPan[nTG] = 64;
		m_nMasterTune[nTG] = 0;
		m_uchMIDIChannel[nTG] = CMIDIDevice::Disabled;
	}
}

CUserInterface::~CUserInterface (void)
{
	delete m_pRotaryEncoder;
	delete m_pLCDBuffered;
	delete m_pLCD;
}

bool CUserInterface::Initialize (void)
{
	assert (m_pConfig);

	if (m_pConfig->GetLCDEnabled ())
	{
		m_pLCD = new CHD44780Device (CConfig::LCDColumns, CConfig::LCDRows,
					     m_pConfig->GetLCDPinData4 (),
					     m_pConfig->GetLCDPinData5 (),
					     m_pConfig->GetLCDPinData6 (),
					     m_pConfig->GetLCDPinData7 (),
					     m_pConfig->GetLCDPinEnable (),
					     m_pConfig->GetLCDPinRegisterSelect (),
					     m_pConfig->GetLCDPinReadWrite ());
		assert (m_pLCD);

		if (!m_pLCD->Initialize ())
		{
			return false;
		}

		m_pLCDBuffered = new CWriteBufferDevice (m_pLCD);
		assert (m_pLCDBuffered);

		LCDWrite ("\x1B[?25l\x1B""d+");		// cursor off, autopage mode

		LOGDBG ("LCD initialized");
	}

	if (m_pConfig->GetEncoderEnabled ())
	{
		m_pRotaryEncoder = new CKY040 (m_pConfig->GetEncoderPinClock (),
					       m_pConfig->GetEncoderPinData (),
					       m_pConfig->GetEncoderPinSwitch (),
					       m_pGPIOManager);
		assert (m_pRotaryEncoder);

		if (!m_pRotaryEncoder->Initialize ())
		{
			return false;
		}

		m_pRotaryEncoder->RegisterEventHandler (EncoderEventStub, this);

		LOGDBG ("Rotary encoder initialized");
	}

	return true;
}

void CUserInterface::Process (void)
{
	if (m_pLCDBuffered)
	{
		m_pLCDBuffered->Update ();
	}
}

void CUserInterface::BankSelected (unsigned nBankLSB, unsigned  nTG)
{
	assert (nBankLSB < 128);
	assert (nTG < CConfig::ToneGenerators);
	m_nBank[nTG] = nBankLSB;

	assert (m_pMiniDexed);
	std::string BankName = m_pMiniDexed->GetSysExFileLoader ()->GetBankName (nBankLSB);

	// MIDI numbering starts with 0, user interface with 1
	printf ("TG%u: Select voice bank %u: \"%s\"\n", nTG+1, nBankLSB+1, BankName.c_str ());

	if (   m_UIMode == UIModeBankSelect
	    && m_nTG == nTG)
	{
		CString TG;
		TG.Format ("TG%u", nTG+1);

		CString String;
		String.Format ("%u", nBankLSB+1);

		DisplayWrite (TG, "BANK", String, BankName.c_str ());
	}
}

void CUserInterface::ProgramChanged (unsigned nProgram, unsigned  nTG)
{
	assert (nProgram < 128);
	assert (nTG < CConfig::ToneGenerators);
	m_nProgram[nTG] = nProgram;

	nProgram++;	// MIDI numbering starts with 0, user interface with 1

	assert (m_pMiniDexed);
	std::string VoiceName = m_pMiniDexed->GetVoiceName (nTG);

	printf ("TG%u: Loading voice %u: \"%s\"\n", nTG+1, nProgram, VoiceName.c_str ());

	if (   m_UIMode == UIModeVoiceSelect
	    && m_nTG == nTG)
	{
		CString TG;
		TG.Format ("TG%u", nTG+1);

		CString String;
		String.Format ("%u", nProgram);

		DisplayWrite (TG, "VOICE", String, VoiceName.c_str ());
	}
}

void CUserInterface::VolumeChanged (unsigned nVolume, unsigned  nTG)
{
	assert (nVolume < 128);
	assert (nTG < CConfig::ToneGenerators);
	m_nVolume[nTG] = nVolume;

	if (   m_UIMode == UIModeVolume
	    && m_nTG == nTG)
	{
		CString TG;
		TG.Format ("TG%u", nTG+1);

		char VolumeBar[CConfig::LCDColumns+1];
		memset (VolumeBar, 0xFF, sizeof VolumeBar);	// 0xFF is the block character
		VolumeBar[nVolume * CConfig::LCDColumns / 127] = '\0';

		DisplayWrite (TG, "VOLUME", VolumeBar);
	}
}

void CUserInterface::PanChanged (unsigned nPan, unsigned  nTG)
{
	assert (nPan < 128);
	assert (nTG < CConfig::ToneGenerators);
	m_nPan[nTG] = nPan;

	if (   m_UIMode == UIModePan
	    && m_nTG == nTG)
	{
		CString TG;
		TG.Format ("TG%u", nTG+1);

		char PanMarker[CConfig::LCDColumns+1];
		memset (PanMarker, '.', CConfig::LCDColumns);
		PanMarker[CConfig::LCDColumns] = '\0';
		unsigned nIndex = nPan * CConfig::LCDColumns / 127;
		if (nIndex == CConfig::LCDColumns)
		{
			nIndex--;
		}
		PanMarker[nIndex] = '\xFF';

		DisplayWrite (TG, "PAN", PanMarker);
	}
}

void CUserInterface::MasterTuneChanged (int nMasterTune, unsigned  nTG)
{
	assert (-99 <= nMasterTune && nMasterTune <= 99);
	assert (nTG < CConfig::ToneGenerators);
	m_nMasterTune[nTG] = nMasterTune;

	if (   m_UIMode == UIModeMasterTune
	    && m_nTG == nTG)
	{
		CString TG;
		TG.Format ("TG%u", nTG+1);

		CString String;
		String.Format ("%d", nMasterTune);

		DisplayWrite (TG, "MASTER TUNE", "DETUNE", (const char *) String);
	}
}

void CUserInterface::MIDIChannelChanged (uint8_t uchChannel, unsigned  nTG)
{
	assert (nTG < CConfig::ToneGenerators);
	m_uchMIDIChannel[nTG] = uchChannel;

	if (   m_UIMode == UIModeMIDI
	    && m_nTG == nTG)
	{
		CString TG;
		TG.Format ("TG%u", nTG+1);

		CString String;
		switch (uchChannel)
		{
		case CMIDIDevice::OmniMode:	String = "OMNI";	break;
		case CMIDIDevice::Disabled:	String = "OFF";		break;

		default:
			String.Format ("%u", (unsigned) uchChannel+1);
			break;
		}

		DisplayWrite (TG, "MIDI", "CHANNEL", (const char *) String);
	}
}

void CUserInterface::DisplayWrite (const char *pInstance, const char *pMenu,
				   const char *pParam, const char *pValue)
{
	assert (pInstance);
	assert (pMenu);
	assert (pParam);

	// Do not show instance, if there is only one.
	if (CConfig::ToneGenerators == 1)
	{
		pInstance = "";
	}

	CString Msg ("\x1B[H");		// cursor home

	// first line
	Msg.Append (pInstance);

	size_t nLen = strlen (pInstance) + strlen (pMenu);
	if (nLen < CConfig::LCDColumns)
	{
		for (unsigned i = CConfig::LCDColumns-nLen; i > 0; i--)
		{
			Msg.Append (" ");
		}
	}

	Msg.Append (pMenu);

	// second line
	CString ParamValue (pParam);
	if (pValue)
	{
		ParamValue.Append ("=");
		ParamValue.Append (pValue);
	}

	Msg.Append (ParamValue);

	if (ParamValue.GetLength () < CConfig::LCDColumns)
	{
		Msg.Append ("\x1B[K");		// clear end of line
	}

	LCDWrite (Msg);
}

void CUserInterface::LCDWrite (const char *pString)
{
	if (m_pLCDBuffered)
	{
		m_pLCDBuffered->Write (pString, strlen (pString));
	}
}

void CUserInterface::EncoderEventHandler (CKY040::TEvent Event)
{
	int nStep = 0;

	switch (Event)
	{
	case CKY040::EventClockwise:
		nStep = 1;
		break;

	case CKY040::EventCounterclockwise:
		nStep = -1;
		break;

	case CKY040::EventSwitchClick:
		m_UIMode = static_cast <TUIMode> (m_UIMode+1);
		if (m_UIMode == UIModeUnknown)
		{
			m_UIMode = UIModeStart;
		}
		break;

	case CKY040::EventSwitchDoubleClick:
		if (++m_nTG == CConfig::ToneGenerators)
		{
			m_nTG = 0;
		}
		break;

	case CKY040::EventSwitchHold:
		if (m_pRotaryEncoder->GetHoldSeconds () >= 3)
		{
			delete m_pLCD;		// reset LCD

			reboot ();
		}
		else
		{
			m_UIMode = UIModeStart;
			m_nTG = 0;
		}
		break;

	default:
		return;
	}

	switch (m_UIMode)
	{
	case UIModeBankSelect:
		if (m_nBank[m_nTG] + nStep < 128)
		{
			m_pMiniDexed->BankSelectLSB (m_nBank[m_nTG] + nStep, m_nTG);
		}
		break;

	case UIModeVoiceSelect:
		if (m_nProgram[m_nTG] + nStep < 32)
		{
			m_pMiniDexed->ProgramChange (m_nProgram[m_nTG] + nStep, m_nTG);
		}
		break;

	case UIModeVolume: {
		const int Increment = 128 / CConfig::LCDColumns;

		int nVolume = m_nVolume[m_nTG] + nStep*Increment;
		if (nVolume < 0)
		{
			nVolume = 0;
		}
		else if (nVolume > 127)
		{
			nVolume = 127;
		}

		m_pMiniDexed->SetVolume (nVolume, m_nTG);
		} break;

	case UIModePan: {
		const int Increment = 128 / CConfig::LCDColumns;

		int nPan = m_nPan[m_nTG] + nStep*Increment;
		if (nPan < 0)
		{
			nPan = 0;
		}
		else if (nPan > 127)
		{
			nPan = 127;
		}

		m_pMiniDexed->SetPan (nPan, m_nTG);
		} break;

	case UIModeMasterTune: {
		int nMasterTune = m_nMasterTune[m_nTG] + nStep;
		if (nMasterTune < -99)
		{
			nMasterTune = -99;
		}
		else if (nMasterTune > 99)
		{
			nMasterTune = 99;
		}

		m_pMiniDexed->SetMasterTune (nMasterTune, m_nTG);
		} break;

	case UIModeMIDI:
		if ((uint8_t) (m_uchMIDIChannel[m_nTG] + nStep) < CMIDIDevice::ChannelUnknown)
		{
			m_pMiniDexed->SetMIDIChannel (m_uchMIDIChannel[m_nTG] + nStep, m_nTG);
		}
		break;

	default:
		break;
	}
}

void CUserInterface::EncoderEventStub (CKY040::TEvent Event, void *pParam)
{
	CUserInterface *pThis = static_cast<CUserInterface *> (pParam);
	assert (pThis != 0);

	pThis->EncoderEventHandler (Event);
}
