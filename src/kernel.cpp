//
// kernel.cpp
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
#include "kernel.h"
#include <circle/logger.h>
#include <circle/synchronize.h>
#include <circle/gpiopin.h>
#include <assert.h>
#include <circle/usb/usbhcidevice.h>
#include "usbminidexedmidigadget.h"

#define NET_DEVICE_TYPE		NetDeviceTypeWLAN		// or: NetDeviceTypeWLAN

LOGMODULE ("kernel");

CKernel *CKernel::s_pThis = 0;

CKernel::CKernel (void)
:	
	CStdlibAppStdio ("minidexed"),
	m_Config (&mFileSystem),
	m_GPIOManager (&mInterrupt),
 	m_I2CMaster (CMachineInfo::Get ()->GetDevice (DeviceI2CMaster), TRUE),
	m_pSPIMaster (nullptr),
	m_pDexed (0)
{
	s_pThis = this;

	// mActLED.Blink (5);	// show we are alive
}

CKernel::~CKernel(void)
{
	s_pThis = 0;
}

static void SystemThrottledHandler (TSystemThrottledState CurrentState, void *pParam)
{
	LOGNOTE ("System Throttled");

	if (CurrentState & SystemStateUnderVoltageOccurred)
		LOGNOTE ("SystemStateUnderVoltageOccurred");
	if (CurrentState & SystemStateFrequencyCappingOccurred)
		LOGNOTE ("SystemStateFrequencyCappingOccurred");
	if (CurrentState & SystemStateThrottlingOccurred)
		LOGNOTE ("SystemStateThrottlingOccurred");
	if (CurrentState & SystemStateSoftTempLimitOccurred)
		LOGNOTE ("SystemStateSoftTempLimitOccurred");

	CCPUThrottle::Get ()->DumpStatus ();
}

bool CKernel::Initialize (void)
{
	if (!CStdlibAppStdio::Initialize ())
	{
		return FALSE;
	}

	mLogger.RegisterPanicHandler (PanicHandler);

	if (!m_GPIOManager.Initialize ())
	{
		return FALSE;
	}

	if (!m_I2CMaster.Initialize ())
	{
		return FALSE;
	}

	m_Config.Load ();

	m_CPUThrottle.DumpStatus ();
	m_CPUThrottle.RegisterSystemThrottledHandler ( SystemStateUnderVoltageOccurred |
		SystemStateFrequencyCappingOccurred | SystemStateThrottlingOccurred |
		SystemStateSoftTempLimitOccurred, SystemThrottledHandler, 0);
	
	unsigned nSPIMaster = m_Config.GetSPIBus();
	unsigned nSPIMode = m_Config.GetSPIMode();
	unsigned long nSPIClock = 1000 * m_Config.GetSPIClockKHz();
#if RASPPI<4
	// By default older RPI versions use SPI 0.
	// It is possible to build circle to support SPI 1 for
	// devices that use the 40-pin header, but that isn't
	// enabled at present...
	if (nSPIMaster == 0)
#else
	// RPI 4+ has several possible SPI Bus Configurations.
	// As mentioned above, SPI 1 is not built by default.
	// See circle/include/circle/spimaster.h
	if (nSPIMaster == 0 || nSPIMaster == 3 || nSPIMaster == 4 || nSPIMaster == 5 || nSPIMaster == 6)
#endif
	{
		unsigned nCPHA = (nSPIMode & 1) ? 1 : 0;
		unsigned nCPOL = (nSPIMode & 2) ? 1 : 0;
		m_pSPIMaster = new CSPIMaster (nSPIClock, nCPOL, nCPHA, nSPIMaster);
		if (!m_pSPIMaster->Initialize())
		{
			delete (m_pSPIMaster);
			m_pSPIMaster = nullptr;
		}
	}

	bool bUSBGadgetMode = false;
	if (m_Config.GetUSBGadget())
	{
		unsigned nUSBGadgetPin = m_Config.GetUSBGadgetPin();
		if (nUSBGadgetPin == 0)
		{
			// No hardware config option
			bUSBGadgetMode = true;
		}
		else
		{
			// State of USB Gadget Mode determined by state of the pin.
			// Pulled down = enable USB Gadget mode
			CGPIOPin usbGadgetPin(nUSBGadgetPin, GPIOModeInputPullUp);

			if (usbGadgetPin.Read() == 0)
			{
				bUSBGadgetMode = true;
			}
		}
	}

	if (bUSBGadgetMode)
	{
#if RASPPI==5
#warning No support for USB Gadget Mode on RPI 5 yet
#else
		// Run the USB stack in USB Gadget (device) mode
		m_pUSB = new CUSBMiniDexedMIDIGadget (&mInterrupt);
#endif
	}
	else
	{
		// Run the USB stack in USB Host (default) mode
		m_pUSB = new CUSBHCIDevice (&mInterrupt, &mTimer, TRUE);
	}
	m_Config.SetUSBGadgetMode(bUSBGadgetMode);

    if (!m_pUSB->Initialize ())
    {
		return FALSE;
    }
	
	m_pDexed = new CMiniDexed (&m_Config, &mInterrupt, &m_GPIOManager, &m_I2CMaster, m_pSPIMaster,
				   &mFileSystem);
	assert (m_pDexed);

	if (!m_pDexed->Initialize ())
	{
		return FALSE;
	}

	return TRUE;
}

CStdlibApp::TShutdownMode CKernel::Run (void)
{
	assert (m_pDexed);

	while (42 == 42)
	{
		boolean bUpdated = m_pUSB->UpdatePlugAndPlay ();

		m_pDexed->Process(bUpdated);

		if (mbScreenAvailable)
		{
			mScreen.Update ();
		}

		m_CPUThrottle.Update ();
	}

	return ShutdownHalt;
}

void CKernel::PanicHandler (void)
{
	EnableIRQs ();

	if (s_pThis->mbScreenAvailable)
	{
		s_pThis->mScreen.Update (4096);
	}
}
