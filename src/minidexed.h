//
// minidexed.h
//
#ifndef _minidexed_h
#define _minidexed_h

#include "dexedadapter.h"
#include <stdint.h>
#include <math.h>
#include <circle/interrupt.h>
#include <circle/i2cmaster.h>
#include <circle/usb/usbmidi.h>
#include <circle/serial.h>
#include <circle/types.h>
#include <circle/pwmsoundbasedevice.h>
#include <circle/i2ssoundbasedevice.h>
#include <circle/hdmisoundbasedevice.h>
#include "config.h"
#include "sysexfileloader.h"
#include "pckeyboard.h"
#include "perftimer.h"
#include "userinterface.h"

class CMiniDexed : public CDexedAdapter
{
  public:
    CMiniDexed(CConfig *pConfig, CInterruptSystem *pInterrupt)
:   CDexedAdapter (CConfig::MaxNotes, pConfig->GetSampleRate ()),
    m_pMIDIDevice (0),
    m_PCKeyboard (this),
    m_Serial (pInterrupt, TRUE),
    m_bUseSerial (FALSE),
    m_nSerialState (0),
    m_GetChunkTimer ("GetChunk", 1000000U * pConfig->GetChunkSize ()/2 / pConfig->GetSampleRate ()),
    m_pConfig (pConfig),
    m_UI (this, pConfig)
    {
      s_pThis = this;
    };

    virtual bool Initialize (void);
    void Process(boolean bPlugAndPlayUpdated);
  protected:
    static void MIDIPacketHandler (unsigned nCable, u8 *pPacket, unsigned nLength);
    static void KeyStatusHandlerRaw (unsigned char ucModifiers, const unsigned char RawKeys[6]);
    void ChangeProgram(unsigned program);
    static void USBDeviceRemovedHandler (CDevice *pDevice, void *pContext);
    CUSBMIDIDevice     * volatile m_pMIDIDevice;
    CPCKeyboard        m_PCKeyboard;
    CSerialDevice m_Serial;
    boolean m_bUseSerial;
    unsigned m_nSerialState;
    u8 m_SerialMessage[3];
    CSysExFileLoader m_SysExFileLoader;
    CPerformanceTimer m_GetChunkTimer;
  private:
    CConfig             *m_pConfig;
    CUserInterface	m_UI;

    static CMiniDexed *s_pThis;
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
