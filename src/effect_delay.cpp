#include <circle/logger.h>
#include "effect_delay.h"

LOGMODULE ("fx chorus");

AudioEffectDelay::AudioEffectDelay(float32_t samplerate) : AudioEffect(samplerate)
{   
    bufferSize = (int) samplerate * MAX_DELAY_TIME;
    bufferL = new float32_t[this->bufferSize];
    bufferR = new float32_t[this->bufferSize];
    index = 0;

    for (size_t i = 0; i < bufferSize; i++)
    {
        bufferL[i] = 0.0f;
        bufferR[i] = 0.0f;
    }

    timeL = 0.36f;
    timeR = 0.36f;
    feedback = 0.3f;
}

AudioEffectDelay::~AudioEffectDelay()
{
    delete bufferL;
    delete bufferR;
}

unsigned AudioEffectDelay::getId()
{
    return EFFECT_DELAY;
}

void AudioEffectDelay::doProcess(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len)
{
    for (uint16_t i=0; i < len; i++) 
    {
        // Update buffers
        bufferL[index] = inblockL[i];
        bufferR[index] = inblockR[i];

        // Calculate offsets
        int offsetL = index - (timeL * samplerate);
        if (offsetL < 0) {
            offsetL = bufferSize + offsetL;
        }
        int offsetR = index - (timeR * samplerate);
        if (offsetR < 0) {
            offsetR = bufferSize + offsetR;
        }

        bufferL[index] += bufferL[offsetL] * feedback;
        bufferR[index] += bufferR[offsetR] * feedback;

        outblockL[i] = bufferL[index];
        outblockR[i] = bufferR[index];

        // Update index
        index++;
        if (index >= bufferSize)
        {
            index = 0;
        }
    }
}
