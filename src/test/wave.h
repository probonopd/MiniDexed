#pragma once

#include <stdint.h>
#include <arm_math.h>
#include <string>

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
    char subchunk2Id[4];
    uint32_t subchunk2Size;
};

float32_t** readWaveFile(const std::string& fileName, unsigned& size);

void saveWaveFile(const std::string& fileName,
                  float32_t* LChannel,
                  float32_t* RChannel,
                  unsigned size,
                  int sampleRate,
                  int bitsPerSample);

// void playSound(float32_t* LChannel,
//                float32_t* RChannel,
//                unsigned size,
//                int sampleRate,
//                int bitsPerSample);
