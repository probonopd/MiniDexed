#include "wave.h"

#include <fstream>
#include <iostream>
#include <cstring>

void saveWaveFile(const std::string& fileName,
                  float32_t* LChannel,
                  float32_t* RChannel,
                  size_t size,
                  int sampleRate,
                  int bitsPerSample)
{
    std::ofstream file(fileName, std::ios::binary);
    if(!file)
    {
        std::cerr << "Error opening file: " << fileName << std::endl;
        return;
    }

    WaveHeader header;
    std::memset(&header, 0, sizeof(header));
    header.sampleRate = sampleRate;
    header.numChannels = 2;
    header.bitsPerSample = bitsPerSample;
    header.byteRate = header.sampleRate * header.numChannels * header.bitsPerSample / 8;
    header.blockAlign = header.numChannels * header.bitsPerSample / 8;
    header.subchunk2Size = size * header.blockAlign;
    header.chunkSize = 36 + header.subchunk2Size;
    header.subchunk1Size = 16;
    header.audioFormat = 1;

    header.chunkId.Value = id2int("RIFF");
    header.format.Value = id2int("WAVE");
    header.subchunk1Id.Value = id2int("fmt ");
    header.subchunk2Id.Value = id2int("data");
    // std::strncpy(header.chunkId, "RIFF", 4);
    // std::strncpy(header.format, "WAVE", 4);
    // std::strncpy(header.subchunk1Id, "fmt ", 4);
    // std::strncpy(header.subchunk2Id, "data", 4);

    file.write((char*)&header, sizeof(header));

    if(bitsPerSample == 8)
    {
        for (size_t i = 0; i < size; i++)
        {
            int8_t leftSample = (int8_t)(LChannel[i] * 128.0f + 128.0f);
            int8_t rightSample = (int8_t)(RChannel[i] * 128.0f + 128.0f);
            file.write((char*)&leftSample, 1);
            file.write((char*)&rightSample, 1);
        }
    } 
    else if(bitsPerSample == 16)
    {
        for (size_t i = 0; i < size; i++)
        {
            int16_t leftSample = (int16_t)(LChannel[i] * 32768.0f);
            int16_t rightSample = (int16_t)(RChannel[i] * 32768.0f);
            file.write((char*)&leftSample, 2);
            file.write((char*)&rightSample, 2);
        }
    }
    else if(bitsPerSample == 24)
    {
        for(size_t i = 0; i < size; i++)
        {
            int32_t leftSample = (int32_t)(LChannel[i] * 8388608.0f);
            int32_t rightSample = (int32_t)(RChannel[i] * 8388608.0f);
            file.write((char*)&leftSample, 3);
            file.write((char*)&rightSample, 3);
        }
    }
    else if(bitsPerSample == 32)
    {
        for (size_t i = 0; i < size; i++)
        {
            int32_t leftSample = (int32_t)(LChannel[i] * 2147483648.0f);
            int32_t rightSample = (int32_t)(RChannel[i] * 2147483648.0f);
            file.write((char*)&leftSample, 4);
            file.write((char*)&rightSample, 4);
        }
    }
    else
    {
        std::cerr << "Error: unsupported bit depth: " << bitsPerSample << std::endl;
        return;
    }

    file.close();
}
