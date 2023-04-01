//
// sysexfileloader.h
//
// See: https://github.com/asb2m10/dexed/blob/master/Documentation/sysex-format.txt
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
#ifndef _sysexfileloader_h
#define _sysexfileloader_h

#include <stdint.h>
#include <string>
#include <circle/macros.h>

class CSysExFileLoader		// Loader for DX7 .syx files
{
public:
	static const unsigned MaxVoiceBankID = 16383; // i.e. 14-bit MSB/LSB value between 0 and 16383
	static const unsigned VoicesPerBank = 32;
	static const size_t SizePackedVoice = 128;
	static const size_t SizeSingleVoice = 156;

	struct TVoiceBank
	{
		uint8_t StatusStart;	// 0xF0
		uint8_t CompanyID;	// 0x43
		uint8_t SubStatus;	// 0x00
		uint8_t Format;		// 0x09
		uint8_t ByteCountMS;	// 0x20
		uint8_t ByteCountLS;	// 0x00

		uint8_t Voice[VoicesPerBank][SizePackedVoice];

		uint8_t Checksum;
		uint8_t StatusEnd;	// 0xF7
	}
	PACKED;

public:
	CSysExFileLoader (const char *pDirName = "/sysex");
	~CSysExFileLoader (void);

	void Load (void);

	std::string GetBankName (unsigned nBankID);	// 0 .. MaxVoiceBankID
	unsigned GetNumHighestBank (); // 0 .. MaxVoiceBankID
	bool     IsValidBank (unsigned nBankID);
	unsigned GetNextBankUp (unsigned nBankID);
	unsigned GetNextBankDown (unsigned nBankID);

	void GetVoice (unsigned nBankID,		// 0 .. MaxVoiceBankID
		       unsigned nVoiceID,		// 0 .. 31
		       uint8_t *pVoiceData);		// returns unpacked format (156 bytes)

private:
	static void DecodePackedVoice (const uint8_t *pPackedData, uint8_t *pDecodedData);

private:
	std::string m_DirName;
	
	unsigned m_nNumHighestBank;

	TVoiceBank *m_pVoiceBank[MaxVoiceBankID+1];
	std::string m_BankFileName[MaxVoiceBankID+1];

	static uint8_t s_DefaultVoice[SizeSingleVoice];
};

#endif
