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

void AudioEffectChorus::setParameter(unsigned param, unsigned value)
{
    switch (param)
    {
    case AudioEffectChorus::Param::BYPASS:
        this->setBypass(value == 1);
        break;
    case AudioEffectChorus::Param::CHORUS_I_ENABLE:
        this->setChorusI(value);
        break;
    case AudioEffectChorus::Param::CHORUS_II_ENABLE:
        this->setChorusII(value);
        break;
    case AudioEffectChorus::Param::CHORUS_I_RATE:
        this->setChorusIRate(value);
        break;
    case AudioEffectChorus::Param::CHORUS_II_RATE:
        this->setChorusIIRate(value);
        break;
    default:
        break;
    }
}

unsigned AudioEffectChorus::getParameter(unsigned param)
{
    switch (param)
    {
    case AudioEffectChorus::Param::BYPASS:
		return this->getBypass() ? 1 : 0;
    case AudioEffectChorus::Param::CHORUS_I_ENABLE:
        return this->getChorusI();
    case AudioEffectChorus::Param::CHORUS_II_ENABLE:
        return this->getChorusII();
    case AudioEffectChorus::Param::CHORUS_I_RATE:
        return this->getChorusIRate();
    case AudioEffectChorus::Param::CHORUS_II_RATE:
        return this->getChorusIIRate();
    default:
        return 0;
    }
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
