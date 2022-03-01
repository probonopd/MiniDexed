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
#include <iostream>
#include <string.h>
#include <circle/logger.h>
#include <circle/synchronize.h>

LOGMODULE ("kernel");

CKernel *CKernel::s_pThis = 0;

CKernel::CKernel (void)
:	CStdlibAppStdio ("minidexed"),
	m_Config (&mFileSystem),
 	m_I2CMaster (CMachineInfo::Get ()->GetDevice (DeviceI2CMaster), TRUE),
	m_pDexed (0)
{
	s_pThis = this;

	// mActLED.Blink (5);	// show we are alive
}

CKernel::~CKernel(void)
{
	s_pThis = 0;
}

bool CKernel::Initialize (void)
{
	if (!CStdlibAppStdio::Initialize ())
	{
		return FALSE;
	}

	mLogger.RegisterPanicHandler (PanicHandler);

	m_Config.Load ();

	// select the sound device
	const char *pSoundDevice = m_Config.GetSoundDevice ();
	if (strcmp (pSoundDevice, "i2s") == 0)
	{
		LOGNOTE ("I2S mode");

		m_pDexed = new CMiniDexedI2S (&m_Config, &mInterrupt, &m_I2CMaster);
	}
	else if (strcmp (pSoundDevice, "hdmi") == 0)
	{
		LOGNOTE ("HDMI mode");

		m_pDexed = new CMiniDexedHDMI (&m_Config, &mInterrupt);
	}
	else
	{
		LOGNOTE ("PWM mode");

		m_pDexed = new CMiniDexedPWM (&m_Config, &mInterrupt);
	}

	if (!m_pDexed->Initialize ())
	{
		return FALSE;
	}

	return TRUE;
}

CStdlibApp::TShutdownMode CKernel::Run (void)
{
	std::cout << "Hello MiniDexed!\n";

	while(42==42)
	{
		boolean bUpdated = mUSBHCI.UpdatePlugAndPlay ();

		m_pDexed->Process(bUpdated);

		mScreen.Update ();
	}

	return ShutdownHalt;
}

void CKernel::PanicHandler (void)
{
	EnableIRQs ();

	s_pThis->mScreen.Update (4096);
}
