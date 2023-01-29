#include "wave.h"

#include <fstream>
#include <iostream>
#include <cassert>

#if defined(DEBUG)
#define ASSERT_NORMALIZED(x) assert(x <= 1.0f && x >= -1.0f)
#else
#define ASSERT_NORMALIZED(x)
#endif

template<typename T>
bool readChunk(std::ifstream& in, uint32_t id, T& chunk)
{
    ChunkID chunkID;
    while(!in.eof())
    {
        in.read((char*)&chunkID.Value, sizeof(chunkID.Value));
        if(chunkID.Value == id)
        {
            in.seekg(-sizeof(chunkID.Value), in.cur);
            in.read((char*)&chunk, sizeof(chunk));
            return true; 
        }
        else
        {
            in.read((char*)&chunkID.Value, sizeof(chunkID.Value));
            in.seekg(chunkID.Value, in.cur);
        }
    }

    return false;
}

float32_t** readWaveFile(const std::string& fileName, size_t& size)
{
    std::ifstream file(fileName, std::ios::binary);
    if(!file)
    {
        std::cerr << "Error opening file: " << fileName << std::endl;
        return nullptr;
    }

    WaveHeaderRIFF riff;
    if(!readChunk(file, id2int("RIFF"), riff))
    {
        std::cerr << "The file " << fileName << " does not contain any 'RIFF' chunk" << std::endl;
        return nullptr;
    }

    if(riff.format.Value != id2int("WAVE"))
    {
        std::cerr << "The file " << fileName << " is not a 'WAVE' file but a '" << riff.format.ID << "'" << std::endl;
        return nullptr;
    }

    WaveHeaderFMT fmt;
    if(!readChunk(file, id2int("fmt "), fmt))
    {
        std::cerr << "The file " << fileName << " does not contain any 'fmt ' chunk" << std::endl;
        return nullptr;
    }

    WaveHeaderDATA data;
    if(!readChunk(file, id2int("data"), data))
    {
        std::cerr << "The file " << fileName << " does not contain any 'data' chunk" << std::endl;
        return nullptr;
    }

    if(fmt.audioFormat != 1)
    {
        std::cerr << "Error: only support PCM format" << std::endl;
        return nullptr;
    }

    size = data.subchunk2Size / (fmt.bitsPerSample / 8);
    float32_t* LChannel = new float32_t[size];
    float32_t* RChannel = new float32_t[size];

    unsigned increment = fmt.numChannels;
    unsigned i = 0;
    while(!file.eof() && i < size)
    {
        if(fmt.bitsPerSample == 8)
        {
            uint8_t LSample;
            file.read((char*)&LSample, 1);
            LChannel[i] = LSample / 128.0f - 1.0f;
            if(fmt.numChannels == 2)
            {
                uint8_t RSample;
                file.read((char*)&RSample, 1);
                RChannel[i] = RSample / 128.0f - 1.0f;
            }
            else
            {
                RChannel[i] = LChannel[i];
            }
        }
        else if(fmt.bitsPerSample == 16)
        {
            int16_t LSample;
            file.read((char*)&LSample, 2);
            LChannel[i] = LSample / 32768.0f;
            if(fmt.numChannels == 2)
            {
                int16_t RSample;
                file.read((char*)&RSample, 2);
                RChannel[i] = RSample / 32768.0f;
            }
            else
            {
                RChannel[i] = LChannel[i];
            }
        }
        else if(fmt.bitsPerSample == 24)
        {
            int32_t LSample;
            file.read((char*)&LSample, 3);
            LChannel[i] = LSample / 8388608.0f;
            if(fmt.numChannels == 2)
            {
                int32_t RSample;
                file.read((char*)&RSample, 3);
                RChannel[i] = RSample / 8388608.0f;
            }
            else
            {
                RChannel[i] = LChannel[i];
            }
        }
        else if(fmt.bitsPerSample == 32)
        {
            int32_t LSample;
            file.read((char*)&LSample, 4);
            LChannel[i] = LSample / 2147483648.0f;
            if(fmt.numChannels == 2)
            {
                int32_t RSample;
                file.read((char*)&RSample, 4);
                RChannel[i] = RSample / 2147483648.0f;
            }
            else
            {
                RChannel[i] = LChannel[i];
            }
        }
        else
        {
            std::cerr << "Error: unsupported bit depth: " << fmt.bitsPerSample << std::endl;
            return nullptr;
        }

        // ASSERT_NORMALIZED(LChannel[i]);
        // ASSERT_NORMALIZED(RChannel[i]);

        i += increment;
    }
    assert(i == size);

    float32_t** result = new float32_t*[2];
    result[0] = LChannel;
    result[1] = RChannel;

    return result;
}
