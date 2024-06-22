#ifndef _EFFECT_DELAY_H
#define _EFFECT_DELAY_H

#include "effect.h"

class AudioEffectDelay : public AudioEffect
{
public:
    AudioEffectDelay(float32_t samplerate);
    virtual ~AudioEffectDelay();

    virtual unsigned getId();
protected:
    virtual void doProcess(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len);
// private:
    // const size_t MaxSampleDelayTime;
    // unsigned write_pos_L_;
    // unsigned write_pos_R_;
    // float32_t* buffer_L_;
    // float32_t* buffer_R_;
    // float32_t delay_time_L_;        // Left delay time in seconds (0.0 - 2.0)
    // float32_t delay_time_R_;        // Right delay time in seconds (0.0 - 2.0)
    // float32_t feedback_;            // Feedback (0.0 - 1.0)
};

#endif // _EFFECT_DELAY_H