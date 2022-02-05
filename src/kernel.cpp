//
// kernel.cpp
//
#include "kernel.h"
#include <iostream>

CKernel::CKernel (void)
:	CStdlibAppStdio ("minidexed")
{
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
