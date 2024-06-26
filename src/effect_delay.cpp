#include <circle/logger.h>
#include "effect_delay.h"

LOGMODULE ("fx delay");

AudioEffectDelay::AudioEffectDelay(float32_t samplerate) : AudioEffect(samplerate)
{   
    this->bufferSize = (int) samplerate * MAX_DELAY_TIME;
    this->bufferL = new float32_t[this->bufferSize];
    this->bufferR = new float32_t[this->bufferSize];
    this->index = 0;
    this->lpf = new AudioEffectLPF(samplerate);
    this->lpf->setParameter(AudioEffectLPF::Param::CUTOFF, 80);
    this->lpf->setParameter(AudioEffectLPF::Param::RESONANCE, 0);

    // Clean buffers
    memset(this->bufferL, 0, this->bufferSize * sizeof(float32_t));
    memset(this->bufferR, 0, this->bufferSize * sizeof(float32_t));
    
    this->timeL = 0.36f;
    this->timeR = 0.36f;
    this->feedback = 0.5f;
}

AudioEffectDelay::~AudioEffectDelay()
{
    delete this->bufferL;
    delete this->bufferR;
    delete this->lpf;
}

unsigned AudioEffectDelay::getId()
{
    return EFFECT_DELAY;
}

void AudioEffectDelay::setParameter(unsigned param, unsigned value)
{
    switch (param)
    {
    case AudioEffectDelay::Param::BYPASS:
        this->setBypass(value);
        break;
    case AudioEffectDelay::Param::TIME_L:
        this->timeL = (float32_t) value / 1000.0f;
        break;
    case AudioEffectDelay::Param::TIME_R:
        this->timeR = (float32_t) value / 1000.0f;
        break;
    case AudioEffectDelay::Param::FEEDBACK:
        this->feedback = (float32_t) value / 100.0f;
        break;
    case AudioEffectDelay::Param::TONE:
        this->lpf->setParameter(AudioEffectLPF::Param::CUTOFF, value);
        break;
    default:
        break;
    }
}

unsigned AudioEffectDelay::getParameter(unsigned param)
{
    switch (param)
    {
    case AudioEffectDelay::Param::TIME_L:
        return roundf(this->timeL * 1000);
    case AudioEffectDelay::Param::TIME_R:
        return roundf(this->timeR * 1000);
    case AudioEffectDelay::Param::FEEDBACK:
        return roundf(this->feedback * 100);
    case AudioEffectDelay::Param::TONE:
        return this->lpf->getParameter(AudioEffectLPF::Param::CUTOFF);
    default:
        return 0;
    }
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

        this->bufferL[index] += this->lpf->processSampleL(this->bufferL[offsetL]) * this->feedback;
        this->bufferR[index] += this->lpf->processSampleR(this->bufferR[offsetR]) * this->feedback;
        
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
