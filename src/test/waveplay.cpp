#include "wave.h"

#include <Windows.h>
#include <mmsystem.h>
#include <iostream>

void playSound(float32_t* LChannel,
               float32_t* RChannel,
               unsigned size,
               int sampleRate,
               int bitsPerSample)
{
    // Calculate the number of samples and the size of the audio buffer
    int numSamples = size;
    int bufferSize = numSamples * bitsPerSample / 8 * 2;

    // Create an audio buffer
    std::vector<uint8_t> buffer(bufferSize);

    // Fill the audio buffer with the sample data
    for(int i = 0; i < numSamples; i++)
    {
        if(bitsPerSample == 8)
        {
            // 8-bit samples are unsigned and range from 0 to 255
            uint8_t leftSample = (uint8_t)(LChannel[i] * 128.0f + 128.0f);
            uint8_t rightSample = (uint8_t)(RChannel[i] * 128.0f + 128.0f);
            buffer[i * 2 + 0] = leftSample;
            buffer[i * 2 + 1] = rightSample;
        }
        else if(bitsPerSample == 16)
        {
            // 16-bit samples are signed and range from -32768 to 32767
            int16_t leftSample = (int16_t)(LChannel[i] * 32768.0f);
            int16_t rightSample = (int16_t)(RChannel[i] * 32768.0f);
            buffer[i * 2 + 0] = (uint8_t)leftSample;
            buffer[i * 2 + 1] = (uint8_t)(leftSample >> 8);
            buffer[i * 2 + 2] = (uint8_t)rightSample;
            buffer[i * 2 + 3] = (uint8_t)(rightSample >> 8);
        }
        else if(bitsPerSample == 24)
        {
            // 24-bit samples are signed and range from -32768 to 32767
            int32_t leftSample = (int16_t)(LChannel[i] * 8388608.0f);
            int32_t rightSample = (int16_t)(RChannel[i] * 8388608.0f);
            buffer[i * 3 + 0] = (uint8_t)leftSample;
            buffer[i * 3 + 1] = (uint8_t)(leftSample >> 8);
            buffer[i * 3 + 2] = (uint8_t)(leftSample >> 16);
            buffer[i * 3 + 3] = (uint8_t)rightSample;
            buffer[i * 3 + 4] = (uint8_t)(rightSample >> 8);
            buffer[i * 3 + 5] = (uint8_t)(rightSample >> 16);
        }
        else if (bitsPerSample == 32)
        {
            // 32-bit samples are signed and range from -32768 to 32767
            int32_t leftSample = (int16_t)(LChannel[i] * 2147483648.0f);
            int32_t rightSample = (int16_t)(RChannel[i] * 2147483648.0f);
            buffer[i * 3 + 0] = (uint8_t)leftSample;
            buffer[i * 3 + 1] = (uint8_t)(leftSample >> 8);
            buffer[i * 3 + 2] = (uint8_t)(leftSample >> 16);
            buffer[i * 3 + 3] = (uint8_t)(leftSample >> 24);
            buffer[i * 3 + 4] = (uint8_t)rightSample;
            buffer[i * 3 + 5] = (uint8_t)(rightSample >> 8);
            buffer[i * 3 + 6] = (uint8_t)(rightSample >> 16);
            buffer[i * 3 + 7] = (uint8_t)(rightSample >> 24);
        }
        else
        {
            std::cerr << "Error: unsupported bit depth: " << bitsPerSample << std::endl;
            return;
        }
    }

    // Set up the WAVEFORMATEX structure
    WAVEFORMATEX waveFormat;
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nChannels = 2;
    waveFormat.nSamplesPerSec = sampleRate;
    waveFormat.nAvgBytesPerSec = sampleRate * bitsPerSample / 8 * 2;
    waveFormat.nBlockAlign = bitsPerSample / 8 * 2;
    waveFormat.wBitsPerSample = bitsPerSample;
    waveFormat.cbSize = 0;

    // Set up the WAVEHDR structure
    WAVEHDR waveHeader;
    waveHeader.lpData = (LPSTR)buffer.data();
    waveHeader.dwBufferLength = bufferSize;
    waveHeader.dwBytesRecorded = 0;
    waveHeader.dwUser = 0;
    waveHeader.dwFlags = 0;
    waveHeader.dwLoops = 0;
    waveHeader.lpNext = nullptr;
    waveHeader.reserved = 0;

    // Open the audio device
    HWAVEOUT audioDevice;
    MMRESULT result = waveOutOpen(&audioDevice, WAVE_MAPPER, &waveFormat, 0, 0, WAVE_FORMAT_QUERY);
    if (result != MMSYSERR_NOERROR) {
        std::cerr << "Error opening audio device" << std::endl;
        return;
    }

    // Prepare the audio buffer for playback
    result = waveOutPrepareHeader(audioDevice, &waveHeader, sizeof(WAVEHDR));
    if (result != MMSYSERR_NOERROR) {
        std::cerr << "Error preparing audio buffer" << std::endl;
        return;
    }

    // Play the audio
    waveOutWrite(audioDevice, &waveHeader, sizeof(WAVEHDR));

    // Cleanup
    waveOutUnprepareHeader(audioDevice, &waveHeader, sizeof(WAVEHDR));
    waveOutClose(audioDevice);
}