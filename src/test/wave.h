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
