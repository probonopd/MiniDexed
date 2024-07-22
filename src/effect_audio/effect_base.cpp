/* 
 * Base AudioEffect interface
 * Javier Nonis (https://github.com/jnonis) - 2024
 */
#include <circle/logger.h>
#include "effect_base.h"

LOGMODULE ("AudioEffect");

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

void AudioEffect::setParameters(std::vector<unsigned> params)
{
    for (size_t i = 0; i < params.size(); i++)
    {
        this->setParameter(i, params[i]);
    }
}

std::vector<unsigned> AudioEffect::getParameters()
{
    size_t len = this->getParametersSize();
    std::vector<unsigned> params;
    for (size_t i = 0; i < len; i++)
    {
        params.push_back(getParameter(i));
    }
    return params;
}

void AudioEffect::process(const float32_t* inblock, float32_t* outblock, uint16_t len)
{
    // Mono process
    // Dummy buffer for right channel
    float32_t dummyBuffer[len];
    memset(dummyBuffer, 0, len * sizeof(float32_t));
    process(inblock, dummyBuffer, outblock, dummyBuffer, len);
}

void AudioEffect::process(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len)
{
    if (bypass) {
        if (inblockL != outblockL)
        {
            memcpy(outblockL, inblockL, len * sizeof(float32_t));
        }
        if (inblockR != outblockR) {
            memcpy(outblockR, inblockR, len * sizeof(float32_t));
        }
        /*
        if (inblockL != outblockL || inblockR != outblockR) {
            // if input and output buffers are different we should copy the content
            for (uint16_t i=0; i < len; i++) 
            {
                outblockL[i] = inblockL[i];
                if (inblockL == inblockR) {
                    outblockR[i] = inblockL[i];
                }
                else
                {
                    outblockR[i] = inblockR[i];
                }
                
            }
        }
        */
        return;
    }
    doProcess(inblockL, inblockR, outblockL, outblockR, len);
}

void AudioEffect::doProcess(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len) {
}

AudioEffectNone::AudioEffectNone(float32_t samplerate) : AudioEffect(samplerate)
{
    setBypass(true);
}

AudioEffectNone::~AudioEffectNone()
{
}

void AudioEffectNone::doProcess(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len)
{
}

unsigned AudioEffectNone::getId()
{
    return EFFECT_NONE;
}