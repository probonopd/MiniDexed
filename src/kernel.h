//
// kernel.h
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
#ifndef _kernel_h
#define _kernel_h

#include "circle_stdlib_app.h"
#include <circle/cputhrottle.h>
#include <circle/gpiomanager.h>
#include <circle/i2cmaster.h>
#include <circle/spimaster.h>
#include <circle/usb/usbcontroller.h>
// #include <circle/sched/scheduler.h>
#include "config.h"
#include "minidexed.h"

enum TShutdownMode
{
	ShutdownNone,
	ShutdownHalt,
	ShutdownReboot
};

class CKernel : public CStdlibAppStdio
{
public:
	CKernel (void);
	~CKernel (void);

	bool Initialize (void);

	TShutdownMode Run (void);

private:
	static void PanicHandler (void);

private:
	// do not change this order
	CConfig		m_Config;
	CCPUThrottle	m_CPUThrottle;
	CGPIOManager	m_GPIOManager;
	CI2CMaster	m_I2CMaster;
	CSPIMaster	*m_pSPIMaster;
	CMiniDexed	*m_pDexed;
	CUSBController *m_pUSB;
	// CScheduler	m_Scheduler;

	static CKernel *s_pThis;
};

#endif
