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
#include "midichunker.h"
#include <circle/devicenameservice.h>
#include <circle/sched/scheduler.h>
#include <circle/timer.h>
#include <circle/logger.h>
#include <cstring>
#include <assert.h>
#include <unistd.h>
#include <vector>

LOGMODULE("midikeyboard");

CMIDIKeyboard::CMIDIKeyboard (CMiniDexed *pSynthesizer, CConfig *pConfig, CUserInterface *pUI, unsigned nInstance)
:	CMIDIDevice (pSynthesizer, pConfig, pUI),
	m_nSysExIdx (0),
	m_nInstance (nInstance),
	m_pMIDIDevice (0),
	m_HasQueuedSysEx(false)
{
	m_DeviceName.Format ("umidi%u", nInstance+1);

	AddDevice (m_DeviceName);
}

CMIDIKeyboard::~CMIDIKeyboard (void)
{
}

void CMIDIKeyboard::Process (boolean bPlugAndPlayUpdated)
{
    // Send any queued SysEx response in a safe context
    if (m_HasQueuedSysEx && m_pMIDIDevice) {
        // Pad to multiple of 4 bytes for USB MIDI event packets
        size_t sysexLen = m_QueuedSysEx.size();
        size_t paddedLen = (sysexLen + 3) & ~3; // round up to next multiple of 4
        if (paddedLen > sysexLen) {
            m_QueuedSysEx.resize(paddedLen, 0x00);
        }
        // Send in safe chunks to avoid USB lockup
        static constexpr size_t kUSBMIDIMaxChunk = 256; // or 512 if your stack allows
        size_t offset = 0;
        // Only send one chunk per Process() call to avoid blocking or watchdog reset
        if (offset < m_QueuedSysEx.size()) {
            size_t chunk = std::min(kUSBMIDIMaxChunk, m_QueuedSysEx.size() - offset);
            LOGNOTE("SendEventPackets: about to send chunk at offset %u, length=%u", offset, chunk);
            m_pMIDIDevice->SendEventPackets(m_QueuedSysEx.data() + offset, chunk);
            offset += chunk;
            // Save progress for next Process() call
            if (offset < m_QueuedSysEx.size()) {
                // Not done yet, keep queued SysEx and return
                m_QueuedSysEx.erase(m_QueuedSysEx.begin(), m_QueuedSysEx.begin() + chunk);
                return;
            }
        }
        m_QueuedSysEx.clear();
        m_HasQueuedSysEx = false;
    }

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

// Helper: Convert SysEx to USB MIDI event packets
std::vector<std::vector<uint8_t>> SysExToUSBMIDIPackets(const uint8_t* data, size_t length, unsigned cable)
{
    LOGNOTE("SysExToUSBMIDIPackets: length=%u, cable=%u", (unsigned)length, cable);
    std::vector<std::vector<uint8_t>> packets;
    size_t idx = 0;
    while (idx < length) {
        size_t remaining = length - idx;
        uint8_t cin;
        uint8_t packet[4] = {0};
        packet[0] = (uint8_t)(cable << 4); // Upper nibble: cable number, lower: CIN
        if (remaining >= 3) {
            if (idx == 0) {
                cin = 0x4; // SysEx Start or continue
            } else {
                cin = 0x4; // SysEx continue
            }
            packet[0] |= cin;
            packet[1] = data[idx];
            packet[2] = data[idx+1];
            packet[3] = data[idx+2];
            LOGNOTE("  Packet: [%02X %02X %02X %02X] (idx=%u)", packet[0], packet[1], packet[2], packet[3], (unsigned)idx);
            idx += 3;
        } else if (remaining == 2) {
            cin = 0x6; // SysEx ends with 2 bytes
            packet[0] |= cin;
            packet[1] = data[idx];
            packet[2] = data[idx+1];
            packet[3] = 0;
            LOGNOTE("  Packet: [%02X %02X %02X %02X] (last 2 bytes)", packet[0], packet[1], packet[2], packet[3]);
            idx += 2;
        } else if (remaining == 1) {
            cin = 0x5; // SysEx ends with 1 byte
            packet[0] |= cin;
            packet[1] = data[idx];
            packet[2] = 0;
            packet[3] = 0;
            LOGNOTE("  Packet: [%02X %02X %02X %02X] (last 1 byte)", packet[0], packet[1], packet[2], packet[3]);
            idx += 1;
        }
        packets.push_back({packet[0], packet[1], packet[2], packet[3]});
    }
    LOGNOTE("SysExToUSBMIDIPackets: total packets=%u", (unsigned)packets.size());
    return packets;
}

void CMIDIKeyboard::Send(const u8 *pMessage, size_t nLength, unsigned nCable)
{
    // NOTE: For USB MIDI, we do NOT use MIDISysExChunker for SysEx sending.
    // The chunker splits SysEx into arbitrary chunks for traditional MIDI (e.g., serial/DIN),
    // but USB MIDI requires SysEx to be split into 4-byte USB MIDI event packets with specific CIN headers.
    // Therefore, for USB MIDI, we packetize SysEx according to the USB MIDI spec and send with SendEventPackets().
    // See: https://www.usb.org/sites/default/files/midi10.pdf (USB MIDI 1.0 spec)
    // This is why the chunker is bypassed for USB MIDI SysEx sending.

    // Check for valid SysEx
    if (nLength >= 2 && pMessage[0] == 0xF0 && pMessage[nLength-1] == 0xF7 && m_pMIDIDevice) {
        // Convert to USB MIDI event packets and send directly
        auto packets = SysExToUSBMIDIPackets(pMessage, nLength, nCable);
        std::vector<uint8_t> flat;
        for (const auto& pkt : packets) {
            flat.insert(flat.end(), pkt.begin(), pkt.end());
        }
        m_QueuedSysEx = flat;
        m_HasQueuedSysEx = true;
        return;
    }
    // Not a SysEx, send as-is
    TSendQueueEntry Entry;
    Entry.pMessage = new u8[nLength];
    Entry.nLength = nLength;
    Entry.nCable = nCable;
    memcpy(Entry.pMessage, pMessage, nLength);
    m_SendQueue.push(Entry);
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
