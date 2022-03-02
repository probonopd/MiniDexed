//
// minidexed.h
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
#ifndef _minidexed_h
#define _minidexed_h

#include "dexedadapter.h"
#include "config.h"
#include "userinterface.h"
#include "sysexfileloader.h"
#include "midikeyboard.h"
#include "pckeyboard.h"
#include "serialmididevice.h"
#include "perftimer.h"
#include <stdint.h>
#include <circle/types.h>
#include <circle/interrupt.h>
#include <circle/i2cmaster.h>
#include <circle/pwmsoundbasedevice.h>
#include <circle/i2ssoundbasedevice.h>
#include <circle/hdmisoundbasedevice.h>

class CMiniDexed : public CDexedAdapter
{
public:
	CMiniDexed (CConfig *pConfig, CInterruptSystem *pInterrupt);

	virtual bool Initialize (void);

	void Process (bool bPlugAndPlayUpdated);

	void BankSelectLSB (unsigned nBankLSB);
	void ProgramChange (unsigned nProgram);

private:
	CConfig *m_pConfig;

	CUserInterface m_UI;
	CSysExFileLoader m_SysExFileLoader;

	CMIDIKeyboard m_MIDIKeyboard;
	CPCKeyboard m_PCKeyboard;
	CSerialMIDIDevice m_SerialMIDI;
	bool m_bUseSerial;

protected:
	CPerformanceTimer m_GetChunkTimer;
	bool m_bProfileEnabled;
};

//// PWM //////////////////////////////////////////////////////////////////////

class CMiniDexedPWM : public CMiniDexed, public CPWMSoundBaseDevice
{
public:
	CMiniDexedPWM (CConfig *pConfig, CInterruptSystem *pInterrupt);

	bool Initialize (void);

	unsigned GetChunk (u32 *pBuffer, unsigned nChunkSize);
};

//// I2S //////////////////////////////////////////////////////////////////////

class CMiniDexedI2S : public CMiniDexed, public CI2SSoundBaseDevice
{
public:
	CMiniDexedI2S (CConfig *pConfig, CInterruptSystem *pInterrupt,
		       CI2CMaster *pI2CMaster);

	bool Initialize (void);

	unsigned GetChunk (u32 *pBuffer, unsigned nChunkSize);
};

//// HDMI /////////////////////////////////////////////////////////////////////

class CMiniDexedHDMI : public CMiniDexed, public CHDMISoundBaseDevice
{
public:
	CMiniDexedHDMI (CConfig *pConfig, CInterruptSystem *pInterrupt);

	bool Initialize (void);

	unsigned GetChunk (u32 *pBuffer, unsigned nChunkSize);
};

#endif
