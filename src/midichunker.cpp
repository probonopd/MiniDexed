//
// midichunker.cpp
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

#include "midichunker.h"
#include <algorithm>

MIDISysExChunker::MIDISysExChunker(const uint8_t* data, size_t length, size_t chunkSize)
    : m_data(data), m_length(length), m_chunkSize(chunkSize), m_offset(0) {}

bool MIDISysExChunker::hasNext() const {
    return m_offset < m_length;
}

std::vector<uint8_t> MIDISysExChunker::next() {
    if (!hasNext()) return {};
    size_t remaining = m_length - m_offset;
    size_t chunkLen = std::min(m_chunkSize, remaining);
    // Only the last chunk should contain the final 0xF7
    if (m_offset + chunkLen >= m_length && m_data[m_length-1] == 0xF7) {
        chunkLen = m_length - m_offset;
    } else if (m_offset + chunkLen > 0 && m_data[m_offset + chunkLen - 1] == 0xF7) {
        chunkLen--;
    }
    std::vector<uint8_t> chunk(m_data + m_offset, m_data + m_offset + chunkLen);
    m_offset += chunkLen;
    return chunk;
}

void MIDISysExChunker::reset() {
    m_offset = 0;
}
