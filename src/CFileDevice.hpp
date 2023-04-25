#pragma once 

#include "extra_features.h"

#include <circle/device.h>
#include <fatfs/ff.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cassert>

class CFileDevice : public CDevice
{
    DISALLOW_COPY_AND_ASSIGN(CFileDevice);

private:
    CFileDevice()
        : m_file(new FIL())
    {
        FRESULT res = f_open(this->m_file, "SD:/debuglog.txt", FA_WRITE | FA_CREATE_ALWAYS);
        assert(res == FR_OK);
    }

public:
    static void UseMeForLogger()
    {
        CLogger::Get()->SetNewTarget(&CFileDevice::GetInstance());
    }

    static CFileDevice& GetInstance()
    {
        static CFileDevice Instance;

        return Instance;
    }

    ~CFileDevice()
    {
        f_sync(this->m_file);
        f_close(this->m_file);    
    }

    virtual int Write(const void* pBuffer, size_t nCount) override
    {
        UINT nb;
        f_write(this->m_file, pBuffer, nCount, &nb);
        f_sync(this->m_file);

        return (int)nb;
    }

private:
    FIL* m_file;
};
