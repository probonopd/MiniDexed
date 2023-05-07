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
// wave.h
//
// Set of helpers to manipulate RIFF Wave files. These helpers are used in the unit tests.
// Author: Vincent Gauché
//
#pragma once

#include <stdint.h>
#include <arm_math.h>
#include <string>
#include <iostream>

inline uint32_t id2int(const char id[4])
{
    uint32_t v = id[3];
    v <<= 8;
    v += id[2];
    v <<= 8;
    v += id[1];
    v <<= 8;
    v += id[0];

    return v;
}

union ChunkID
{
    char ID[4];
    uint32_t Value;
};

struct WaveHeader {
    ChunkID chunkId;
    uint32_t chunkSize;
    ChunkID format;
    ChunkID subchunk1Id;
    uint32_t subchunk1Size;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    ChunkID subchunk2Id;
    uint32_t subchunk2Size;
};

struct WaveHeaderRIFF {
    ChunkID chunkId;
    uint32_t chunkSize;
    ChunkID format;
};

struct WaveHeaderFMT {
    ChunkID subchunk1Id;
    uint32_t subchunk1Size;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
};

struct WaveHeaderDATA {
    ChunkID subchunk2Id;
    uint32_t subchunk2Size;
};

std::ostream& operator<<(std::ostream& out, const ChunkID& id);
std::ostream& operator<<(std::ostream& out, const WaveHeader& hdr);
std::ostream& operator<<(std::ostream& out, const WaveHeaderRIFF& riff);
std::ostream& operator<<(std::ostream& out, const WaveHeaderFMT& fmt);
std::ostream& operator<<(std::ostream& out, const WaveHeaderDATA& data);

float32_t** readWaveFile(const std::string& fileName, size_t& size, WaveHeader* hdr = nullptr);

void saveWaveFile(const std::string& fileName,
                  const float32_t* LChannel,
                  const float32_t* RChannel,
                  size_t size,
                  int sampleRate,
                  int bitsPerSample);

// void playSound(float32_t* LChannel,
//                float32_t* RChannel,
//                unsigned size,
//                int sampleRate,
//                int bitsPerSample);
