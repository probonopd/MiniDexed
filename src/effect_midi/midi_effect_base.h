/* 
 * Base MidiEffect interface
 * Javier Nonis (https://github.com/jnonis) - 2024
 */
#ifndef _MIDI_EFFECT_H
#define _MIDI_EFFECT_H

#include <string>
#include <vector>
#include "../dexedadapter.h"

class MidiEffect
{
public:
    // ID must be unique for each MidiEffect
    static const unsigned ID = 0;
    static constexpr const char* NAME = "None";
    
    MidiEffect(float32_t samplerate, CDexedAdapter* synth)
    {
        this->samplerate = samplerate;
        this->synth = synth;
    }

    virtual ~MidiEffect()
    {
    }

    virtual unsigned getId()
    {
        return MidiEffect::ID;
    }
    
    virtual std::string getName()
    {
        return MidiEffect::NAME;
    }

    void setBypass(bool bypass)
    {
        this->bypass = bypass;
    }
    
    bool getBypass()
    {
        return this->getId() == MidiEffect::ID ? true : this->bypass;
    }

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

    void setParameters(std::vector<unsigned> params)
    {
        for (size_t i = 0; i < params.size(); i++)
        {
            this->setParameter(i, params[i]);
        }
    }

    std::vector<unsigned> getParameters()
    {
        size_t len = this->getParametersSize();
        std::vector<unsigned> params;
        for (size_t i = 0; i < len; i++)
        {
            params.push_back(getParameter(i));
        }
        return params;
    }

    virtual void keydown(int16_t pitch, uint8_t velocity)
    {
    }

    virtual void keyup(int16_t pitch)
    {
    }

    void process(uint16_t len)
    {
        if (this->getBypass())
        {
            return;
        }
        this->doProcess(len);
    }
protected:
    bool bypass = false;
    float32_t samplerate;
    CDexedAdapter* synth;

    virtual size_t getParametersSize()
    {
        return 0;
    }

    virtual void doProcess(uint16_t len)
    {
    }
};

#endif // _MIDI_EFFECT_H