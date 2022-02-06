//
// kernel.cpp
//
#include "kernel.h"
#include <iostream>

CKernel::CKernel (void)
:	CStdlibAppStdio ("minidexed"),
 	m_I2CMaster (CMachineInfo::Get ()->GetDevice (DeviceI2CMaster), TRUE),
	m_MiniOrgan (&mInterrupt, &m_I2CMaster)
{
	mActLED.Blink (5);	// show we are alive
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

	if (!m_MiniOrgan.Initialize ())
	{
		return FALSE;
	}

	return TRUE;
}

CStdlibApp::TShutdownMode CKernel::Run (void)
{
	std::cout << "Hello MiniDexed!\n";

	m_MiniOrgan.Start ();

	while (m_MiniOrgan.IsActive ())
	{
		boolean bUpdated = mUSBHCI.UpdatePlugAndPlay ();

		m_MiniOrgan.Process (bUpdated);
	}

	return ShutdownHalt;
}
