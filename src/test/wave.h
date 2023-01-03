#pragma once

#include <stdint.h>
#include <arm_math.h>
#include <string>

struct WaveHeader {
    char chunkId[4];
    uint32_t chunkSize;
    char format[4];
    char subchunk1Id[4];
    uint32_t subchunk1Size;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
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

void playSound(float32_t* LChannel,
               float32_t* RChannel,
               unsigned size,
               int sampleRate,
               int bitsPerSample);