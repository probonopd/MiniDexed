#include <circle/logger.h>
#include "effect_delay.h"

LOGMODULE ("fx delay");

AudioEffectDelay::AudioEffectDelay(float32_t samplerate) : AudioEffect(samplerate)
{   
    this->bufferSize = (int) samplerate * MAX_DELAY_TIME;
    this->bufferL = new float32_t[this->bufferSize];
    this->bufferR = new float32_t[this->bufferSize];
    this->index = 0;

    for (size_t i = 0; i < this->bufferSize; i++)
    {
        this->bufferL[i] = 0.0f;
        this->bufferR[i] = 0.0f;
    }

    this->timeL = 0.36f;
    this->timeR = 0.36f;
    this->feedback = 0.3f;
}

AudioEffectDelay::~AudioEffectDelay()
{
    delete this->bufferL;
    delete this->bufferR;
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
        this->bufferL[index] = inblockL[i];
        this->bufferR[index] = inblockR[i];

        // Calculate offsets
        int offsetL = this->index - (this->timeL * this->samplerate);
        if (offsetL < 0) {
            offsetL = this->bufferSize + offsetL;
        }
        int offsetR = this->index - (this->timeR * this->samplerate);
        if (offsetR < 0) {
            offsetR = this->bufferSize + offsetR;
        }

        this->bufferL[index] += this->bufferL[offsetL] * this->feedback;
        this->bufferR[index] += this->bufferR[offsetR] * this->feedback;

        outblockL[i] = this->bufferL[index];
        outblockR[i] = this->bufferR[index];

        // Update index
        this->index++;
        if (this->index >= this->bufferSize)
        {
            this->index = 0;
        }
    }
}
