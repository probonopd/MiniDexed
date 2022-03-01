//
// kernel.cpp
//
#include "kernel.h"
#include <iostream>
#include <string.h>
#include <circle/logger.h>
#include <circle/synchronize.h>
#include "voices.c"

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
