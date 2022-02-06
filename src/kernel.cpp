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
	return CStdlibAppStdio::Initialize ();
}

CStdlibApp::TShutdownMode CKernel::Run (void)
{
	std::cout << "Hello MiniDexed!\n";

	return ShutdownHalt;
}
