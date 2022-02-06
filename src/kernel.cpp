//
// kernel.cpp
//
#include "kernel.h"
#include <iostream>

CKernel::CKernel (void)
:	CStdlibAppStdio ("minidexed")
{
	m_Screen (m_Options.GetWidth (), m_Options.GetHeight ()),
	m_Timer (&m_Interrupt),
	m_Logger (m_Options.GetLogLevel (), &m_Timer),
	m_I2CMaster (CMachineInfo::Get ()->GetDevice (DeviceI2CMaster), TRUE),
	m_USBHCI (&m_Interrupt, &m_Timer, TRUE),		// TRUE: enable plug-and-play
	m_MiniOrgan (&m_Interrupt, &m_I2CMaster)
	mActLED.Blink (5);	// show we are alive
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
