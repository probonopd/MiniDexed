#include "effect_base.h"

AudioEffect::AudioEffect(float32_t samplerate)
{
    this->samplerate = samplerate;
}

AudioEffect::~AudioEffect()
{
}

void AudioEffect::setBypass(bool bypass)
{
    this->bypass = bypass;
}

bool AudioEffect::getBypass()
{
    return bypass;
}

unsigned AudioEffect::getId()
{
    return EFFECT_NONE;
}

void AudioEffect::process(const float32_t* inblock, float32_t* outblock, uint16_t len)
{
    // Mono process
    // Dummy buffer for right channel
    float32_t dummyBuffer[len];
    process(inblock, dummyBuffer, outblock, dummyBuffer, len);
}

void AudioEffect::process(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len)
{
    if (bypass) {
        return;
    }
    doProcess(inblockL, inblockR, outblockL, outblockR, len);
}

void AudioEffect::doProcess(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len) {
    for (uint16_t i=0; i < len; i++) 
    {
        outblockL[i] = inblockL[i];
        outblockR[i] = inblockR[i];
    }
}

AudioEffectNone::AudioEffectNone(float32_t samplerate) : AudioEffect(samplerate)
{
}

AudioEffectNone::~AudioEffectNone()
{
}

void AudioEffectNone::doProcess(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len)
{
    for (uint16_t i=0; i < len; i++) 
    {
        outblockL[i] = inblockL[i];
        outblockR[i] = inblockR[i];
    }
}

unsigned AudioEffectNone::getId()
{
    return EFFECT_NONE;
}