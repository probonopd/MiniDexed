//
// serialmididevice.cpp
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
#include "rtpmididevice.h"
#include <assert.h>

LOGMODULE("rtpmididevice");

CRTPMIDIDevice::CRTPMIDIDevice (CMiniDexed *pSynthesizer,
				      CConfig *pConfig, CUserInterface *pUI)
:	CMIDIDevice (pSynthesizer, pConfig, pUI),
	m_pConfig (pConfig)
	
	//m_Serial (pInterrupt, TRUE),
	//m_nSerialState (0),
	//m_nSysEx (0),
	//m_SendBuffer (&m_Serial)
{
	AddDevice ("rtpdummy");
}

CRTPMIDIDevice::~CRTPMIDIDevice (void)
{
	//m_nSerialState = 255;
}

boolean CRTPMIDIDevice::Initialize (void)
{
	m_pAppleMIDIParticipant = new CAppleMIDIParticipant(&m_Random, this);
	if (!m_pAppleMIDIParticipant->Initialize())
	{
		LOGERR("Failed to init RTP listener");
		return false; //continue without rtp midi
	}
	else
		LOGNOTE("RTP Listener initialized");
		return true;
}

// Methods to handle MIDI events

void CRTPMIDIDevice::OnAppleMIDIDataReceived(const u8* pData, size_t nSize)
{
	LOGNOTE("Recieved RTP MIDI Data");
	printf ("MIDI-RTP: %02X %02X\n",
				(unsigned) pData[0], (unsigned) pData[1]);
	MIDIMessageHandler(pData, nSize);
}

void CRTPMIDIDevice::OnAppleMIDIConnect(const CIPAddress* pIPAddress, const char* pName)
{
	LOGNOTE("RTP Device connected");
}

void CRTPMIDIDevice::OnAppleMIDIDisconnect(const CIPAddress* pIPAddress, const char* pName)
{
	// RemoveRTPDevice
}
