//
// midichunker.h
//
// MiniDexed - Dexed FM synthesizer for bare metal Raspberry Pi
// Copyright (C) 2022-25  The MiniDexed Team
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

#pragma once
#include <cstddef>
#include <vector>
#include <cstdint>

class MIDISysExChunker {
public:
    MIDISysExChunker(const uint8_t* data, size_t length, size_t chunkSize = 256);
    bool hasNext() const;
    std::vector<uint8_t> next();
    void reset();
private:
    const uint8_t* m_data;
    size_t m_length;
    size_t m_chunkSize;
    size_t m_offset;
};
