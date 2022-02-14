//
// kernel.cpp
//
#include "kernel.h"
#include <iostream>
#include <synth_dexed.h>

CKernel::CKernel (void)
:	CStdlibAppStdio ("minidexed"),
 	m_I2CMaster (CMachineInfo::Get ()->GetDevice (DeviceI2CMaster), TRUE),
	m_Dexed(16,SAMPLE_RATE,&mInterrupt, &m_I2CMaster)
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

	m_Dexed.activate();

	return TRUE;
}

CStdlibApp::TShutdownMode CKernel::Run (void)
{
	std::cout << "Hello MiniDexed!\n";

	while(42==42)
	{
		boolean bUpdated = mUSBHCI.UpdatePlugAndPlay ();

		m_Dexed.Process(bUpdated);
	}

	return ShutdownHalt;
}
