/* 
 * Stereo Delay
 * Features:
 * - Tone control using Low Pass Filter
 * - Ping Pong mode.
 * Javier Nonis (https://github.com/jnonis) - 2024
 */
#ifndef _EFFECT_DELAY_H
#define _EFFECT_DELAY_H

#include "effect_base.h"
#include "effect_lpf.h"

class AudioEffectDelay : public AudioEffect
{
public:
    static const unsigned MAX_DELAY_TIME = 1;

    enum Param
    {
        BYPASS,
        TIME_L,
        TIME_R,
        FEEDBACK,
        TONE,
        PING_PONG,
        MIX,
        UNKNOWN
    };

    AudioEffectDelay(float32_t samplerate);
    virtual ~AudioEffectDelay();

    virtual unsigned getId();

    virtual void initializeSendFX();
    virtual void setParameter(unsigned param, unsigned value);
    virtual unsigned getParameter(unsigned param);
protected:
    virtual size_t getParametersSize()
    {
        return AudioEffectDelay::Param::UNKNOWN;
    }
    virtual void doProcess(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len);
private:
    size_t bufferSize;
    float32_t* bufferL;
    float32_t* bufferR;
    unsigned index;
    
    float32_t timeL; // Left delay time in seconds (0.0 - 2.0)
    float32_t timeR; // Right delay time in seconds (0.0 - 2.0)
    float32_t feedback; // Feedback (0.0 - 1.0)
    AudioEffectLPF* lpf;
    bool pingPongMode;
    float32_t mix;
    float32_t dryMix;
    float32_t wetMix;
    
    void setMix(float32_t mix);
};

#endif // _EFFECT_DELAY_H