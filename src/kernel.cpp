//
// kernel.cpp
//
#include "kernel.h"
#include <iostream>
#include <string.h>
#include <circle/logger.h>
#include "voices.c"

LOGMODULE ("kernel");

CKernel::CKernel (void)
:	CStdlibAppStdio ("minidexed"),
 	m_I2CMaster (CMachineInfo::Get ()->GetDevice (DeviceI2CMaster), TRUE),
	m_pDexed (0)
{
	// mActLED.Blink (5);	// show we are alive
}

CKernel::~CKernel(void)
{
}

bool CKernel::Initialize (void)
{
	if (!CStdlibAppStdio::Initialize ())
	{
		return FALSE;
	}

	// select the sound device
	const char *pSoundDevice = mOptions.GetSoundDevice ();
	if (strcmp (pSoundDevice, "sndi2s") == 0)
	{
		LOGNOTE ("I2S mode");

		m_pDexed = new CMiniDexedI2S (16, SAMPLE_RATE, &mInterrupt, &m_I2CMaster);
	}
	else if (strcmp (pSoundDevice, "sndhdmi") == 0)
	{
		LOGNOTE ("HDMI mode");

		m_pDexed = new CMiniDexedHDMI (16, SAMPLE_RATE, &mInterrupt);
	}
	else
	{
		LOGNOTE ("PWM mode");

		m_pDexed = new CMiniDexedPWM (16, SAMPLE_RATE, &mInterrupt);
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