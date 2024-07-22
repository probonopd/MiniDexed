/* 
 * Base AudioEffect interface
 * Javier Nonis (https://github.com/jnonis) - 2024
 */
#ifndef _EFFECT_BASE_H
#define _EFFECT_BASE_H

#include <arm_math.h>
#include <vector>

#define EFFECT_NONE 0
#define EFFECT_CHORUS 1
#define EFFECT_DELAY 2
#define EFFECT_LPF 3
#define EFFECT_DS1 4
#define EFFECT_BIGMUFF 5
#define EFFECT_TALREVERB3 6
#define EFFECT_REVERB 7
#define EFFECT_MVERB 8

class AudioEffect
{
public:
    AudioEffect(float32_t samplerate);
    virtual ~AudioEffect();

    void setBypass(bool bypass);
    bool getBypass();

    virtual unsigned getId();
    
    /**
     * Set default parameters for the FX when is used as Send FX.
     */
    virtual void initializeSendFX()
    {
    }

    /**
     * Set the tempo in BPM.
     */
    virtual void setTempo(unsigned tempo)
    {
    }

    virtual void setParameter(unsigned param, unsigned value)
    {
    }
    
    virtual unsigned getParameter(unsigned param)
    {
        return 0;
    }

    void setParameters(std::vector<unsigned> params);
    std::vector<unsigned> getParameters();
    void process(const float32_t* inblockL, float32_t* outblockL, uint16_t len);
    void process(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len);
protected:
    bool bypass = false;
    float32_t samplerate;
    
    virtual size_t getParametersSize()
    {
        return 0;
    }
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