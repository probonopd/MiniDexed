#ifndef _EFFECT_BASE_H
#define _EFFECT_BASE_H

//#include <stdint.h>
#include <arm_math.h>

#define EFFECT_NONE 0
#define EFFECT_CHORUS 1
#define EFFECT_DELAY 2
#define EFFECT_LPF 3

class AudioEffect
{
public:
    AudioEffect(float32_t samplerate);
    virtual ~AudioEffect();

    void setBypass(bool bypass);
    bool getBypass();

    virtual unsigned getId();
    virtual void setParameter(unsigned param, unsigned value)
    {
    }
    virtual unsigned getParameter(unsigned param)
    {
        return 0;
    }

    void process(const float32_t* inblockL, float32_t* outblockL, uint16_t len);
    void process(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len);
protected:
    bool bypass = false;
    float32_t samplerate;
    
    virtual void doProcess(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len);
};

class AudioEffectNone : public AudioEffect
{
public:
    AudioEffectNone(float32_t samplerate);
    virtual ~AudioEffectNone();

    virtual unsigned getId();
protected:
    virtual void doProcess(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len);
};

#endif // _EFFECT_BASE_H