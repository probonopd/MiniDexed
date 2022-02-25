//
// minidexed.cpp
//
#include "minidexed.h"
#include <circle/devicenameservice.h>
#include <stdio.h>

#define MIDI_DUMP

#define MIDI_NOTE_OFF	0b1000
#define MIDI_NOTE_ON	0b1001
#define MIDI_AFTERTOUCH		0xA0
#define MIDI_PROGRAM_CHANGE	0xC0
#define MIDI_PITCH_BEND		0xE0

CMiniDexed *CMiniDexed::s_pThis = 0;

extern uint8_t voices_bank[1][32][156];

bool CMiniDexed::Initialize (void)
{
  if (!m_Serial.Initialize(31250))
  {
    return false;
  }

  m_bUseSerial = true;

  activate();

  return true;
}

void CMiniDexed::Process(boolean bPlugAndPlayUpdated)
{
	if (m_pMIDIDevice != 0)
	{
		return;
	}

	if (bPlugAndPlayUpdated)
	{
		m_pMIDIDevice =
			(CUSBMIDIDevice *) CDeviceNameService::Get ()->GetDevice ("umidi1", FALSE);
		if (m_pMIDIDevice != 0)
		{
			m_pMIDIDevice->RegisterRemovedHandler (USBDeviceRemovedHandler);
			m_pMIDIDevice->RegisterPacketHandler (MIDIPacketHandler);

			return;
		}
	}

	m_PCKeyboard.Process (bPlugAndPlayUpdated);

	if (!m_bUseSerial)
	{
		return;
	}

	// Read serial MIDI data
	u8 Buffer[20];
	int nResult = m_Serial.Read (Buffer, sizeof Buffer);
	if (nResult <= 0)
	{
		return;
	}

	// Process MIDI messages
	// See: https://www.midi.org/specifications/item/table-1-summary-of-midi-message
	for (int i = 0; i < nResult; i++)
	{
		u8 uchData = Buffer[i];

		switch (m_nSerialState)
		{
		case 0:
		MIDIRestart:
			if ((uchData & 0xE0) == 0x80)		// Note on or off, all channels
			{
				m_SerialMessage[m_nSerialState++] = uchData;
			}
			break;

		case 1:
		case 2:
			if (uchData & 0x80)			// got status when parameter expected
			{
				m_nSerialState = 0;

				goto MIDIRestart;
			}

			m_SerialMessage[m_nSerialState++] = uchData;

			if (m_nSerialState == 3)		// message is complete
			{
				MIDIPacketHandler (0, m_SerialMessage, sizeof m_SerialMessage);

				m_nSerialState = 0;
			}
			break;

		default:
			assert (0);
			break;
		}
	}
}

void CMiniDexed::MIDIPacketHandler (unsigned nCable, u8 *pPacket, unsigned nLength)
{
	assert (s_pThis != 0);

	// The packet contents are just normal MIDI data - see
	// https://www.midi.org/specifications/item/table-1-summary-of-midi-message

#ifdef MIDI_DUMP
	switch (nLength)
	{
	case 1:
		printf ("MIDI %u: %02X\n", nCable, (unsigned) pPacket[0]);
		break;

	case 2:
		printf ("MIDI %u: %02X %02X\n", nCable,
			(unsigned) pPacket[0], (unsigned) pPacket[1]);
		break;

	case 3:
		printf ("MIDI %u: %02X %02X %02X\n", nCable,
			(unsigned) pPacket[0], (unsigned) pPacket[1], (unsigned) pPacket[2]);
		break;
	}
#endif

	if (pPacket[0] == MIDI_PROGRAM_CHANGE)
	{
		printf ("Loading voice %d\n", (unsigned) pPacket[1]);
		s_pThis->loadVoiceParameters(voices_bank[0][(unsigned) pPacket[1]]);
		// FIXME: The following 3 lines do not work yet
		char* buf_name = new char[11];
		s_pThis->getName(buf_name);
		printf ("%s\n", buf_name);
		return;
	}

	if (pPacket[0] == MIDI_PITCH_BEND)
	{
		s_pThis->setPitchbend((unsigned) pPacket[1]);
		
		return;
	}

	if (nLength < 3)
	{
		return;
	}

	u8 ucStatus    = pPacket[0];
	//u8 ucChannel   = ucStatus & 0x0F;
	u8 ucType      = ucStatus >> 4;
	u8 ucKeyNumber = pPacket[1];
	u8 ucVelocity  = pPacket[2];

	if (ucType == MIDI_NOTE_ON)
	{
	  s_pThis->keydown(ucKeyNumber,ucVelocity);
	}
	else if (ucType == MIDI_NOTE_OFF)
	{
	  s_pThis->keyup(ucKeyNumber);
	}
}

void CMiniDexed::USBDeviceRemovedHandler (CDevice *pDevice, void *pContext)
{
        if (s_pThis->m_pMIDIDevice == (CUSBMIDIDevice *) pDevice)
        {
                s_pThis->m_pMIDIDevice = 0;
        }
}

bool CMiniDexedPWM::Initialize (void)
{
  if (!CMiniDexed::Initialize())
  {
    return false;
  }

  return Start ();
}

unsigned CMiniDexedPWM::GetChunk(u32 *pBuffer, unsigned nChunkSize)
{
  unsigned nResult = nChunkSize;

  int16_t int16_buf[nChunkSize/2];

  getSamples(nChunkSize/2, int16_buf);

  for (unsigned i = 0; nChunkSize > 0; nChunkSize -= 2)		// fill the whole buffer
  {
    s32 nSample = int16_buf[i++];
    nSample += 32768;
    nSample *= GetRangeMax()/2;
    nSample /= 32768;

    *pBuffer++ = nSample;		// 2 stereo channels
    *pBuffer++ = nSample;
  }
  return(nResult);
};

bool CMiniDexedI2S::Initialize (void)
{
  if (!CMiniDexed::Initialize())
  {
    return false;
  }

  return Start ();
}

unsigned CMiniDexedI2S::GetChunk(u32 *pBuffer, unsigned nChunkSize)
{
  unsigned nResult = nChunkSize;

  int16_t int16_buf[nChunkSize/2];

  getSamples(nChunkSize/2, int16_buf);

  for (unsigned i = 0; nChunkSize > 0; nChunkSize -= 2)		// fill the whole buffer
  {
    s32 nSample = int16_buf[i++];
    nSample <<= 8;

    *pBuffer++ = nSample;		// 2 stereo channels
    *pBuffer++ = nSample;
  }
  return(nResult);
};

bool CMiniDexedHDMI::Initialize (void)
{
  if (!CMiniDexed::Initialize())
  {
    return false;
  }

  return Start ();
}

unsigned CMiniDexedHDMI::GetChunk(u32 *pBuffer, unsigned nChunkSize)
{
  unsigned nResult = nChunkSize;

  int16_t int16_buf[nChunkSize/2];
  unsigned nFrame = 0;

  getSamples(nChunkSize/2, int16_buf);

  for (unsigned i = 0; nChunkSize > 0; nChunkSize -= 2)		// fill the whole buffer
  {
    s32 nSample = int16_buf[i++];
    nSample <<= 8;

    nSample = ConvertIEC958Sample (nSample, nFrame);

    if (++nFrame == IEC958_FRAMES_PER_BLOCK)
      nFrame = 0;

    *pBuffer++ = nSample;		// 2 stereo channels
    *pBuffer++ = nSample;
  }
  return(nResult);
};
