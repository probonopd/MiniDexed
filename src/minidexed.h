//
// minidexed.h
//
#ifndef _minidexed_h
#define _minidexed_h

#include <synth_dexed.h>
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
#include "sysexfileloader.h"
#include "pckeyboard.h"
#include <display/hd44780device.h>

#define SAMPLE_RATE	48000

#define CHUNK_SIZE	(256)
#define CHUNK_SIZE_HDMI	(384 * 6)

#define DAC_I2C_ADDRESS	0		// I2C slave address of the DAC (0 for auto probing)

// HD44780 LCD configuration
#define COLUMNS		16
#define ROWS		2
// GPIO pins (Brcm numbering)
#define EN_PIN		17		// Enable
#define RS_PIN		18		// Register Select
#define RW_PIN		19		// Read/Write (set to 0 if not connected)
#define D4_PIN		22		// Data 4
#define D5_PIN		23		// Data 5
#define D6_PIN		24		// Data 6
#define D7_PIN		25		// Data 7

class CMiniDexed : public Dexed
{
  public:
    CMiniDexed(uint8_t max_notes, uint16_t sample_rate, CInterruptSystem *pInterrupt)
:   Dexed(max_notes,(int)sample_rate),
    m_pMIDIDevice (0),
    m_PCKeyboard (this),
    m_Serial (pInterrupt, TRUE),
    m_bUseSerial (FALSE),
    m_nSerialState (0),
    m_LCD (COLUMNS, ROWS, D4_PIN, D5_PIN, D6_PIN, D7_PIN, EN_PIN, RS_PIN, RW_PIN)
    {
      s_pThis = this;
    };

    virtual bool Initialize (void);
    void Process(boolean bPlugAndPlayUpdated);
  private:
    void LCDWrite (const char *pString);
  protected:
    static void MIDIPacketHandler (unsigned nCable, u8 *pPacket, unsigned nLength);
    static void KeyStatusHandlerRaw (unsigned char ucModifiers, const unsigned char RawKeys[6]);
    static void ChangeProgram(unsigned program);
    static void USBDeviceRemovedHandler (CDevice *pDevice, void *pContext);
    CUSBMIDIDevice     * volatile m_pMIDIDevice;
    CPCKeyboard        m_PCKeyboard;
    CSerialDevice m_Serial;
    boolean m_bUseSerial;
    unsigned m_nSerialState;
    u8 m_SerialMessage[3];
    CSysExFileLoader m_SysExFileLoader;
  private:
    CHD44780Device	m_LCD;

    static CMiniDexed *s_pThis;
};

class CMiniDexedPWM : public CMiniDexed, public CPWMSoundBaseDevice
{
  public:
    CMiniDexedPWM(uint8_t max_notes, uint16_t sample_rate, CInterruptSystem *pInterrupt)
:   CMiniDexed(max_notes,(int)sample_rate, pInterrupt),
    CPWMSoundBaseDevice (pInterrupt, sample_rate, CHUNK_SIZE)
    {
    }

    bool Initialize (void);
    unsigned GetChunk (u32 *pBuffer, unsigned nChunkSize);
};

class CMiniDexedI2S : public CMiniDexed, public CI2SSoundBaseDevice
{
  public:
    CMiniDexedI2S(uint8_t max_notes, uint16_t sample_rate, CInterruptSystem *pInterrupt, CI2CMaster *pI2CMaster)
:   CMiniDexed(max_notes,(int)sample_rate, pInterrupt),
    CI2SSoundBaseDevice (pInterrupt, sample_rate, CHUNK_SIZE, FALSE, pI2CMaster, DAC_I2C_ADDRESS)
    {
    }

    bool Initialize (void);
    unsigned GetChunk (u32 *pBuffer, unsigned nChunkSize);
};

class CMiniDexedHDMI : public CMiniDexed, public CHDMISoundBaseDevice
{
  public:
    CMiniDexedHDMI(uint8_t max_notes, uint16_t sample_rate, CInterruptSystem *pInterrupt)
:   CMiniDexed(max_notes,(int)sample_rate, pInterrupt),
    CHDMISoundBaseDevice (pInterrupt, sample_rate, CHUNK_SIZE_HDMI)
    {
    }

    bool Initialize (void);
    unsigned GetChunk (u32 *pBuffer, unsigned nChunkSize);
};

#endif
