#ifndef _EFFECT_CHORUS_H
#define _EFFECT_CHORUS_H

#include "effect_base.h"
#include "ykchorus/ChorusEngine.h"

class AudioEffectChorus : public AudioEffect
{
public:
    enum Param
    {
        CHORUS_I_ENABLE,
        CHORUS_II_ENABLE,
        CHORUS_I_RATE,
        CHORUS_II_RATE,
        UNKNOWN
    };

    AudioEffectChorus(float32_t samplerate);
    virtual ~AudioEffectChorus();

    virtual unsigned getId();

    virtual void setParameter(unsigned param, unsigned value);
    virtual unsigned getParameter(unsigned param);

    unsigned getChorusI();
    void setChorusI(unsigned enable);

    unsigned getChorusII();
    void setChorusII(unsigned enable);

    unsigned getChorusIRate();
    void setChorusIRate(unsigned int rate);

    unsigned getChorusIIRate();
    void setChorusIIRate(unsigned int rate);
protected:
    virtual void doProcess(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len);

private:
    ChorusEngine *engine;
};

#endif // _EFFECT_CHORUS_H