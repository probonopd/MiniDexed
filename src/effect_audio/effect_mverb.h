/* 
 * MVerb Reverb Port
 * Ported from https://github.com/DISTRHO/MVerb
 * Original https://github.com/martineastwood/mverb/
 *
 * Javier Nonis (https://github.com/jnonis) - 2024
 */
#ifndef _EFFECT_MVERB_H
#define _EFFECT_MVERB_H

#include "effect_base.h"
#include "mverb/MVerb.h"

class AudioEffectMVerb : public AudioEffect
{
public:
    // ID must be unique for each AudioEffect
    static const unsigned ID = 8;
    static constexpr const char* NAME = "MVerb";

    enum Param
    {
        BYPASS,
        DAMPINGFREQ,
        DENSITY,
        BANDWIDTHFREQ,
        DECAY,
        PREDELAY,
        SIZE,
        GAIN,
        MIX,
        EARLYMIX,
        UNKNOWN
    };

    AudioEffectMVerb(float32_t samplerate);
    virtual ~AudioEffectMVerb();

    virtual unsigned getId()
    {
        return AudioEffectMVerb::ID;
    }
    
    virtual std::string getName()
    {
        return AudioEffectMVerb::NAME;
    }

    virtual void initializeSendFX();
    virtual void setParameter(unsigned param, unsigned value);
    virtual unsigned getParameter(unsigned param);
protected:
    virtual size_t getParametersSize()
    {
        return AudioEffectMVerb::Param::UNKNOWN;
    }
    virtual void doProcess(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len);

private:
    MVerb<float>* fVerb;
};

#endif // _EFFECT_MVERB_H
