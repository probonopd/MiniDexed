#include "wave.h"

#include <fstream>
#include <iostream>

float32_t** readWaveFile(const std::string& fileName, unsigned& size)
{
    std::ifstream file(fileName, std::ios::binary);
    if(!file)
    {
        std::cerr << "Error opening file: " << fileName << std::endl;
        return nullptr;
    }

    WaveHeader header;
    file.read((char*)&header, sizeof(header));

    if(strncmp(header.chunkId, "RIFF", 4) != 0 || strncmp(header.format, "WAVE", 4) != 0)
    {
        std::cerr << "Error: not a WAVE file" << std::endl;
        return nullptr;
    }

    if(header.audioFormat != 1)
    {
        std::cerr << "Error: only support PCM format" << std::endl;
        return nullptr;
    }

    size = header.subchunk2Size / (header.bitsPerSample / 8);
    float32_t* LChannel = new float32_t[size];
    float32_t* RChannel = new float32_t[size];

    unsigned i = 0;
    while(!file.eof())
    {
        if(header.bitsPerSample == 8)
        {
            uint8_t LSample;
            uint8_t RSample;
            file.read((char*)&LSample, 1);
            file.read((char*)&RSample, 1);
            LChannel[i] = LSample / 128.0f - 1.0f;
            RChannel[i] = RSample / 128.0f - 1.0f;
        }
        else if (header.bitsPerSample == 16)
        {
            int16_t LSample;
            int16_t RSample;
            file.read((char*)&LSample, 2);
            file.read((char*)&RSample, 2);
            LChannel[i] = LSample / 32768.0f;
            RChannel[i] = RSample / 32768.0f;
        }
        else if (header.bitsPerSample == 24)
        {
            int32_t LSample;
            int32_t RSample;
            file.read((char*)&LSample, 3);
            file.read((char*)&RSample, 3);
            LChannel[i] = LSample / 8388608.0f;
            RChannel[i] = RSample / 8388608.0f;
        }
        else if (header.bitsPerSample == 32)
        {
            int32_t LSample;
            int32_t RSample;
            file.read((char*)&LSample, 4);
            file.read((char*)&RSample, 4);
            LChannel[i] = LSample / 2147483648.0f;
            RChannel[i] = RSample / 2147483648.0f;
        }
        else
        {
            std::cerr << "Error: unsupported bit depth: " << header.bitsPerSample << std::endl;
            return nullptr;
        }

        ++i;
    }

    float32_t** result = new float32_t*[2];
    result[0] = LChannel;
    result[1] = RChannel;

    return result;
}
