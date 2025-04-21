//
// udpmididevice.cpp
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

#include <circle/logger.h>
#include <cstring>
#include "udpmididevice.h"
#include <assert.h>

#define VIRTUALCABLE 24

LOGMODULE("udpmididevice");

CUDPMIDIDevice::CUDPMIDIDevice (CMiniDexed *pSynthesizer,
				      CConfig *pConfig, CUserInterface *pUI)
:	CMIDIDevice (pSynthesizer, pConfig, pUI),
	m_pSynthesizer (pSynthesizer),
	m_pConfig (pConfig)
{
	AddDevice ("udp");
}

CUDPMIDIDevice::~CUDPMIDIDevice (void)
{
	//m_pSynthesizer = 0;
}

boolean CUDPMIDIDevice::Initialize (void)
{
	m_pAppleMIDIParticipant = new CAppleMIDIParticipant(&m_Random, this);
	if (!m_pAppleMIDIParticipant->Initialize())
	{
		LOGERR("Failed to init RTP listener");
		delete m_pAppleMIDIParticipant;
		m_pAppleMIDIParticipant = nullptr;
	}
	else
		LOGNOTE("RTP Listener initialized");
	m_pUDPMIDIReceiver = new CUDPMIDIReceiver(this);
	if (!m_pUDPMIDIReceiver->Initialize())
	{
		LOGERR("Failed to init UDP MIDI receiver");
		delete m_pUDPMIDIReceiver;
		m_pUDPMIDIReceiver = nullptr;
	}
	else
		LOGNOTE("UDP MIDI receiver initialized");
	return true;
}

// Methods to handle MIDI events

void CUDPMIDIDevice::OnAppleMIDIDataReceived(const u8* pData, size_t nSize)
{
	MIDIMessageHandler(pData, nSize, VIRTUALCABLE);
}

void CUDPMIDIDevice::OnAppleMIDIConnect(const CIPAddress* pIPAddress, const char* pName)
{
	LOGNOTE("RTP Device connected");
}

void CUDPMIDIDevice::OnAppleMIDIDisconnect(const CIPAddress* pIPAddress, const char* pName)
{
	LOGNOTE("RTP Device disconnected");
}

void CUDPMIDIDevice::OnUDPMIDIDataReceived(const u8* pData, size_t nSize)
{
	MIDIMessageHandler(pData, nSize, VIRTUALCABLE);
}