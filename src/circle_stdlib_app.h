/**
 * This file has been taken from the circle-stdlib project:
 *	https://github.com/smuehlst/circle-stdlib
 *
 * Convenience classes that package different levels
 * of functionality of Circle applications.
 *
 * Derive the kernel class of the application from one of
 * the CStdlibApp* classes and implement at least the
 * Run () method. Extend the Initalize () and Cleanup ()
 * methods if necessary.
 */
#ifndef _circle_stdlib_app_h
#define _circle_stdlib_app_h

#include <circle/actled.h>
#include <circle/string.h>
#include <circle/koptions.h>
#include <circle/devicenameservice.h>
#include <circle/nulldevice.h>
#include <circle/exceptionhandler.h>
#include <circle/interrupt.h>
#include <circle/screen.h>
#include <circle/serial.h>
#include <circle/writebuffer.h>
#include <circle/timer.h>
#include <circle/logger.h>
#include <SDCard/emmc.h>
#include <circle/input/console.h>
#include <circle/sched/scheduler.h>
#include <circle/net/netsubsystem.h>
#include <wlan/bcm4343.h>
#include <wlan/hostap/wpa_supplicant/wpasupplicant.h>

#include <circle_glue.h>
#include <string.h>

/**
 * Basic Circle Stdlib application that supports GPIO access.
 */
class CStdlibApp
{
public:
        enum TShutdownMode
        {
                ShutdownNone,
                ShutdownHalt,
                ShutdownReboot
        };

        CStdlibApp (const char *kernel) :
                FromKernel (kernel)
        {
        }

        virtual ~CStdlibApp (void)
        {
        }

        virtual bool Initialize (void)
        {
                return mInterrupt.Initialize ();
        }

        virtual void Cleanup (void)
        {
        }

        virtual TShutdownMode Run (void) = 0;

        const char *GetKernelName(void) const
        {
                return FromKernel;
        }

protected:
        CActLED            mActLED;
        CKernelOptions     mOptions;
        CDeviceNameService mDeviceNameService;
        CNullDevice        mNullDevice;
        CExceptionHandler  mExceptionHandler;
        CInterruptSystem   mInterrupt;

private:
        char const *FromKernel;
};

/**
 * Stdlib application that adds screen support
 * to the basic CStdlibApp features.
 */
class CStdlibAppScreen : public CStdlibApp
{
public:
        CStdlibAppScreen(const char *kernel)
                : CStdlibApp (kernel),
                  mScreenUnbuffered (mOptions.GetWidth (), mOptions.GetHeight ()),
                  mScreen (&mScreenUnbuffered),
                  mbScreenAvailable (false),
                  mTimer (&mInterrupt),
                  mLogger (mOptions.GetLogLevel (), &mTimer)
        {
        }

        virtual bool Initialize (void)
        {
                if (!CStdlibApp::Initialize ())
                {
                        return false;
                }

                mbScreenAvailable = mScreenUnbuffered.Initialize ();
#if 0
                if (!mSerial.Initialize (115200))
                {
                        return false;
                }
#endif
                CDevice *pTarget =
                        mDeviceNameService.GetDevice (mOptions.GetLogDevice (), false);
                if (pTarget == 0)
                {
                        pTarget = &mScreen;
                }

                if (!mLogger.Initialize (pTarget))
                {
                        return false;
                }

                return mTimer.Initialize ();
        }

protected:
        CScreenDevice   mScreenUnbuffered;
        //CSerialDevice   mSerial;
        CWriteBufferDevice mScreen;
        bool            mbScreenAvailable;
        CTimer          mTimer;
        CLogger         mLogger;
};

/**
 * Stdlib application that adds stdio support
 * to the CStdlibAppScreen functionality.
 */
class CStdlibAppStdio: public CStdlibAppScreen
{
private:
        char const *mpPartitionName;

public:
        // TODO transform to constexpr
        // constexpr char static DefaultPartition[] = "emmc1-1";
#define CSTDLIBAPP_LEGACY_DEFAULT_PARTITION "emmc1-1"
#define CSTDLIBAPP_DEFAULT_PARTITION "SD:"

        CStdlibAppStdio (const char *kernel,
                         const char *pPartitionName = CSTDLIBAPP_DEFAULT_PARTITION)
                : CStdlibAppScreen (kernel),
                  mpPartitionName (pPartitionName),
                  mEMMC (&mInterrupt, &mTimer, &mActLED),
#if !defined(__aarch64__) || !defined(LEAVE_QEMU_ON_HALT)
                  //mConsole (&mScreen, TRUE)
                  mConsole (&mNullDevice, &mScreen)
#else
                  mConsole (&mScreen)
#endif
        {
        }

        virtual bool Initialize (void)
        {
                if (!CStdlibAppScreen::Initialize ())
                {
                        return false;
                }

                if (!mEMMC.Initialize ())
                {
                        return false;
                }

                char const *partitionName = mpPartitionName;

                // Recognize the old default partion name
                if (strcmp(partitionName, CSTDLIBAPP_LEGACY_DEFAULT_PARTITION) == 0)
                {
                        partitionName = CSTDLIBAPP_DEFAULT_PARTITION;
                }

                if (f_mount (&mFileSystem, partitionName, 1) != FR_OK)
                {
                        mLogger.Write (GetKernelName (), LogError,
                                         "Cannot mount partition: %s", partitionName);

                        return false;
                }

                if (!mConsole.Initialize ())
                {
                        return false;
                }

                // Initialize newlib stdio with a reference to Circle's console
                // (Remove mFileSystem as a parameter to mirror change in circle-stdlib's
                //  commit "Remove obsolete FATFS-related code", dated Dec 2022)
                CGlueStdioInit (mConsole);

                mLogger.Write (GetKernelName (), LogNotice, "Compile time: " __DATE__ " " __TIME__);

                return true;
        }
		
        virtual void Cleanup (void)
        {
                f_mount(0, "", 0);

                CStdlibAppScreen::Cleanup ();
        }

protected:
        CEMMCDevice     mEMMC;
        FATFS           mFileSystem;
        CConsole        mConsole;
};

#endif
