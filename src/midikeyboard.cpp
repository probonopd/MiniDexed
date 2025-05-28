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
	m_pMIDIDevice (0)
{
	m_DeviceName.Format ("umidi%u", nInstance+1);

	AddDevice (m_DeviceName);
}

CMIDIKeyboard::~CMIDIKeyboard (void)
{
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
void CMIDIKeyboard::USBMIDIMessageHandler (u8 *pPacket, unsigned nLength, unsigned nCable, unsigned nDevice)
{
	assert (nDevice == m_nInstance + 1);

	if ((pPacket[0] == 0xF0) && (m_nSysExIdx == 0))
	{
		// Start of SysEx message
		for (unsigned i=0; i<USB_SYSEX_BUFFER_SIZE; i++) {
			m_SysEx[i] = 0;
		}
		for (unsigned i=0; i<nLength; i++) {
			m_SysEx[m_nSysExIdx++] = pPacket[i];
		}
		// Early check for manufacturer ID after at least 2 bytes
		if (m_nSysExIdx > 2 && m_SysEx[1] != 0x43 && m_SysEx[1] != 0x7D) {
			// LOGNOTE("Aborting SysEx assembly: manufacturer 0x%02X not Yamaha or MiniDexed", m_SysEx[1]);
			m_nSysExIdx = 0;
			// Do not process remaining bytes as MIDI events
		}
	}
	else if (m_nSysExIdx != 0)
	{
		for (unsigned i=0; i<nLength; i++) {
			if (pPacket[i] == 0xF8 || pPacket[i] == 0xFA || pPacket[i] == 0xFB || pPacket[i] == 0xFC || pPacket[i] == 0xFE || pPacket[i] == 0xFF) {
				MIDIMessageHandler (&pPacket[i], 1, nCable);
			}
			else if (m_nSysExIdx >= USB_SYSEX_BUFFER_SIZE) {
				// LOGERR("SysEx buffer overflow, resetting SysEx assembly");
				m_nSysExIdx = 0;
				break;
			}
			else if (pPacket[i] == 0xF7) {
				m_SysEx[m_nSysExIdx++] = pPacket[i];
				// Check manufacturer ID before passing to handler
				if (m_SysEx[1] == 0x43 || m_SysEx[1] == 0x7D) {
					MIDIMessageHandler (m_SysEx, m_nSysExIdx, nCable);
				} else {
					// LOGNOTE("Ignoring completed SysEx: manufacturer 0x%02X not Yamaha or MiniDexed", m_SysEx[1]);
				}
				m_nSysExIdx = 0;
			}
			else if ((pPacket[i] & 0x80) != 0) {
				// LOGERR("Unexpected status byte 0x%02X in SysEx, resetting", pPacket[i]);
				m_nSysExIdx = 0;
				break;
			}
			else {
				m_SysEx[m_nSysExIdx++] = pPacket[i];
				// Early check for manufacturer ID after at least 2 bytes
				if (m_nSysExIdx == 2 && m_SysEx[1] != 0x43 && m_SysEx[1] != 0x7D) {
					// LOGNOTE("Aborting SysEx assembly: manufacturer 0x%02X not Yamaha or MiniDexed", m_SysEx[1]);
					m_nSysExIdx = 0;
					// Do not process remaining bytes as MIDI events
				}
			}
		}
	}
	else
	{
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
