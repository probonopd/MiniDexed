#include <circle/logger.h>
#include "effect_ds1.h"
#include "moddistortion/OverSample.h"

LOGMODULE ("fx ds1");

AudioEffectDS1::AudioEffectDS1(float32_t samplerate) : AudioEffect(samplerate)
{
    dist = 0.5f;
    tone = 0.5f;
    level = 0.5f;

    cont = 0;
    
    u_f = (float*)malloc(2*TAMANHO_DO_BUFFER*sizeof(float));
    y_f = (float*)malloc(2*TAMANHO_DO_BUFFER*sizeof(float));
    u = (float*)malloc(2*TAMANHO_DO_BUFFER*sizeof(float));
    y = (float*)malloc(2*TAMANHO_DO_BUFFER*sizeof(float));
    u2 = (float*)malloc(8*TAMANHO_DO_BUFFER*sizeof(float));
    y2 = (float*)malloc(8*TAMANHO_DO_BUFFER*sizeof(float));
    v1 = (float*)malloc(8*TAMANHO_DO_BUFFER*sizeof(float));
    v2 = (float*)malloc(8*TAMANHO_DO_BUFFER*sizeof(float));
    v3 = (float*)malloc(8*TAMANHO_DO_BUFFER*sizeof(float));
    
    T = 1/samplerate;
    
    h1u_1 = 0;
    h1y_1 = 0;
    
    h2u_1 = 0;
    h2y_1 = 0;
    h2u_2 = 0;
    h2y_2 = 0;
    h2u_3 = 0;
    h2y_3 = 0;
    h2u_4 = 0;
    h2y_4 = 0;
    
    h3u_1 = 0;
    h3y_1 = 0;
    h3u_2 = 0;
    h3y_2 = 0;
    
    u_1 = 0;
    y_1 = 0;
    v1_1 = 0;
    v2_1 = 0;
    v3_1 = 0;
}

AudioEffectDS1::~AudioEffectDS1()
{
    free(u_f);
	free(y_f);
	free(u);
	free(y);
	free(u2);
	free(y2);
	free(v1);
	free(v2);
	free(v3);
}

unsigned AudioEffectDS1::getId()
{
    return EFFECT_DS1;
}

void AudioEffectDS1::setParameter(unsigned param, unsigned value)
{
    switch (param)
    {
    case AudioEffectDS1::Param::BYPASS:
        this->setBypass(value == 1);
        break;
    case AudioEffectDS1::Param::DIST:
        this->dist = value / 100.0f;
        break;
    case AudioEffectDS1::Param::TONE:
        this->tone = value / 100.0f;
        break;
    case AudioEffectDS1::Param::LEVEL:
        this->level = value / 100.0f;
        break;
    default:
        break;
    }
}

unsigned AudioEffectDS1::getParameter(unsigned param)
{
    switch (param)
    {
    case AudioEffectDS1::Param::BYPASS:
		return this->getBypass() ? 1 : 0;
    case AudioEffectDS1::Param::DIST:
        return roundf(this->dist * 100);
    case AudioEffectDS1::Param::TONE:
        return roundf(this->tone * 100);
    case AudioEffectDS1::Param::LEVEL:
        return roundf(this->level * 100);
    default:
        return 0;
    }
}

void AudioEffectDS1::doProcess(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len)
{
    if( (len > TAMANHO_DO_BUFFER) && (cont == 0) )
	{
		u_f = (float*)realloc(u_f, 2*len*sizeof(float));
		y_f = (float*)realloc(y_f, 2*len*sizeof(float));
		u = (float*)realloc(u, 2*len*sizeof(float));
		y = (float*)realloc(y, 2*len*sizeof(float));
		u2 = (float*)realloc(u2, 8*len*sizeof(float));
		y2 = (float*)realloc(y2, 8*len*sizeof(float));
		v1 = (float*)realloc(v1, 8*len*sizeof(float));
		v2 = (float*)realloc(v2, 8*len*sizeof(float));
		v3 = (float*)realloc(v3, 8*len*sizeof(float));
    
		cont = 1;
    
		return;
	}
    
    float Tone;
    float Level;
    float Dist;
    
    Tone = tone;
    Level = level;
    Dist = dist;
    
    float T2;
    float T3;
    
    float SampleRate2;
    
    uint32_t n2;
    uint32_t n3;
    
    /*
    for (uint32_t i=1; i<=len; i++)
    {
		inblockL[i-1] = INPUT_GAIN * inblockL[i-1]; //15dB
	}
    */
	
	//Over 2x
	
	T2 = 0.5*T;
	SampleRate2 = 2* this->samplerate;
    Over2_Float((float *)inblockL, u_f, &h1u_1, len);
    n2 = 2*len;
    
    /*****************************************************************/
    
    if (this->samplerate == 48000)
    {
		Filter1_48000(u_f, y_f, n2, &h1u_1, &h1y_1 );
	}
	else
	{
		Filter1(u_f, y_f, n2, SampleRate2, &h1u_1, &h1y_1 );
	}
	
    /*****************************************************************/
   

    
    for (uint32_t i=1; i<=n2; i++)
    {
		u_f[i-1] = y_f[i-1]; 
	}
	
	/*****************************************************************/
	
	if (this->samplerate == 48000)
    {
		Filter2_48000(u_f, y_f, n2, &h2u_1, &h2y_1, &h2u_2, &h2y_2, &h2u_3, &h2y_3, &h2u_4, &h2y_4 );
	}
	else
	{
		Filter2(u_f, y_f, n2, SampleRate2, &h2u_1, &h2y_1, &h2u_2, &h2y_2, &h2u_3, &h2y_3, &h2u_4, &h2y_4 );
	}
	
	
   /*****************************************************************/
    
   
    
    for (uint32_t i=1; i<=n2; i++)
    {
		u_f[i-1] = y_f[i-1];
	}
	
    if (this->samplerate == 48000)
    {
		FilterGain_48000(u_f, y_f, n2, Dist, &h3u_1, &h3y_1, &h3u_2, &h3y_2 );
	}
	else
	{
		FilterGain(u_f, y_f, n2, Dist, SampleRate2, &h3u_1, &h3y_1, &h3u_2, &h3y_2 );
	}
	 
		
	//Over 4x
	
	T3 = 0.25*T2;
    Over4_Float(y_f, u2, &u_1, n2);
    n3 = 4*n2;



		DS1_Clip_Tone(u2, y2, v1, v2, v3, n3, T3, &u_1, &y_1, &v1_1, &v2_1, &v3_1, Tone, Level, &obj);
    
    /*****************************************************************/
    
    Down8_Float(outblockL, y2, len);
	
	 for (uint32_t i=1; i<=len; i++)
    {
		outblockL[i-1] = outblockL[i-1]*OUTPUT_GAIN; //-15dB
	}
}