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
#include <circle/gpiomanager.h>
#include <circle/i2cmaster.h>
#include <circle/multicore.h>
#include <circle/soundbasedevice.h>

class CMiniDexed : public CDexedAdapter
#ifdef ARM_ALLOW_MULTI_CORE
	, public CMultiCoreSupport
#endif
{
public:
	CMiniDexed (CConfig *pConfig, CInterruptSystem *pInterrupt,
		    CGPIOManager *pGPIOManager, CI2CMaster *pI2CMaster);

	bool Initialize (void);

	void Process (bool bPlugAndPlayUpdated);

#ifdef ARM_ALLOW_MULTI_CORE
	void Run (unsigned nCore);
#endif

	CSysExFileLoader *GetSysExFileLoader (void);

	void BankSelectLSB (unsigned nBankLSB);
	void ProgramChange (unsigned nProgram);
	void SetVolume (unsigned nVolume);

private:
	void ProcessSound (void);

private:
	CConfig *m_pConfig;

	CUserInterface m_UI;
	CSysExFileLoader m_SysExFileLoader;

	CMIDIKeyboard *m_pMIDIKeyboard[CConfig::MaxUSBMIDIDevices];
	CPCKeyboard m_PCKeyboard;
	CSerialMIDIDevice m_SerialMIDI;
	bool m_bUseSerial;

	CSoundBaseDevice *m_pSoundDevice;
	unsigned m_nQueueSizeFrames;

	CPerformanceTimer m_GetChunkTimer;
	bool m_bProfileEnabled;
};

#endif
