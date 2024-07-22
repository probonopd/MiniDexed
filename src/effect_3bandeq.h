/* 
 * DISTHRO 3 Band EQ
 * Ported from https://github.com/DISTRHO/Mini-Series/blob/master/plugins/3BandEQ
 *  
 * Javier Nonis (https://github.com/jnonis) - 2024
 */
#ifndef _EFFECT_3BANDEQ_H
#define _EFFECT_3BANDEQ_H

#include <cmath>
#include "effect_base.h"

class AudioEffect3BandEQ : public AudioEffect
{
public:
    static constexpr float kAMP_DB = 8.656170245f;
    static constexpr float kDC_ADD = 1e-30f;
    static constexpr float kPI     = 3.141592654f;
    
    enum Param
    {
        BYPASS,
        EQ_LOW,
        EQ_MID,
        EQ_HIGH,
        MASTER,
        LOW_MID_FQ,
        MID_HIGH_FQ,
        UNKNOWN
    };

    AudioEffect3BandEQ(float32_t samplerate) : AudioEffect(samplerate)
    {
        // Default values
        fLow = 0.0f;
        fMid = 0.0f;
        fHigh = 0.0f;
        fMaster = 0.0f;
        fLowMidFreq = 440.0f;
        fMidHighFreq = 2000.0f;

        // Internal stuff
        lowVol = midVol = highVol = outVol = 1.0f;
        freqLP = 200.0f;
        freqHP = 2000.0f;

        // reset filter values
        xLP  = std::exp(-2.0f * kPI * freqLP / samplerate);

        a0LP = 1.0f - xLP;
        b1LP = -xLP;

        xHP  = std::exp(-2.0f * kPI * freqHP / samplerate);
        a0HP = 1.0f - xHP;
        b1HP = -xHP;
        
        out1LP = out2LP = out1HP = out2HP = 0.0f;
        tmp1LP = tmp2LP = tmp1HP = tmp2HP = 0.0f;
    }

    virtual ~AudioEffect3BandEQ()
    {
    }

    virtual unsigned getId()
    {
        return EFFECT_3BANDEQ;
    }

    virtual void setParameter(unsigned param, unsigned value)
    {
        switch (param)
        {
        case AudioEffect3BandEQ::Param::BYPASS:
            this->setBypass(value == 1);
            break;
        case AudioEffect3BandEQ::Param::EQ_LOW:
            fLow = (value / 100.0f) * 48.0f - 24.0f;
            lowVol = std::exp( (fLow/48.0f) * 48.0f / kAMP_DB);
            break;
        case AudioEffect3BandEQ::Param::EQ_MID:
            fMid = (value / 100.0f) * 48.0f - 24.0f;
            midVol = std::exp( (fMid/48.0f) * 48.0f / kAMP_DB);
            break;
        case AudioEffect3BandEQ::Param::EQ_HIGH:
            fHigh = (value / 100.0f) * 48.0f - 24.0f;
            highVol = std::exp( (fHigh/48.0f) * 48.0f / kAMP_DB);
            break;
        case AudioEffect3BandEQ::Param::MASTER:
            fMaster = (value / 100.0f) * 48.0f - 24.0f;
            outVol = std::exp( (fMaster/48.0f) * 48.0f / kAMP_DB);
            break;
        case AudioEffect3BandEQ::Param::LOW_MID_FQ:
            fLowMidFreq = std::min((float) value, fMidHighFreq);
            freqLP = fLowMidFreq;
            xLP  = std::exp(-2.0f * kPI * freqLP / (float)samplerate);
            a0LP = 1.0f - xLP;
            b1LP = -xLP;
            break;
        case AudioEffect3BandEQ::Param::MID_HIGH_FQ:
            fMidHighFreq = std::max((float) value, fLowMidFreq);
            freqHP = fMidHighFreq;
            xHP  = std::exp(-2.0f * kPI * freqHP / (float)samplerate);
            a0HP = 1.0f - xHP;
            b1HP = -xHP;
            break;
        default:
            break;
        }
    }

    virtual unsigned getParameter(unsigned param)
    {
        switch (param)
        {
        case AudioEffect3BandEQ::Param::BYPASS:
            return this->getBypass() ? 1 : 0;
        case AudioEffect3BandEQ::Param::EQ_LOW:
            return roundf(((fLow + 24.0f) / 48.0f) * 100.0f);
        case AudioEffect3BandEQ::Param::EQ_MID:
            return roundf(((fMid + 24.0f) / 48.0f) * 100.0f);
        case AudioEffect3BandEQ::Param::EQ_HIGH:
            return roundf(((fHigh + 24.0f) / 48.0f) * 100.0f);
        case AudioEffect3BandEQ::Param::MASTER:
            return roundf(((fMaster + 24.0f) / 48.0f) * 100.0f);
        case AudioEffect3BandEQ::Param::LOW_MID_FQ:
            return fLowMidFreq;
        case AudioEffect3BandEQ::Param::MID_HIGH_FQ:
            return fMidHighFreq;
        default:
            return 0;
        }
    }

protected:
    virtual size_t getParametersSize()
    {
        return AudioEffect3BandEQ::Param::UNKNOWN;
    }
    virtual void doProcess(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len)
    {
        const float* in1 = inblockL;
        const float* in2 = inblockR;
        float* out1 = outblockL;
        float* out2 = outblockR;

        for (uint32_t i=0; i < len; ++i)
        {
            tmp1LP = a0LP * in1[i] - b1LP * tmp1LP + kDC_ADD;
            tmp2LP = a0LP * in2[i] - b1LP * tmp2LP + kDC_ADD;
            out1LP = tmp1LP - kDC_ADD;
            out2LP = tmp2LP - kDC_ADD;

            tmp1HP = a0HP * in1[i] - b1HP * tmp1HP + kDC_ADD;
            tmp2HP = a0HP * in2[i] - b1HP * tmp2HP + kDC_ADD;
            out1HP = in1[i] - tmp1HP - kDC_ADD;
            out2HP = in2[i] - tmp2HP - kDC_ADD;

            out1[i] = (out1LP*lowVol + (in1[i] - out1LP - out1HP)*midVol + out1HP*highVol) * outVol;
            out2[i] = (out2LP*lowVol + (in2[i] - out2LP - out2HP)*midVol + out2HP*highVol) * outVol;
        }
    }
private:
    float fLow, fMid, fHigh, fMaster, fLowMidFreq, fMidHighFreq;

    float lowVol, midVol, highVol, outVol;
    float freqLP, freqHP;

    float xLP, a0LP, b1LP;
    float xHP, a0HP, b1HP;

    float out1LP, out2LP, out1HP, out2HP;
    float tmp1LP, tmp2LP, tmp1HP, tmp2HP;
};

#endif // _EFFECT_3BANDEQ_H
