/* 
 * Base AudioEffect interface
 * Javier Nonis (https://github.com/jnonis) - 2024
 */
#ifndef _EFFECT_BASE_H
#define _EFFECT_BASE_H

#include <arm_math.h>
#include <vector>
#include <string>

class AudioEffect
{
public:
    // ID must be unique for each AudioEffect
    static const unsigned ID = 0;
    static constexpr const char* NAME = "None";

    AudioEffect(float32_t samplerate)
    {
    this->samplerate = samplerate;
    }

    virtual ~AudioEffect()
    {
    }

    void setBypass(bool bypass)
    {
        this->bypass = bypass;
    }

    bool getBypass()
    {
        return this->getId() == AudioEffect::ID ? true : this->bypass;
    }

    virtual unsigned getId()
    {
        return AudioEffect::ID;
    }

    virtual std::string getName()
    {
        return AudioEffect::NAME;
    }
    
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

    void process(const float32_t* inblock, float32_t* outblock, uint16_t len)
    {
        // Mono process
        // Dummy buffer for right channel
        float32_t dummyBuffer[len];
        memset(dummyBuffer, 0, len * sizeof(float32_t));
        process(inblock, dummyBuffer, outblock, dummyBuffer, len);
    }

    void process(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len)
    {
        if (this->getBypass()) {
            if (inblockL != outblockL)
            {
                memcpy(outblockL, inblockL, len * sizeof(float32_t));
            }
            if (inblockR != outblockR) {
                memcpy(outblockR, inblockR, len * sizeof(float32_t));
            }
            return;
        }
        doProcess(inblockL, inblockR, outblockL, outblockR, len);
    }

protected:
    bool bypass = false;
    float32_t samplerate;
    
    virtual size_t getParametersSize()
    {
        return 0;
    }

    virtual void doProcess(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len)
    {
    }
};

#endif // _EFFECT_BASE_H
