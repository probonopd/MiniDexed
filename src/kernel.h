//
// kernel.h
//
#ifndef _kernel_h
#define _kernel_h

#include <circle_stdlib_app.h>

class CKernel : public CStdlibAppStdio
{
public:
	CKernel (void);

	bool Initialize (void);

	TShutdownMode Run (void);
};

#endif
