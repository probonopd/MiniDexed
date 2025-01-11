/* 
 * MOD DS-1 Port
 * Ported from https://github.com/moddevices/mod-distortion/tree/master/ds1
 *  
 * Javier Nonis (https://github.com/jnonis) - 2024
 */
#ifndef _EFFECT_DS1_H
#define _EFFECT_DS1_H

#include "effect_base.h"
#include "moddistortion/Distortion_DS1.h"

class AudioEffectDS1 : public AudioEffect
{
public:
    // ID must be unique for each AudioEffect
    static const unsigned ID = 4;
    static constexpr const char* NAME = "DS1";
#ifdef ARM_ALLOW_MULTI_CORE
    static const unsigned TAMANHO_DO_BUFFER = 256;
#else
    static const unsigned TAMANHO_DO_BUFFER = 1024;
#endif
    static constexpr float32_t INPUT_GAIN  = 1;
    static constexpr float32_t OUTPUT_GAIN = 1;
    
    enum Param
    {
        BYPASS,
        DIST,
        TONE,
        LEVEL,
        UNKNOWN
    };

    AudioEffectDS1(float32_t samplerate);
    virtual ~AudioEffectDS1();

    virtual unsigned getId();
    virtual std::string getName();
    virtual void setParameter(unsigned param, unsigned value);
    virtual unsigned getParameter(unsigned param);
protected:
    virtual size_t getParametersSize()
    {
        return AudioEffectDS1::Param::UNKNOWN;
    }
    virtual void doProcess(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len);
private:
    float tone;
    float level;
    float dist;
    
    float *u_f;
    float *y_f;
    
    float *u;
    float *u2;
    float *y;
    float *y2;
    float *v1;
    float *v2;
    float *v3;
    
    float T;
    
    float h1u_1;
    float h1y_1;
    
    float h2u_1;
    float h2y_1;
    float h2u_2;
    float h2y_2;
    float h2u_3;
    float h2y_3;
    float h2u_4;
    float h2y_4;
    
    float h3u_1;
    float h3y_1;
    float h3u_2;
    float h3y_2;
    
    float u_1;
    float y_1;
    float v1_1;
    float v2_1;
    float v3_1;
    
    int cont;
    
    ClipClass obj;
};

#endif // _EFFECT_DS1_H