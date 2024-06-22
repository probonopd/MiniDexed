#include <circle/logger.h>
#include "effect_chorus.h"

LOGMODULE ("fx chorus");

AudioEffectChorus::AudioEffectChorus(float32_t samplerate) : AudioEffect(samplerate)
{
    engine = new ChorusEngine(samplerate);

    engine->setEnablesChorus(true, true);
    engine->setChorus1LfoRate(5.0f / 10.0f);
    engine->setChorus2LfoRate(8.3f / 10.0f);
}

AudioEffectChorus::~AudioEffectChorus()
{
    delete engine;
}

unsigned AudioEffectChorus::getId()
{
    return EFFECT_CHORUS;
}

void AudioEffectChorus::doProcess(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len)
{
    for (uint16_t i=0; i < len; i++) 
    {
        outblockL[i] = inblockL[i];
        outblockR[i] = inblockR[i];
        engine->process(&outblockL[i], &outblockR[i]);
    }
}

unsigned AudioEffectChorus::getChorusI()
{
    return engine->isChorus1Enabled;
}

void AudioEffectChorus::setChorusI(unsigned enable)
{
    engine->setEnablesChorus(enable == 1, engine->isChorus2Enabled);
}

unsigned AudioEffectChorus::getChorusII()
{
    return engine->isChorus2Enabled;
}

void AudioEffectChorus::setChorusII(unsigned enable)
{
    engine->setEnablesChorus(engine->isChorus1Enabled, enable == 1);
}

unsigned AudioEffectChorus::getChorusIRate()
{
    return (int) roundf(engine->chorus1L->rate * 100);
}

void AudioEffectChorus::setChorusIRate(unsigned int rate)
{
    engine->setChorus1LfoRate(((float) rate) / 100.0f);
}

unsigned AudioEffectChorus::getChorusIIRate()
{
    return (int) roundf(engine->chorus2L->rate * 100);
}

void AudioEffectChorus::setChorusIIRate(unsigned int rate)
{
    engine->setChorus2LfoRate(((float) rate) / 100.0f);
}
