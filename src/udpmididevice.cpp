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
#include <circle/net/netsubsystem.h>
#include <circle/net/in.h>

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
	m_pAppleMIDIParticipant = new CAppleMIDIParticipant(&m_Random, this, m_pConfig->GetNetworkHostname());
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

	// UDP MIDI send socket setup (default: broadcast 255.255.255.255:1999)
	CNetSubSystem* pNet = CNetSubSystem::Get();
	m_pUDPSendSocket = new CSocket(pNet, IPPROTO_UDP);
	m_UDPDestAddress.Set(0xFFFFFFFF); // Broadcast by default
	m_UDPDestPort = 1999;

	return true;
}

// Methods to handle MIDI events

void CUDPMIDIDevice::OnAppleMIDIDataReceived(const u8* pData, size_t nSize)
{
	for (size_t i = 0; i < nSize; ++i) {
		u8 byte = pData[i];
		if (byte == 0xF0 && !m_SysExActive) {
			m_SysExActive = true;
			m_SysExLen = 0;
		}
		if (m_SysExActive) {
			if (m_SysExLen < MAX_MIDI_MESSAGE) {
				m_SysExBuffer[m_SysExLen++] = byte;
			}
			if (byte == 0xF7 || m_SysExLen >= MAX_MIDI_MESSAGE) {
				MIDIMessageHandler(m_SysExBuffer, m_SysExLen, VIRTUALCABLE);
				m_SysExActive = false;
				m_SysExLen = 0;
			}
			continue;
		}
		MIDIMessageHandler(&byte, 1, VIRTUALCABLE);
	}
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
	for (size_t i = 0; i < nSize; ++i) {
		u8 byte = pData[i];
		if (byte == 0xF0 && !m_SysExActive) {
			m_SysExActive = true;
			m_SysExLen = 0;
		}
		if (m_SysExActive) {
			if (m_SysExLen < MAX_MIDI_MESSAGE) {
				m_SysExBuffer[m_SysExLen++] = byte;
			}
			if (byte == 0xF7 || m_SysExLen >= MAX_MIDI_MESSAGE) {
				MIDIMessageHandler(m_SysExBuffer, m_SysExLen, VIRTUALCABLE);
				m_SysExActive = false;
				m_SysExLen = 0;
			}
			continue;
		}
		MIDIMessageHandler(&byte, 1, VIRTUALCABLE);
	}
}

void CUDPMIDIDevice::Send(const u8 *pMessage, size_t nLength, unsigned nCable)
{
    bool sentRTP = false;
    if (m_pAppleMIDIParticipant && m_pAppleMIDIParticipant->SendMIDIToHost(pMessage, nLength)) {
        sentRTP = true;
        LOGNOTE("Sent %u bytes to RTP-MIDI host", nLength);
    }
    if (!sentRTP && m_pUDPSendSocket) {
        int res = m_pUDPSendSocket->SendTo(pMessage, nLength, 0, m_UDPDestAddress, m_UDPDestPort);
        if (res < 0) {
            LOGERR("Failed to send %u bytes to UDP MIDI host", nLength);
        } else {
            LOGNOTE("Sent %u bytes to UDP MIDI host (broadcast)", nLength);
        }
    }
}