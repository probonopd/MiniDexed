//
// kernel.h
//
#ifndef _kernel_h
#define _kernel_h

#include "circle_stdlib_app.h"
#include <circle/i2cmaster.h>
#include "config.h"
#include "minidexed.h"

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
	static void PanicHandler (void);

private:
	// do not change this order
	CConfig			m_Config;
	CI2CMaster		m_I2CMaster;
	CMiniDexed		*m_pDexed;

	static CKernel *s_pThis;
};

#endif
