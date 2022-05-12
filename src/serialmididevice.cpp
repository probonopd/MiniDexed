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
#include <cstring>
#include "serialmididevice.h"
#include <assert.h>

CSerialMIDIDevice::CSerialMIDIDevice (CMiniDexed *pSynthesizer, CInterruptSystem *pInterrupt,
				      CConfig *pConfig)
:	CMIDIDevice (pSynthesizer, pConfig),
	m_pConfig (pConfig),
	m_Serial (pInterrupt, TRUE),
	m_nSerialState (0),
	m_nSysEx (0),
	m_SendBuffer (&m_Serial)
{
	AddDevice ("ttyS1");
}

CSerialMIDIDevice::~CSerialMIDIDevice (void)

{
	m_nSerialState = 255;
}

boolean CSerialMIDIDevice::Initialize (void)
{
	assert (m_pConfig);
	return m_Serial.Initialize (m_pConfig->GetMIDIBaudRate ());
}

void CSerialMIDIDevice::Process (void)
{
	m_SendBuffer.Update ();

	// Read serial MIDI data
	u8 Buffer[100];
	int nResult = m_Serial.Read (Buffer, sizeof Buffer);
	if (nResult <= 0)
	{
		if(nResult!=0)
			printf("Serial-Read: %d\n",nResult);
		return;
	}

        if (m_pConfig->GetMIDIDumpEnabled ())
	{
		printf("Incoming MIDI data:\n");
		for (uint16_t i = 0; i < nResult; i++)
		{
			if((i % 8) == 0)
				printf("%04d:",i);
			printf(" 0x%02x",Buffer[i]);
			if((i > 1 ) && (i % 8) == 0)
				printf("\n");
		}
		if((nResult % 8) != 0)
			printf("\n");
	}

	// Process MIDI messages
	// See: https://www.midi.org/specifications/item/table-1-summary-of-midi-message
	// "Running status" see: https://www.lim.di.unimi.it/IEEE/MIDI/SOT5.HTM#Running-	
	
	for (int i = 0; i < nResult; i++)
	{
		u8 uchData = Buffer[i];

		if(uchData == 0xF0)
		{
			// SYSEX found
			m_SerialMessage[m_nSysEx++]=uchData;
			continue;
		}

		if(m_nSysEx > 0)
		{
			m_SerialMessage[m_nSysEx++]=uchData;
			if ((uchData & 0x80) == 0x80 || m_nSysEx >= MAX_MIDI_MESSAGE)
			{
				if(uchData == 0xF7)
					MIDIMessageHandler (m_SerialMessage, m_nSysEx);
				m_nSysEx = 0;
			}
			continue;
		}
		else
		{
			switch (m_nSerialState)
			{
			case 0:
			MIDIRestart:
				if (   (uchData & 0x80) == 0x80		// status byte, all channels
				    && (uchData & 0xF0) != 0xF0)	// ignore system messages
				{
					m_SerialMessage[m_nSerialState++] = uchData;
				}
				break;
	
			case 1:
			case 2:
			DATABytes:
				if (uchData & 0x80)			// got status when parameter expected
				{
					m_nSerialState = 0;
	
					goto MIDIRestart;
				}
	
				m_SerialMessage[m_nSerialState++] = uchData;
	
				if (   (m_SerialMessage[0] & 0xE0) == 0xC0
				    || m_nSerialState == 3)		// message is complete
				{
					MIDIMessageHandler (m_SerialMessage, m_nSerialState);
	
					m_nSerialState = 4; // State 4 for test if 4th byte is a status byte or a data byte 
				}
	
				break;
			case 4:
				
				if ((uchData & 0x80) == 0)  // true data byte, false status byte
				{
					m_nSerialState = 1;
					goto DATABytes;
				}
				else 
				{
					m_nSerialState = 0;
					goto MIDIRestart; 
				}
				break;
			default:
				assert (0);
				break;
			}
		}
	}
}

void CSerialMIDIDevice::Send (const u8 *pMessage, size_t nLength, unsigned nCable)
{
	m_SendBuffer.Write (pMessage, nLength);
}
