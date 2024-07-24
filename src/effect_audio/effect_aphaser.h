/* 
 * Phaser Port
 * Ported from https://github.com/ssj71/rkrlv2
 *  
 * Javier Nonis (https://github.com/jnonis) - 2024
 */
#ifndef _EFFECT_APHASER_H
#define _EFFECT_APHASER_H

#include "effect_base.h"
#include "rkrlv2/APhaser.h"

class AudioEffectAPhaser : public AudioEffect
{
public:
    enum Param
    {
        BYPASS,
        WETDRY,
        PAN,
        PH_FREQ,
        PH_RND,
        TYPE,
        STDL,
        PH_DEPTH,
        FB,
        STAGES,
        LRCR,
        SUB,
        PHASE,
        UNKNOWN
    };

    AudioEffectAPhaser(float32_t samplerate) : AudioEffect(samplerate)
    {
        this->phaser = new Analog_Phaser(0, 0, (double) samplerate);
        this->init_params = true;
        
        this->phaser->setpreset(0);
        /*
        this->setParameter(AudioEffectAPhaser::Param::WETDRY, 64);
        this->setParameter(AudioEffectAPhaser::Param::PAN, 64);
        this->setParameter(AudioEffectAPhaser::Param::PH_FREQ, 11);
        this->setParameter(AudioEffectAPhaser::Param::PH_RND, 0);
        this->setParameter(AudioEffectAPhaser::Param::TYPE, 0);
        this->setParameter(AudioEffectAPhaser::Param::STDL, 64);
        this->setParameter(AudioEffectAPhaser::Param::PH_DEPTH, 110);
        this->setParameter(AudioEffectAPhaser::Param::FB, 64);
        this->setParameter(AudioEffectAPhaser::Param::STAGES, 4);
        this->setParameter(AudioEffectAPhaser::Param::LRCR, 0);
        this->setParameter(AudioEffectAPhaser::Param::SUB, 0);
        this->setParameter(AudioEffectAPhaser::Param::PHASE, 20);
        */
    }

    virtual ~AudioEffectAPhaser()
    {
        delete this->phaser;
    }

    virtual unsigned getId()
    {
        return EFFECT_APHASER;
    }

    virtual void setParameter(unsigned param, unsigned value)
    {
        switch (param)
        {
        case AudioEffectAPhaser::Param::BYPASS:
            this->setBypass(value == 1);
            this->phaser->cleanup();
            break;
        case AudioEffectAPhaser::Param::WETDRY:
            this->phaser->changepar(0, value);
            break;
        case AudioEffectAPhaser::Param::PAN:
            this->phaser->changepar(1, value);
            break;
        case AudioEffectAPhaser::Param::PH_FREQ:
            this->phaser->changepar(2, value);
            break;
        case AudioEffectAPhaser::Param::PH_RND:
            this->phaser->changepar(3, value);
            break;
        case AudioEffectAPhaser::Param::TYPE:
            this->phaser->changepar(4, value);
            break;
        case AudioEffectAPhaser::Param::STDL:
            this->phaser->changepar(5, value);
            break;
        case AudioEffectAPhaser::Param::PH_DEPTH:
            this->phaser->changepar(6, value);
            break;
        case AudioEffectAPhaser::Param::FB:
            this->phaser->changepar(7, value);
            break;
        case AudioEffectAPhaser::Param::STAGES:
            this->phaser->changepar(8, value);
            break;
        case AudioEffectAPhaser::Param::LRCR:
            this->phaser->changepar(9, value);
            break;
        case AudioEffectAPhaser::Param::SUB:
            this->phaser->changepar(10, value);
            break;
        case AudioEffectAPhaser::Param::PHASE:
            this->phaser->changepar(11, value);
            break;
        default:
            break;
        }
    }

    virtual unsigned getParameter(unsigned param)
    {
        switch (param)
        {
        case AudioEffectAPhaser::Param::BYPASS:
            return this->getBypass() ? 1 : 0;
        case AudioEffectAPhaser::Param::WETDRY:
            return this->phaser->getpar(0);
        case AudioEffectAPhaser::Param::PAN:
            return this->phaser->getpar(1);
        case AudioEffectAPhaser::Param::PH_FREQ:
            return this->phaser->getpar(2);
        case AudioEffectAPhaser::Param::PH_RND:
            return this->phaser->getpar(3);
        case AudioEffectAPhaser::Param::TYPE:
            return this->phaser->getpar(4);
        case AudioEffectAPhaser::Param::STDL:
            return this->phaser->getpar(5);
        case AudioEffectAPhaser::Param::PH_DEPTH:
            return this->phaser->getpar(6);
        case AudioEffectAPhaser::Param::FB:
            return this->phaser->getpar(7);
        case AudioEffectAPhaser::Param::STAGES:
            return this->phaser->getpar(8);
        case AudioEffectAPhaser::Param::LRCR:
            return this->phaser->getpar(9);
        case AudioEffectAPhaser::Param::SUB:
            return this->phaser->getpar(10);
        case AudioEffectAPhaser::Param::PHASE:
            return this->phaser->getpar(11);
        default:
            return 0;
        }
    }
   
protected:
    virtual size_t getParametersSize()
    {
        return AudioEffectAPhaser::Param::UNKNOWN;
    }

    virtual void doProcess(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len)
    {
        // LFO effects require period be set before setting other params
        if(this->init_params)
        {
            this->phaser->PERIOD = len;
            this->phaser->lfo->updateparams(len);
            this->init_params = false; // so we only do this once
        }

        // now set out ports and global period size
        this->phaser->efxoutl = outblockL;
        this->phaser->efxoutr = outblockR;

        //now run
        this->phaser->out((float*) inblockL, (float*) inblockR, len);
    }

private:
    Analog_Phaser* phaser;
    bool init_params;
};

#endif // _EFFECT_PHASER_H