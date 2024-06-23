#ifndef _EFFECTS_H
#define _EFFECTS_H

#include <string>
#include "effect_base.h"
#include "effect_chorus.h"
#include "effect_delay.h"
#include "effect_lpf.h"

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
	case EFFECT_NONE:
	default: return "None";
	}
}

#endif // _EFFECTS_H