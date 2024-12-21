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

CMIDIKeyboard::CMIDIKeyboard (CMiniDexed *pSynthesizer, CConfig *pConfig, CUserInterface *pUI, unsigned nInstance)
:	CMIDIDevice (pSynthesizer, pConfig, pUI),
	m_nSysExIdx (0),
	m_nInstance (nInstance),
	m_pMIDIDevice (0),
	m_pDAWController (0)
{
	m_DeviceName.Format ("umidi%u", nInstance+1);

	AddDevice (m_DeviceName);

	if (pConfig->GetDAWControllerEnabled ())
		m_pDAWController = new CDAWController (pSynthesizer, this, pConfig, pUI);
}

CMIDIKeyboard::~CMIDIKeyboard (void)
{
	delete m_pDAWController;
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
			m_pMIDIDevice->RegisterPacketHandler (MIDIPacketHandler, this);

			m_pMIDIDevice->RegisterRemovedHandler (DeviceRemovedHandler, this);

			if (m_pDAWController)
				m_pDAWController->OnConnect();
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

void CMIDIKeyboard::SendDebounce (const u8 *pMessage, size_t nLength, unsigned nCable)
{
	TSendQueueEntry Entry = m_SendQueue.back ();
	if (Entry.nLength != nLength || Entry.nCable != nCable || memcmp (Entry.pMessage, pMessage, nLength) != 0)
		Send (pMessage, nLength, nCable);
}

// Most packets will be passed straight onto the main MIDI message handler
// but SysEx messages are multiple USB packets and so will need building up
// before parsing.
void CMIDIKeyboard::USBMIDIMessageHandler (u8 *pPacket, unsigned nLength, unsigned nCable, unsigned nDevice)
{
	assert (nDevice == m_nInstance + 1);

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

				if (m_pDAWController)
					m_pDAWController->MIDISysexHandler (m_SysEx, m_nSysExIdx, nCable);

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

void CMIDIKeyboard::MIDIPacketHandler (unsigned nCable, u8 *pPacket, unsigned nLength, unsigned nDevice, void *pParam)
{
	CMIDIKeyboard *pThis = static_cast<CMIDIKeyboard *> (pParam);
	assert (pThis != 0);

	pThis->USBMIDIMessageHandler (pPacket, nLength, nCable, nDevice);
}

void CMIDIKeyboard::DeviceRemovedHandler (CDevice *pDevice, void *pContext)
{
	CMIDIKeyboard *pThis = static_cast<CMIDIKeyboard *> (pContext);
	assert (pThis != 0);

	pThis->m_pMIDIDevice = 0;
}

void CMIDIKeyboard::DisplayWrite (const char *pMenu, const char *pParam, const char *pValue,
			   bool bArrowDown, bool bArrowUp)
{
	if (m_pMIDIDevice && m_pDAWController)
		m_pDAWController->DisplayWrite (pMenu, pParam, pValue, bArrowDown, bArrowUp);
}

void CMIDIKeyboard::UpdateDAWState (void)
{
	if (m_pMIDIDevice && m_pDAWController)
		m_pDAWController->UpdateState ();
}

void CMIDIKeyboard::MIDIListener (u8 ucCable, u8 ucChannel, u8 ucType, u8 ucP1, u8 ucP2)
{
	if (m_pDAWController)
		m_pDAWController->MIDIListener (ucCable, ucChannel, ucType, ucP1, ucP2);
}
