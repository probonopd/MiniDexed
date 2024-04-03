//
// midikeyboard.cpp
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
#include "midikeyboard.h"
#include <circle/devicenameservice.h>
#include <cstring>
#include <assert.h>

CMIDIKeyboard *CMIDIKeyboard::s_pThis[MaxInstances] = {0};

TMIDIPacketHandler * const CMIDIKeyboard::s_pMIDIPacketHandler[MaxInstances] =
{
	MIDIPacketHandler0,
	MIDIPacketHandler1,
	MIDIPacketHandler2,
	MIDIPacketHandler3
};

CMIDIKeyboard::CMIDIKeyboard (CMiniDexed *pSynthesizer, CConfig *pConfig, CUserInterface *pUI, unsigned nInstance)
:	CMIDIDevice (pSynthesizer, pConfig, pUI),
	m_nSysExIdx (0),
	m_nInstance (nInstance),
	m_pMIDIDevice (0)
{
	assert (m_nInstance < MaxInstances);
	s_pThis[m_nInstance] = this;

	m_DeviceName.Format ("umidi%u", nInstance+1);

	AddDevice (m_DeviceName);
}

CMIDIKeyboard::~CMIDIKeyboard (void)
{
	assert (m_nInstance < MaxInstances);
	s_pThis[m_nInstance] = 0;
}

void CMIDIKeyboard::Process (boolean bPlugAndPlayUpdated)
{
	while (!m_SendQueue.empty ())
	{
		TSendQueueEntry Entry = m_SendQueue.front ();
		m_SendQueue.pop ();

		if (m_pMIDIDevice)
		{
			m_pMIDIDevice->SendPlainMIDI (Entry.nCable, Entry.pMessage, Entry.nLength);
		}

		delete [] Entry.pMessage;
	}

	if (!bPlugAndPlayUpdated)
	{
		return;
	}

	if (m_pMIDIDevice == 0)
	{
		m_pMIDIDevice =
			(CUSBMIDIDevice *) CDeviceNameService::Get ()->GetDevice (m_DeviceName, FALSE);
		if (m_pMIDIDevice != 0)
		{
			assert (m_nInstance < MaxInstances);
			m_pMIDIDevice->RegisterPacketHandler (s_pMIDIPacketHandler[m_nInstance]);

			m_pMIDIDevice->RegisterRemovedHandler (DeviceRemovedHandler, this);
		}
	}
}

void CMIDIKeyboard::Send (const u8 *pMessage, size_t nLength, unsigned nCable)
{
	TSendQueueEntry Entry;
	Entry.pMessage = new u8[nLength];
	Entry.nLength = nLength;
	Entry.nCable = nCable;

	memcpy (Entry.pMessage, pMessage, nLength);

	m_SendQueue.push (Entry);
}

// Most packets will be passed straight onto the main MIDI message handler
// but SysEx messages are multiple USB packets and so will need building up
// before parsing.
void CMIDIKeyboard::USBMIDIMessageHandler (unsigned nInstance, u8 *pPacket, unsigned nLength, unsigned nCable)
{
	if ((pPacket[0] == 0xF0) && (m_nSysExIdx == 0))
	{
		// Start of SysEx message
		//printf("SysEx Start  Idx=%d, (%d)\n", m_nSysExIdx, nLength);
		for (unsigned i=0; i<USB_SYSEX_BUFFER_SIZE; i++) {
			m_SysEx[i] = 0;
		}
		for (unsigned i=0; i<nLength; i++) {
			m_SysEx[m_nSysExIdx++] = pPacket[i];
		}
	}
	else if (m_nSysExIdx != 0)
	{
		// Continue processing SysEx message
		//printf("SysEx Packet Idx=%d, (%d)\n", m_nSysExIdx, nLength);
		for (unsigned i=0; i<nLength; i++) {
			if (pPacket[i] == 0xF8 || pPacket[i] == 0xFA || pPacket[i] == 0xFB || pPacket[i] == 0xFC || pPacket[i] == 0xFE || pPacket[i] == 0xFF) {
				// Singe-byte System Realtime Messages can happen at any time!
				MIDIMessageHandler (&pPacket[i], 1, nCable);
			}
			else if (m_nSysExIdx >= USB_SYSEX_BUFFER_SIZE) {
				// Run out of space, so reset and ignore rest of the message
				m_nSysExIdx = 0;
				break;
			}
			else if (pPacket[i] == 0xF7) {
				// End of SysEx message
				m_SysEx[m_nSysExIdx++] = pPacket[i];
				//printf ("SysEx End    Idx=%d\n", m_nSysExIdx);
				MIDIMessageHandler (m_SysEx, m_nSysExIdx, nCable);
				// Reset ready for next time
				m_nSysExIdx = 0;
			}
			else if ((pPacket[i] & 0x80) != 0) {
				// Received another command, so reset processing as something has gone wrong
				//printf ("SysEx Reset\n");
				m_nSysExIdx = 0;
				break;
			}
			else
			{
				// Store the byte
				m_SysEx[m_nSysExIdx++] = pPacket[i];
			}
		}
	}
	else
	{
		// Assume it is a standard message
		MIDIMessageHandler (pPacket, nLength, nCable);
	}
}

void CMIDIKeyboard::MIDIPacketHandler0 (unsigned nCable, u8 *pPacket, unsigned nLength)
{
	assert (s_pThis[0] != 0);
	s_pThis[0]->USBMIDIMessageHandler (0, pPacket, nLength, nCable);
}

void CMIDIKeyboard::MIDIPacketHandler1 (unsigned nCable, u8 *pPacket, unsigned nLength)
{
	assert (s_pThis[1] != 0);
	s_pThis[1]->USBMIDIMessageHandler (1, pPacket, nLength, nCable);
}

void CMIDIKeyboard::MIDIPacketHandler2 (unsigned nCable, u8 *pPacket, unsigned nLength)
{
	assert (s_pThis[2] != 0);
	s_pThis[2]->USBMIDIMessageHandler (2, pPacket, nLength, nCable);
}

void CMIDIKeyboard::MIDIPacketHandler3 (unsigned nCable, u8 *pPacket, unsigned nLength)
{
	assert (s_pThis[3] != 0);
	s_pThis[3]->USBMIDIMessageHandler (3, pPacket, nLength, nCable);
}

void CMIDIKeyboard::DeviceRemovedHandler (CDevice *pDevice, void *pContext)
{
	CMIDIKeyboard *pThis = static_cast<CMIDIKeyboard *> (pContext);
	assert (pThis != 0);

	pThis->m_pMIDIDevice = 0;
}
