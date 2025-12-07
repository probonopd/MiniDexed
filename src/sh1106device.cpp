//
// sh1106device.cpp
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

#include "sh1106device.h"

CSH1106Device::CSH1106Device (unsigned nWidth, unsigned nHeight, CI2CMaster *pI2CMaster,
                              unsigned nI2CAddress, bool bRotate, bool bMirror)
:       CSSD1306Device (nWidth, nHeight, pI2CMaster, nI2CAddress, bRotate, bMirror)
{
}
