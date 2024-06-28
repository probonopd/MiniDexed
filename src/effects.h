#ifndef _EFFECTS_H
#define _EFFECTS_H

#include <string>
#include "effect_base.h"
#include "effect_chorus.h"
#include "effect_delay.h"
#include "effect_lpf.h"
#include "effect_ds1.h"
#include "effect_bigmuff.h" 
#include "effect_talreverb3.h"

inline AudioEffect* newAudioEffect(unsigned type, float32_t samplerate)
{
    switch (type)
	{
	case EFFECT_CHORUS:
		return new AudioEffectChorus(samplerate);
	case EFFECT_DELAY:
		return new AudioEffectDelay(samplerate);
	case EFFECT_LPF:
		return new AudioEffectLPF(samplerate);
	case EFFECT_DS1:
		return new AudioEffectDS1(samplerate);
	case EFFECT_BIGMUFF:
		return new AudioEffectBigMuff(samplerate);
	case EFFECT_TALREVERB3:
		return new AudioEffectTalReverb3(samplerate);
	case EFFECT_NONE:
	default:
		return new AudioEffectNone(samplerate);
	}
}

inline std::string getFXTypeName(int nValue)
{
	switch (nValue)
	{
	case EFFECT_CHORUS: return "Juno Chorus";
	case EFFECT_DELAY: return "Delay";
	case EFFECT_LPF: return "LP Filter";
	case EFFECT_DS1: return "DS1";
	case EFFECT_BIGMUFF: return "Big Muff";
	case EFFECT_TALREVERB3: return "Tal Reverb 3";
	case EFFECT_NONE:
	default: return "None";
	}
}

#endif // _EFFECTS_H