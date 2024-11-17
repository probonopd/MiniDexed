/* 
 * MOD Big Muff Port
 * Ported from https://github.com/moddevices/mod-distortion/tree/master/bigmuff
 *  
 * Javier Nonis (https://github.com/jnonis) - 2024
 */
#ifndef _EFFECT_BIGMUFF_H
#define _EFFECT_BIGMUFF_H

#include "effect_base.h"
#include "moddistortion/Distortion_BigMuff.h"

class AudioEffectBigMuff : public AudioEffect
{
public:
    // ID must be unique for each AudioEffect
    static const unsigned ID = 5;
    static constexpr const char* NAME = "Big Muff";

    static const unsigned TAMANHO_DO_BUFFER = 256;
    static constexpr float32_t INPUT_GAIN  = 1;
    static constexpr float32_t OUTPUT_GAIN = 1;
    
    enum Param
    {
        BYPASS,
        SUSTAIN,
        TONE,
        LEVEL,
        UNKNOWN
    };

    AudioEffectBigMuff(float32_t samplerate);
    virtual ~AudioEffectBigMuff();

    virtual unsigned getId();
    virtual std::string getName();
    virtual void setParameter(unsigned param, unsigned value);
    virtual unsigned getParameter(unsigned param);
protected:
    virtual size_t getParametersSize()
    {
        return AudioEffectBigMuff::Param::UNKNOWN;
    }
    virtual void doProcess(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len);
private:
    float tone;
    float level;
    float sustain;
    
    float *u;
    float *u2;
    float *u3;
    float *y;
    float *y2;
    float *y3;
    
    double T;
    
    float h1u_1;
    float h1y_1;
    
    float h2u_1;
    float h2y_1;
    float h2u_2;
    float h2y_2;
    float h2u_3;
    float h2y_3;
    
    float h3u_1;
    float h3y_1;
    float h3u_2;
    float h3y_2;
    float h3u_3;
    float h3y_3;
    
    float u_1;
    float y_1;
    
    float h4u_1;
    float h4y_1;
    float h4u_2;
    float h4y_2;
    float h4u_3;
    float h4y_3;
    
    int cont;
    
    double *Sust;
    int nSust;
    
    double SustainMedia_1;
};

#endif // _EFFECT_BIGMUFF