/* 
 * Tal Reverb 3 Port
 * Ported from https://github.com/DISTRHO/DISTRHO-Ports/tree/master/ports-juce5/tal-reverb-3
 * Original https://tal-software.com/
 * 
 * Javier Nonis (https://github.com/jnonis) - 2024
 */
#ifndef _EFFECT_TALREVERB3_H
#define _EFFECT_TALREVERB3_H

#include "effect_base.h"
#include "tal-reverb-3/ReverbEngine.h"

class AudioEffectTalReverb3 : public AudioEffect
{
public:
    // ID must be unique for each AudioEffect
    static const unsigned ID = 6;
    static constexpr const char* NAME = "TalRvrb3";

    enum Param
    {
        BYPASS,
        DRY,
        WET,
        DECAYTIME,
        PREDELAY,
        LOWSHELFGAIN,
        HIGHSHELFGAIN,
        STEREO,
        REALSTEREOMODE,
        POWER,
        UNKNOWN
    };

    AudioEffectTalReverb3(float32_t samplerate);
    virtual ~AudioEffectTalReverb3();

    virtual unsigned getId()
    {
        return AudioEffectTalReverb3::ID;
    }

    virtual std::string getName()
    {
        return AudioEffectTalReverb3::NAME;
    }

    virtual void initializeSendFX();
    virtual void setParameter(unsigned param, unsigned value);
    virtual unsigned getParameter(unsigned param);
protected:
    virtual size_t getParametersSize()
    {
        return AudioEffectTalReverb3::Param::UNKNOWN;
    }
    virtual void doProcess(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len);
private:
    float *params;
	ReverbEngine *engine;
};

#endif // _EFFECT_TALREVERB3_H