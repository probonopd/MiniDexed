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
#include <stdint.h>
#include <math.h>
#include <circle/interrupt.h>
#include <circle/i2cmaster.h>
#include <circle/types.h>
#include <circle/pwmsoundbasedevice.h>
#include <circle/i2ssoundbasedevice.h>
#include <circle/hdmisoundbasedevice.h>
#include "config.h"
#include "sysexfileloader.h"
#include "midikeyboard.h"
#include "pckeyboard.h"
#include "serialmididevice.h"
#include "perftimer.h"
#include "userinterface.h"

class CMiniDexed : public CDexedAdapter
{
  public:
    CMiniDexed(CConfig *pConfig, CInterruptSystem *pInterrupt)
:   CDexedAdapter (CConfig::MaxNotes, pConfig->GetSampleRate ()),
    m_MIDIKeyboard (this, pConfig),
    m_PCKeyboard (this),
    m_SerialMIDI (this, pInterrupt, pConfig),
    m_bUseSerial (FALSE),
    m_pConfig (pConfig),
    m_UI (this, pConfig),
    m_GetChunkTimer ("GetChunk", 1000000U * pConfig->GetChunkSize ()/2 / pConfig->GetSampleRate ())
    {
    };

    virtual bool Initialize (void);
    void Process(boolean bPlugAndPlayUpdated);

    void BankSelectLSB (unsigned nBankLSB);
    void ProgramChange (unsigned program);

  private:
    CMIDIKeyboard m_MIDIKeyboard;
    CPCKeyboard m_PCKeyboard;
    CSerialMIDIDevice m_SerialMIDI;
    boolean m_bUseSerial;
    CSysExFileLoader m_SysExFileLoader;
    CConfig             *m_pConfig;
    CUserInterface	m_UI;

  protected:
    CPerformanceTimer m_GetChunkTimer;
};

class CMiniDexedPWM : public CMiniDexed, public CPWMSoundBaseDevice
{
  public:
    CMiniDexedPWM(CConfig *pConfig, CInterruptSystem *pInterrupt)
:   CMiniDexed(pConfig, pInterrupt),
    CPWMSoundBaseDevice (pInterrupt, pConfig->GetSampleRate (), pConfig->GetChunkSize ())
    {
    }

    bool Initialize (void);
    unsigned GetChunk (u32 *pBuffer, unsigned nChunkSize);
};

class CMiniDexedI2S : public CMiniDexed, public CI2SSoundBaseDevice
{
  public:
    CMiniDexedI2S(CConfig *pConfig, CInterruptSystem *pInterrupt, CI2CMaster *pI2CMaster)
:   CMiniDexed(pConfig, pInterrupt),
    CI2SSoundBaseDevice (pInterrupt, pConfig->GetSampleRate (), pConfig->GetChunkSize (),
			 FALSE, pI2CMaster, pConfig->GetDACI2CAddress ())
    {
    }

    bool Initialize (void);
    unsigned GetChunk (u32 *pBuffer, unsigned nChunkSize);
};

class CMiniDexedHDMI : public CMiniDexed, public CHDMISoundBaseDevice
{
  public:
    CMiniDexedHDMI(CConfig *pConfig, CInterruptSystem *pInterrupt)
:   CMiniDexed(pConfig, pInterrupt),
    CHDMISoundBaseDevice (pInterrupt, pConfig->GetSampleRate (), pConfig->GetChunkSize ())
    {
    }

    bool Initialize (void);
    unsigned GetChunk (u32 *pBuffer, unsigned nChunkSize);
};

#endif
