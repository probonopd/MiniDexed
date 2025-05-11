//
// performance_sysex_handler.h
//
// MiniDexed - Dexed FM synthesizer for bare metal Raspberry Pi
// Copyright (C) 2025  The MiniDexed Team
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

#ifndef PERFORMANCE_SYSEX_H
#define PERFORMANCE_SYSEX_H

#include <cstdint>
#include <cstddef>
#include "performanceconfig.h"

class CMIDIDevice;
// Handles a MiniDexed performance SysEx message.
// pMessage: pointer to the SysEx message (must be at least 2 bytes)
// pDevice: the MIDI device to send the response to
// perf: the performance config to operate on
void handle_performance_sysex(const uint8_t* pMessage, CMIDIDevice* pDevice, CPerformanceConfig* perf, unsigned nCable = 0);

#endif // PERFORMANCE_SYSEX_H
