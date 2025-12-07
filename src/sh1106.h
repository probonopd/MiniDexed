//
// sh1106.h
//
// MiniDexed - Dexed FM synthesizer for bare metal Raspberry Pi
// Copyright (C) 2024  The MiniDexed Team
//
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

#ifndef _sh1106_h
#define _sh1106_h

#include <display/ssd1306.h>

// Low-level SH1106 OLED driver derived from the SSD1306 implementation.
class CSH1106 : public CSSD1306
{
public:
        CSH1106 (CI2CMaster *pI2CMaster, u8 nAddress, u8 nWidth, u8 nHeight, TLCDRotation Rotation);

        void WriteFrameBuffer (bool bForceFullUpdate) const;
};

#endif
