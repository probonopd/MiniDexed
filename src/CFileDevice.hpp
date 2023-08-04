// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

//
// CFileDevice.h
//
// Device implementation that writes into a file.
// This device is a singleton dedicated to allow the Circle CLogger class to output log event into the file instead of the internal buffer.
// Author: Vincent Gauché
//

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
    CFileDevice() :
        m_file(new FIL())
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
        UINT nb = 0;
        if(nCount > 0)
        {
            f_write(this->m_file, pBuffer, nCount, &nb);
            f_sync(this->m_file);
        }

        return (int)nb;
    }

private:
    FIL* m_file;
};
