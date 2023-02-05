#include "wave.h"

#include <fstream>
#include <iostream>
#include <cassert>

#if defined(DEBUG)
#define ASSERT_NORMALIZED(x) assert(x <= 1.0f && x >= -1.0f)
#else
#define ASSERT_NORMALIZED(x)
#endif

std::ostream& operator<<(std::ostream& out, const WaveHeader& hdr)
{
    out << "WaveHeader" << std::endl;
    out << " + chunkId       : " << hdr.chunkId << std::endl;
    out << " + chunkSize     : " << hdr.chunkSize << std::endl;
    out << " + format        : " << hdr.format << std::endl;
    out << " + subchunk1Id   : " << hdr.subchunk1Id << std::endl;
    out << " + subchunk1Size : " << hdr.subchunk1Size << std::endl;
    out << " + audioFormat   : " << hdr.audioFormat << std::endl;
    out << " + numChannels   : " << hdr.numChannels << std::endl;
    out << " + sampleRate    : " << hdr.sampleRate << std::endl;
    out << " + byteRate      : " << hdr.byteRate << std::endl;
    out << " + blockAlign    : " << hdr.blockAlign << std::endl;
    out << " + bitsPerSample : " << hdr.bitsPerSample << std::endl;
    out << " + subchunk2Id   : " << hdr.subchunk2Id << std::endl;
    out << " + subchunk2Size : " << hdr.subchunk2Size << std::endl;

    return out;
}

std::ostream& operator<<(std::ostream& out, const ChunkID& id)
{
    out << "'"
        << id.ID[0]
        << id.ID[1]
        << id.ID[2]
        << id.ID[3]
        << "'";

    return out;
}

std::ostream& operator<<(std::ostream& out, const WaveHeaderRIFF& riff)
{
    out << "WaveHeaderRIFF" << std::endl;
    out << " + chunkId       : " << riff.chunkId << std::endl;
    out << " + chunkSize     : " << riff.chunkSize << std::endl;
    out << " + format        : " << riff.format << std::endl;

    return out;
}

std::ostream& operator<<(std::ostream& out, const WaveHeaderFMT& fmt)
{
    out << "WaveHeaderFMT" << std::endl;
    out << " + subchunk1Id   : " << fmt.subchunk1Id << std::endl;
    out << " + subchunk1Size : " << fmt.subchunk1Size << std::endl;
    out << " + audioFormat   : " << fmt.audioFormat << std::endl;
    out << " + numChannels   : " << fmt.numChannels << std::endl;
    out << " + sampleRate    : " << fmt.sampleRate << std::endl;
    out << " + byteRate      : " << fmt.byteRate << std::endl;
    out << " + blockAlign    : " << fmt.blockAlign << std::endl;
    out << " + bitsPerSample : " << fmt.bitsPerSample << std::endl;

    return out;
}

std::ostream& operator<<(std::ostream& out, const WaveHeaderDATA& data)
{
    out << "WaveHeaderDATA" << std::endl;
    out << " + subchunk2Id   : " << data.subchunk2Id << std::endl;
    out << " + subchunk2Size : " << data.subchunk2Size << std::endl;

    return out;
}

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

float32_t** readWaveFile(const std::string& fileName, size_t& size, WaveHeader* hdr)
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

    if(hdr != nullptr)
    {
        hdr->chunkId         = riff.chunkId;
        hdr->chunkSize       = riff.chunkSize;
        hdr->format          = riff.format;
        hdr->subchunk1Id     = fmt.subchunk1Id;
        hdr->subchunk1Size   = fmt.subchunk1Size;
        hdr->audioFormat     = fmt.audioFormat;
        hdr->numChannels     = fmt.numChannels;
        hdr->sampleRate      = fmt.sampleRate;
        hdr->byteRate        = fmt.byteRate;
        hdr->blockAlign      = fmt.blockAlign;
        hdr->bitsPerSample   = fmt.bitsPerSample;
        hdr->subchunk2Id     = data.subchunk2Id;
        hdr->subchunk2Size   = data.subchunk2Size;
    }

    size = data.subchunk2Size / fmt.blockAlign;
    float32_t* LChannel = new float32_t[size];
    float32_t* RChannel = new float32_t[size];

    size_t i = 0;
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

        ++i;
    }
    assert(i == size);

    float32_t** result = new float32_t*[2];
    result[0] = LChannel;
    result[1] = RChannel;

    return result;
}
