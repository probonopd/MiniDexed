#include <circle/logger.h>
#include "effect_delay.h"

LOGMODULE ("fx chorus");

AudioEffectDelay::AudioEffectDelay(float32_t samplerate) : AudioEffect(samplerate)
{   
}

AudioEffectDelay::~AudioEffectDelay()
{
}

unsigned AudioEffectDelay::getId()
{
    return EFFECT_DELAY;
}

void AudioEffectDelay::doProcess(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len)
{
    for (uint16_t i=0; i < len; i++) 
    {
        outblockL[i] = inblockL[i];
        outblockR[i] = inblockR[i];
    }
}
