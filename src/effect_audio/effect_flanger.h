/* 
 * Flanger / Chorus Port
 * Ported from https://github.com/ssj71/rkrlv2
 * Ported from https://github.com/zynaddsubfx/zynaddsubfx
 *  
 * Javier Nonis (https://github.com/jnonis) - 2024
 */
#ifndef _EFFECT_FLANGER_H
#define _EFFECT_FLANGER_H

#include "effect_base.h"
#include "rkrlv2/Chorus.h"

class AudioEffectFlanger : public AudioEffect
{
public:
    // ID must be unique for each AudioEffect
    static const unsigned ID = 12;
    static constexpr const char* NAME = "Flanger";

    enum Param
    {
        BYPASS,
        MIX,
        PAN,
        FL_FREQ,
        FL_RND,
        TYPE,
        STDL,
        FL_DEPTH,
        DELAY,
        FB,
        LRCR,
        MODE,
        SUB,
        AWESOME,
        UNKNOWN
    };

    AudioEffectFlanger(float32_t samplerate) : AudioEffect(samplerate)
    {
        this->chorus = new RKRChorus(0, 0, (double) samplerate);
        this->init_params = true;
        
        this->chorus->setpreset(5);
    }

    virtual ~AudioEffectFlanger()
    {
        delete this->chorus;
    }

    virtual unsigned getId()
    {
        return AudioEffectFlanger::ID;
    }

    virtual std::string getName()
    {
        return AudioEffectFlanger::NAME;
    }

    virtual void initializeSendFX()
    {
        this->setParameter(AudioEffectFlanger::Param::MIX, 127);
    }
    
    virtual void setParameter(unsigned param, unsigned value)
    {
        switch (param)
        {
        case AudioEffectFlanger::Param::BYPASS:
            this->setBypass(value == 1);
            this->chorus->cleanup();
            break;
        case AudioEffectFlanger::Param::MIX:
            this->chorus->changepar(0, value);
            break;
        case AudioEffectFlanger::Param::PAN:
            this->chorus->changepar(1, value);
            break;
        case AudioEffectFlanger::Param::FL_FREQ:
            this->chorus->changepar(2, value);
            break;
        case AudioEffectFlanger::Param::FL_RND:
            this->chorus->changepar(3, value);
            break;
        case AudioEffectFlanger::Param::TYPE:
            this->chorus->changepar(4, value);
            break;
        case AudioEffectFlanger::Param::STDL:
            this->chorus->changepar(5, value);
            break;
        case AudioEffectFlanger::Param::FL_DEPTH:
            this->chorus->changepar(6, value);
            break;
        case AudioEffectFlanger::Param::DELAY:
            this->chorus->changepar(7, value);
            break;
        case AudioEffectFlanger::Param::FB:
            this->chorus->changepar(8, value);
            break;
        case AudioEffectFlanger::Param::LRCR:
            this->chorus->changepar(9, value);
            break;
        case AudioEffectFlanger::Param::MODE:
            this->chorus->changepar(10, value);
            break;
        case AudioEffectFlanger::Param::SUB:
            this->chorus->changepar(11, value);
            break;
        case AudioEffectFlanger::Param::AWESOME:
            this->chorus->changepar(12, value);
            break;
        default:
            break;
        }
    }

    virtual unsigned getParameter(unsigned param)
    {
        switch (param)
        {
        case AudioEffectFlanger::Param::BYPASS:
            return this->getBypass() ? 1 : 0;
        case AudioEffectFlanger::Param::MIX:
            return this->chorus->getpar(0);
        case AudioEffectFlanger::Param::PAN:
            return this->chorus->getpar(1);
        case AudioEffectFlanger::Param::FL_FREQ:
            return this->chorus->getpar(2);
        case AudioEffectFlanger::Param::FL_RND:
            return this->chorus->getpar(3);
        case AudioEffectFlanger::Param::TYPE:
            return this->chorus->getpar(4);
        case AudioEffectFlanger::Param::STDL:
            return this->chorus->getpar(5);
        case AudioEffectFlanger::Param::FL_DEPTH:
            return this->chorus->getpar(6);
        case AudioEffectFlanger::Param::DELAY:
            return this->chorus->getpar(7);
        case AudioEffectFlanger::Param::FB:
            return this->chorus->getpar(8);
        case AudioEffectFlanger::Param::LRCR:
            return this->chorus->getpar(9);
        case AudioEffectFlanger::Param::MODE:
            return this->chorus->getpar(10);
        case AudioEffectFlanger::Param::SUB:
            return this->chorus->getpar(11);
        case AudioEffectFlanger::Param::AWESOME:
            return this->chorus->getpar(12);
        default:
            return 0;
        }
    }

protected:
    virtual size_t getParametersSize()
    {
        return AudioEffectFlanger::Param::UNKNOWN;
    }

    virtual void doProcess(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len)
    {
        // LFO effects require period be set before setting other params
        if(this->init_params)
        {
            this->chorus->PERIOD = len;
            this->init_params = false; // so we only do this once
        }

        // now set out ports and global period size
        this->chorus->efxoutl = outblockL;
        this->chorus->efxoutr = outblockR;

        //now run
        this->chorus->out((float*) inblockL, (float*) inblockR, len);
    }

private:
    RKRChorus* chorus;
    bool init_params;
};

#endif // _EFFECT_FLANGER_H