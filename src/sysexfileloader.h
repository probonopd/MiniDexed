//
// sysexfileloader.h
//
// See: https://github.com/asb2m10/dexed/blob/master/Documentation/sysex-format.txt
//
#ifndef _sysexfileloader_h
#define _sysexfileloader_h

#include <stdint.h>
#include <string>
#include <circle/macros.h>

class CSysExFileLoader		// Loader for DX7 .syx files
{
public:
	static const unsigned MaxVoiceBankID = 127;	// TODO? 16383
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

	void SelectVoiceBank (unsigned nBankID);	// 0 .. 127

	void GetVoice (unsigned nVoiceID,		// 0 .. 31
		       uint8_t *pVoiceData);		// returns unpacked format (156 bytes)

private:
	static void DecodePackedVoice (const uint8_t *pPackedData, uint8_t *pDecodedData);

private:
	std::string m_DirName;

	TVoiceBank *m_pVoiceBank[MaxVoiceBankID+1];

	unsigned m_nBankID;

	static uint8_t s_DefaultVoice[SizeSingleVoice];
};

#endif
