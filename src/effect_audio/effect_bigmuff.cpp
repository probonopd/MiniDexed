/* 
 * MOD Big Muff Port
 * Ported from https://github.com/moddevices/mod-distortion/tree/master/bigmuff
 *  
 * Javier Nonis (https://github.com/jnonis) - 2024
 */
#include <circle/logger.h>
#include "effect_bigmuff.h"
#include "moddistortion/OverSample.h"

LOGMODULE ("fx bigmuff");

AudioEffectBigMuff::AudioEffectBigMuff(float32_t samplerate) : AudioEffect(samplerate)
{
    sustain = 0.5f;
    tone = 0.5f;
    level = 0.5f;

    cont = 0;
    
    u = (float*)malloc(2*TAMANHO_DO_BUFFER*sizeof(float));
    y = (float*)malloc(2*TAMANHO_DO_BUFFER*sizeof(float));
    u2 = (float*)malloc(8*TAMANHO_DO_BUFFER*sizeof(float));
    y2 = (float*)malloc(8*TAMANHO_DO_BUFFER*sizeof(float));
    u3 = (float*)malloc(TAMANHO_DO_BUFFER*sizeof(float));
    y3 = (float*)malloc(TAMANHO_DO_BUFFER*sizeof(float));
    
    nSust = 50;
    Sust = (double*)calloc(nSust,sizeof(double));
    
    SustainMedia_1 = 0.5;
    
    T = 1/samplerate;
        
    h1u_1 = 0;
    h1y_1 = 0;
    
    h2u_1 = 0;
    h2y_1 = 0;
    h2u_2 = 0;
    h2y_2 = 0;
    h2u_3 = 0;
    h2y_3 = 0;
    
    h3u_1 = 0;
    h3y_1 = 0;
    h3u_2 = 0;
    h3y_2 = 0;
    h3u_3 = 0;
    h3y_3 = 0;
    
    u_1 = 0;
    y_1 = 0;
    
    h4u_1 = 0;
    h4y_1 = 0;
    h4u_2 = 0;
    h4y_2 = 0;
    h4u_3 = 0;
    h4y_3 = 0;
}

AudioEffectBigMuff::~AudioEffectBigMuff()
{
    free(u);
	free(y);
	free(u2);
	free(y2);
	free(u3);
	free(y3);
	
	free(Sust);
}

unsigned AudioEffectBigMuff::getId()
{
    return EFFECT_BIGMUFF;
}

void AudioEffectBigMuff::setParameter(unsigned param, unsigned value)
{
    switch (param)
    {
    case AudioEffectBigMuff::Param::BYPASS:
        this->setBypass(value == 1);
        break;
    case AudioEffectBigMuff::Param::SUSTAIN:
        this->sustain = value / 100.0f;
        break;
    case AudioEffectBigMuff::Param::TONE:
        this->tone = value / 100.0f;
        break;
    case AudioEffectBigMuff::Param::LEVEL:
        this->level = value / 100.0f;
        break;
    default:
        break;
    }
}

unsigned AudioEffectBigMuff::getParameter(unsigned param)
{
    switch (param)
    {
    case AudioEffectBigMuff::Param::BYPASS:
		return this->getBypass() ? 1 : 0;
    case AudioEffectBigMuff::Param::SUSTAIN:
        return roundf(this->sustain * 100);
    case AudioEffectBigMuff::Param::TONE:
        return roundf(this->tone * 100);
    case AudioEffectBigMuff::Param::LEVEL:
        return roundf(this->level * 100);
    default:
        return 0;
    }
}

void AudioEffectBigMuff::doProcess(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len)
{
    if( (len > TAMANHO_DO_BUFFER) && (cont == 0) )
	{
		u = (float*)realloc(u, 2*len*sizeof(float));
		y = (float*)realloc(y, 2*len*sizeof(float));
		u2 = (float*)realloc(u2, 8*len*sizeof(float));
		y2 = (float*)realloc(y2, 8*len*sizeof(float));
		u3 = (float*)realloc(u3, len*sizeof(float));
		y3 = (float*)realloc(y3, len*sizeof(float));
    
		cont = 1;
    
		return;
	}
    
    double Tone;
    double Level;
    double Sustain;
    double SustainMedia = 0;
    
    Tone = tone;
    Level = level;
    Sustain = sustain;
    
    for (int i=0; i<nSust-1; i++)
    {
		Sust[i] = Sust[i+1];
	}
    Sust[nSust-1] = Sustain;
    
    for (int i=0; i<nSust; i++)
    {
		SustainMedia = SustainMedia + Sust[i];
	}
	
	SustainMedia = SustainMedia/nSust;
    
    double T2;
    double T3;
    uint32_t n2;
    uint32_t n3;
    
    /*
    for (uint32_t i=1; i<=len; i++)
    {
		in[i-1] = INPUT_GAIN*in[i-1]; //15dB
	}
    */
	
	//Over 2x
	
	T2 = 0.5*T;
    Over2_Float((float *)inblockL, u, &h1u_1, len);
    n2 = 2*len;
    
    /*****************************************************************/
    
	BM_Filter1(u, y, n2, T2, &h1u_1, &h1y_1 );
	
    /*****************************************************************/
   
    for (uint32_t i=1; i<=n2; i++)
    {
		u[i-1] = y[i-1];
	}
	
	/*****************************************************************/
	
	BM_Filter2(u, y, n2, T2, &h2u_1, &h2y_1, &h2u_2, &h2y_2, &h2u_3, &h2y_3 );
	
   /*****************************************************************/
       
    for (uint32_t i=1; i<=n2; i++)
    {
		u[i-1] = y[i-1]; 
	}
	
	/*****************************************************************/
	
	BM_Filter3(u, y, n2, T2, &h3u_1, &h3y_1, &h3u_2, &h3y_2, SustainMedia, SustainMedia_1 );
	 
	/*****************************************************************/
			
	//Over 4x
	
	T3 = 0.25*T2;
    Over4_Float(y, u2, &u_1, n2);
    n3 = 4*n2;
    
    /*****************************************************************/

	BM_Clip(u2, y2, n3, T3, &u_1, &y_1);
    
    /*****************************************************************/
    
    for (uint32_t i=1; i<=n3; i++)
    {
		u2[i-1] = y2[i-1]; 
	}
	
	/*****************************************************************/
    
    Down8_Float(u3, y2, len);
    
    /*****************************************************************/
    
    BM_Filter4(u3, y3, len, T, &h4u_1, &h4y_1, &h4u_2, &h4y_2, &h4u_3, &h4y_3, Tone, Level);
	
	 /*****************************************************************/
	
	 for (uint32_t i=1; i<=len; i++)
    {
		outblockL[i-1] = y3[i-1]*OUTPUT_GAIN; //-26dB
        outblockR[i-1] = outblockL[i-1];
	}
	
	SustainMedia_1 = SustainMedia;
}