/* 
 * Tal Reverb 3 Port
 * Ported from https://github.com/DISTRHO/DISTRHO-Ports/tree/master/ports-juce5/tal-reverb-3
 *  
 * Javier Nonis (https://github.com/jnonis) - 2024
 */
#include <circle/logger.h>
#include "effect_talreverb3.h"

LOGMODULE ("fx talreverb3");

AudioEffectTalReverb3::AudioEffectTalReverb3(float32_t samplerate) : AudioEffect(samplerate)
{   
    this->engine = new ReverbEngine(samplerate);
	this->params = this->engine->param;

    this->setParameter(AudioEffectTalReverb3::Param::DRY, 50);
    this->setParameter(AudioEffectTalReverb3::Param::WET, 23);
    this->setParameter(AudioEffectTalReverb3::Param::DECAYTIME, 42);
    this->setParameter(AudioEffectTalReverb3::Param::PREDELAY, 0);
    this->setParameter(AudioEffectTalReverb3::Param::LOWSHELFGAIN, 89);
    this->setParameter(AudioEffectTalReverb3::Param::HIGHSHELFGAIN, 33);
    this->setParameter(AudioEffectTalReverb3::Param::STEREO, 100);
    this->setParameter(AudioEffectTalReverb3::Param::REALSTEREOMODE, 1);
    this->setParameter(AudioEffectTalReverb3::Param::POWER, 1);
}

AudioEffectTalReverb3::~AudioEffectTalReverb3()
{
    delete this->engine;
}

void AudioEffectTalReverb3::setParameter(unsigned param, unsigned value)
{

    this->params[param] = value;
	switch (param)
    {
    case AudioEffectTalReverb3::Param::BYPASS:
        this->setBypass(value == 1);
        break;
    case AudioEffectTalReverb3::Param::WET:
        this->engine->setWet((float) value / 100.0f);
        break;
    case AudioEffectTalReverb3::Param::DRY:
        this->engine->setDry((float) value / 100.0f);
        break;
    case AudioEffectTalReverb3::Param::DECAYTIME:
        this->engine->setDecayTime((float) value / 100.0f);
        break;
    case AudioEffectTalReverb3::Param::PREDELAY:
        this->engine->setPreDelay((float) value / 100.0f);
        break;
    case AudioEffectTalReverb3::Param::LOWSHELFGAIN:
        this->engine->setLowShelfGain((float) value / 100.0f);
        break;
    case AudioEffectTalReverb3::Param::HIGHSHELFGAIN:
        this->engine->setHighShelfGain((float) value / 100.0f);
        break;
    case AudioEffectTalReverb3::Param::STEREO:
        this->engine->setStereoWidth((float) value / 100.0f);
        break;
    case AudioEffectTalReverb3::Param::REALSTEREOMODE:
        this->engine->setStereoMode((float) value);
        break;
    case AudioEffectTalReverb3::Param::POWER:
        this->engine->setPower((float) value);
        break;
    default:
        break;
    }
}

unsigned AudioEffectTalReverb3::getParameter(unsigned param)
{
    switch (param)
    {
    case AudioEffectTalReverb3::Param::BYPASS:
		return this->getBypass() ? 1 : 0;
    case AudioEffectTalReverb3::Param::WET:
    case AudioEffectTalReverb3::Param::DRY:
    case AudioEffectTalReverb3::Param::DECAYTIME:
    case AudioEffectTalReverb3::Param::PREDELAY:
    case AudioEffectTalReverb3::Param::LOWSHELFGAIN:
    case AudioEffectTalReverb3::Param::HIGHSHELFGAIN:
    case AudioEffectTalReverb3::Param::STEREO:
    case AudioEffectTalReverb3::Param::REALSTEREOMODE:
    case AudioEffectTalReverb3::Param::POWER:
        return params[param];
    default:
        return 0;
    }
}


void AudioEffectTalReverb3::doProcess(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len)
{
    int numberOfChannels = 2;
    if (inblockL == inblockR) {
        numberOfChannels = 1;
    }

    for (size_t i = 0; i < len; i++)
    {
        if (numberOfChannels == 2)
	    {
            outblockL[i] = inblockL[i];
            outblockR[i] = inblockR[i];
        }
        else
        {
            outblockL[i] = inblockL[i];
            outblockR[i] = inblockL[i];
        }

        engine->process(&outblockL[i], &outblockR[i]);
    }
}