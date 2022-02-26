//
// kernel.cpp
//
#include "kernel.h"
#include <iostream>
#include <string.h>
#include <circle/logger.h>
#include "voices.c"

uint8_t fmpiano_sysex[156] = {
  95, 29, 20, 50, 99, 95, 00, 00, 41, 00, 19, 00, 00, 03, 00, 06, 79, 00, 01, 00, 14, // OP6 eg_rate_1-4, level_1-4, kbd_lev_scl_brk_pt, kbd_lev_scl_lft_depth, kbd_lev_scl_rht_depth, kbd_lev_scl_lft_curve, kbd_lev_scl_rht_curve, kbd_rate_scaling, amp_mod_sensitivity, key_vel_sensitivity, operator_output_level, osc_mode, osc_freq_coarse, osc_freq_fine, osc_detune
  95, 20, 20, 50, 99, 95, 00, 00, 00, 00, 00, 00, 00, 03, 00, 00, 99, 00, 01, 00, 00, // OP5
  95, 29, 20, 50, 99, 95, 00, 00, 00, 00, 00, 00, 00, 03, 00, 06, 89, 00, 01, 00, 07, // OP4
  95, 20, 20, 50, 99, 95, 00, 00, 00, 00, 00, 00, 00, 03, 00, 02, 99, 00, 01, 00, 07, // OP3
  95, 50, 35, 78, 99, 75, 00, 00, 00, 00, 00, 00, 00, 03, 00, 07, 58, 00, 14, 00, 07, // OP2
  96, 25, 25, 67, 99, 75, 00, 00, 00, 00, 00, 00, 00, 03, 00, 02, 99, 00, 01, 00, 10, // OP1
  94, 67, 95, 60, 50, 50, 50, 50,                                                     // 4 * pitch EG rates, 4 * pitch EG level
  04, 06, 00,                                                                         // algorithm, feedback, osc sync
  34, 33, 00, 00, 00, 04,                                                             // lfo speed, lfo delay, lfo pitch_mod_depth, lfo_amp_mod_depth, lfo_sync, lfo_waveform
  03, 24,                                                                             // pitch_mod_sensitivity, transpose
  70, 77, 45, 80, 73, 65, 78, 79, 00, 00                                              // 10 * char for name ("DEFAULT   ")
}; // FM-Piano


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

	// m_pDexed->loadVoiceParameters(voices_banks[0][0]);
	m_pDexed->loadVoiceParameters(fmpiano_sysex);
	m_pDexed->setTranspose(24);

	while(42==42)
	{
		boolean bUpdated = mUSBHCI.UpdatePlugAndPlay ();

		m_pDexed->Process(bUpdated);

		mScreen.Update ();
	}

	return ShutdownHalt;
}