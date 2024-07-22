/* 
 * MVerb Reverb Port
 * Ported from https://github.com/DISTRHO/MVerb
 * Original https://github.com/martineastwood/mverb/
 *
 * Javier Nonis (https://github.com/jnonis) - 2024
 */
#include "effect_mverb.h"

AudioEffectMVerb::AudioEffectMVerb(float32_t samplerate) : AudioEffect(samplerate)
{   
    fVerb.setSampleRate(samplerate);

    fVerb.setParameter(MVerb<float>::DAMPINGFREQ, 0.5f);
    fVerb.setParameter(MVerb<float>::DENSITY, 0.5f);
    fVerb.setParameter(MVerb<float>::BANDWIDTHFREQ, 0.5f);
    fVerb.setParameter(MVerb<float>::DECAY, 0.5f);
    fVerb.setParameter(MVerb<float>::PREDELAY, 0.5f);
    fVerb.setParameter(MVerb<float>::SIZE, 0.75f);
    fVerb.setParameter(MVerb<float>::GAIN, 1.0f);
    fVerb.setParameter(MVerb<float>::MIX, 0.5f);
    fVerb.setParameter(MVerb<float>::EARLYMIX, 0.5f);
    fVerb.reset();
}

AudioEffectMVerb::~AudioEffectMVerb()
{
}

void AudioEffectMVerb::initializeSendFX()
{
    this->setParameter(AudioEffectMVerb::Param::MIX, 100);
}

void AudioEffectMVerb::setParameter(unsigned param, unsigned value)
{
    switch (param)
    {
    case AudioEffectMVerb::Param::BYPASS:
        this->setBypass(value == 1);
        break;
    case AudioEffectMVerb::Param::DAMPINGFREQ:
        fVerb.setParameter(MVerb<float>::DAMPINGFREQ, (float) value / 100.0f);
        break;
    case AudioEffectMVerb::Param::DENSITY:
        fVerb.setParameter(MVerb<float>::DENSITY, (float) value / 100.0f);
        break;
    case AudioEffectMVerb::Param::BANDWIDTHFREQ:
        fVerb.setParameter(MVerb<float>::BANDWIDTHFREQ, (float) value / 100.0f);
        break;
    case AudioEffectMVerb::Param::DECAY:
        fVerb.setParameter(MVerb<float>::DECAY, (float) value / 100.0f);
        break;
    case AudioEffectMVerb::Param::PREDELAY:
        fVerb.setParameter(MVerb<float>::PREDELAY, (float) value / 100.0f);
        break;
    case AudioEffectMVerb::Param::SIZE:
        fVerb.setParameter(MVerb<float>::SIZE, (float) value / 100.0f);
        break;
    case AudioEffectMVerb::Param::GAIN:
        fVerb.setParameter(MVerb<float>::GAIN, (float) value / 100.0f);
        break;
    case AudioEffectMVerb::Param::MIX:
        fVerb.setParameter(MVerb<float>::MIX, (float) value / 100.0f);
        break;
    case AudioEffectMVerb::Param::EARLYMIX:
        fVerb.setParameter(MVerb<float>::EARLYMIX, (float) value / 100.0f);
        break;
    default:
        break;
    }
}

unsigned AudioEffectMVerb::getParameter(unsigned param)
{
    switch (param)
    {
    case AudioEffectMVerb::Param::BYPASS:
		return this->getBypass() ? 1 : 0;
    case AudioEffectMVerb::Param::DAMPINGFREQ:
        return roundf(fVerb.getParameter(MVerb<float>::DAMPINGFREQ) * 100);
    case AudioEffectMVerb::Param::DENSITY:
        return roundf(fVerb.getParameter(MVerb<float>::DENSITY) * 100);
    case AudioEffectMVerb::Param::BANDWIDTHFREQ:
        return roundf(fVerb.getParameter(MVerb<float>::BANDWIDTHFREQ) * 100);
    case AudioEffectMVerb::Param::DECAY:
        return roundf(fVerb.getParameter(MVerb<float>::DECAY) * 100);
    case AudioEffectMVerb::Param::PREDELAY:
        return roundf(fVerb.getParameter(MVerb<float>::PREDELAY) * 100);
    case AudioEffectMVerb::Param::SIZE:
        return roundf(fVerb.getParameter(MVerb<float>::SIZE) * 100);
    case AudioEffectMVerb::Param::GAIN:
        return roundf(fVerb.getParameter(MVerb<float>::GAIN) * 100);
    case AudioEffectMVerb::Param::MIX:
        return roundf(fVerb.getParameter(MVerb<float>::MIX) * 100);
    case AudioEffectMVerb::Param::EARLYMIX:
        return roundf(fVerb.getParameter(MVerb<float>::EARLYMIX) * 100);
    default:
        return 0;
    }
}


void AudioEffectMVerb::doProcess(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len)
{
    const float32_t* inputs[2];
    inputs[0] = inblockL;
    inputs[1] = inblockR;

    float32_t* outputs[2];
    outputs[0] = outblockL;
    outputs[1] = outblockR;

    fVerb.process(inputs, outputs, static_cast<int>(len));
}

