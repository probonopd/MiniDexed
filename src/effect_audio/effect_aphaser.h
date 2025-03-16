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
    // ID must be unique for each AudioEffect
    static const unsigned ID = 11;
    static constexpr const char* NAME = "A Phaser";

    enum Param
    {
        BYPASS,
        WETDRY,
        DISTORTION,
        PH_FREQ,
        PH_RND,
        TYPE,
        STDL,
        WIDTH,
        FB,
        STAGES,
        OFFSET,
        SUB,
        PH_DEPTH,
        HYPER,
        UNKNOWN
    };

    AudioEffectAPhaser(float32_t samplerate) : AudioEffect(samplerate)
    {
        this->phaser = new Analog_Phaser(0, 0, (double) samplerate);
        this->init_params = true;
        
        this->phaser->setpreset(0);
    }

    virtual ~AudioEffectAPhaser()
    {
        delete this->phaser;
    }

    virtual unsigned getId()
    {
        return AudioEffectAPhaser::ID;
    }

    virtual std::string getName()
    {
        return AudioEffectAPhaser::NAME;
    }

    virtual void initializeSendFX()
    {
        this->setParameter(AudioEffectAPhaser::Param::WETDRY, 127);
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
        case AudioEffectAPhaser::Param::DISTORTION:
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
        case AudioEffectAPhaser::Param::WIDTH:
            this->phaser->changepar(6, value);
            break;
        case AudioEffectAPhaser::Param::FB:
            this->phaser->changepar(7, value);
            break;
        case AudioEffectAPhaser::Param::STAGES:
            this->phaser->changepar(8, value);
            break;
        case AudioEffectAPhaser::Param::OFFSET:
            this->phaser->changepar(9, value);
            break;
        case AudioEffectAPhaser::Param::SUB:
            this->phaser->changepar(10, value);
            break;
        case AudioEffectAPhaser::Param::PH_DEPTH:
            this->phaser->changepar(11, value);
            break;
        case AudioEffectAPhaser::Param::HYPER:
            this->phaser->changepar(12, value);
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
        case AudioEffectAPhaser::Param::DISTORTION:
            return this->phaser->getpar(1);
        case AudioEffectAPhaser::Param::PH_FREQ:
            return this->phaser->getpar(2);
        case AudioEffectAPhaser::Param::PH_RND:
            return this->phaser->getpar(3);
        case AudioEffectAPhaser::Param::TYPE:
            return this->phaser->getpar(4);
        case AudioEffectAPhaser::Param::STDL:
            return this->phaser->getpar(5);
        case AudioEffectAPhaser::Param::WIDTH:
            return this->phaser->getpar(6);
        case AudioEffectAPhaser::Param::FB:
            return this->phaser->getpar(7);
        case AudioEffectAPhaser::Param::STAGES:
            return this->phaser->getpar(8);
        case AudioEffectAPhaser::Param::OFFSET:
            return this->phaser->getpar(9);
        case AudioEffectAPhaser::Param::SUB:
            return this->phaser->getpar(10);
        case AudioEffectAPhaser::Param::PH_DEPTH:
            return this->phaser->getpar(11);
        case AudioEffectAPhaser::Param::HYPER:
            return this->phaser->getpar(12);
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