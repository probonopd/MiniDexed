//
// kernel.h
//
#ifndef _kernel_h
#define _kernel_h

#include "circle_stdlib_app.h"
#include <circle/i2cmaster.h>
#include "minidexed.h"
#include <display/hd44780device.h>

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
	void LCDWrite (const char *pString);

private:
	// do not change this order
	CI2CMaster		m_I2CMaster;
	CMiniDexed		*m_pDexed;
	CHD44780Device	m_LCD;
};

#endif