/* 
 * AudioEffect Utilities
 * Javier Nonis (https://github.com/jnonis) - 2024
 */
#ifndef _EFFECTS_H
#define _EFFECTS_H

#include <string>
#include "effect_audio/effect_base.h"
#include "effect_audio/effect_chorus.h"
#include "effect_audio/effect_delay.h"
#include "effect_audio/effect_lpf.h"
#include "effect_audio/effect_ds1.h"
#include "effect_audio/effect_bigmuff.h" 
#include "effect_audio/effect_talreverb3.h"
#include "effect_audio/effect_platervbstereo.h"
#include "effect_audio/effect_mverb.h"
#include "effect_audio/effect_3bandeq.h"
#include "effect_audio/effect_phaser.h"
#include "effect_audio/effect_aphaser.h"


class AudioEffects
{
public:
	enum Types
	{
		NONE = AudioEffect::ID,
		CHORUS = AudioEffectChorus::ID,
		DELAY = AudioEffectDelay::ID,
		LPF = AudioEffectLPF::ID,
		DS1 = AudioEffectDS1::ID,
		BIGMUFF = AudioEffectBigMuff::ID,
		TALREVERB3 = AudioEffectTalReverb3::ID,
		REVERB = AudioEffectPlateReverb::ID,
		MVERB = AudioEffectMVerb::ID,
		EQ3BAND = AudioEffect3BandEQ::ID,
		PHASER = AudioEffectPhaser::ID,
		APHASER = AudioEffectAPhaser::ID,
		UNKNOWN
	};
};

inline AudioEffect* newAudioEffect(unsigned type, float32_t samplerate)
{
    switch (type)
	{
	case AudioEffects::Types::CHORUS:
		return new AudioEffectChorus(samplerate);
	case AudioEffects::Types::DELAY:
		return new AudioEffectDelay(samplerate);
	case AudioEffects::Types::LPF:
		return new AudioEffectLPF(samplerate);
	case AudioEffects::Types::DS1:
		return new AudioEffectDS1(samplerate);
	case AudioEffects::Types::BIGMUFF:
		return new AudioEffectBigMuff(samplerate);
	case AudioEffects::Types::TALREVERB3:
		return new AudioEffectTalReverb3(samplerate);
	case AudioEffects::Types::REVERB:
		return new AudioEffectPlateReverb(samplerate);
	case AudioEffects::Types::MVERB:
		return new AudioEffectMVerb(samplerate);
	case AudioEffects::Types::EQ3BAND:
		return new AudioEffect3BandEQ(samplerate);
	case AudioEffects::Types::PHASER:
		return new AudioEffectPhaser(samplerate);
	case AudioEffects::Types::APHASER:
		return new AudioEffectAPhaser(samplerate);
	case AudioEffects::Types::NONE:
	default:
		return new AudioEffect(samplerate);
	}
}

inline std::string ToFXType(int nValue)
{
	switch (nValue)
	{
	case AudioEffects::Types::CHORUS:
		return AudioEffectChorus::NAME;
	case AudioEffects::Types::DELAY:
		return AudioEffectDelay::NAME;
	case AudioEffects::Types::LPF:
		return AudioEffectLPF::NAME;
	case AudioEffects::Types::DS1:
		return AudioEffectDS1::NAME;
	case AudioEffects::Types::BIGMUFF:
		return AudioEffectBigMuff::NAME;
	case AudioEffects::Types::TALREVERB3:
		return AudioEffectTalReverb3::NAME;
	case AudioEffects::Types::REVERB:
		return AudioEffectPlateReverb::NAME;
	case AudioEffects::Types::MVERB:
		return AudioEffectMVerb::NAME;
	case AudioEffects::Types::EQ3BAND:
		return AudioEffect3BandEQ::NAME;
	case AudioEffects::Types::PHASER:
		return AudioEffectPhaser::NAME;
	case AudioEffects::Types::APHASER:
		return AudioEffectAPhaser::NAME;
	case AudioEffects::Types::NONE:
	default:
		return AudioEffect::NAME;
	}
}

inline std::string ToMix (int nValue)
{
	switch (nValue)
	{
	case 0:
		return "Dry";
	case 100:
		return "Wet";
	default:
		return std::to_string (nValue);
	}
}

inline std::string ToDelayTime (int nValue)
{
	if (nValue < (int) (AudioEffectDelay::MAX_DELAY_TIME * 1000)) {
		return std::to_string (nValue);
	}
	switch (nValue - AudioEffectDelay::MAX_DELAY_TIME * 1000)
	{
	case AudioEffectDelay::SyncTime::T_1_32:
		return "1/32";
	case AudioEffectDelay::SyncTime::T_1_24:
		return "1/24";
	case AudioEffectDelay::SyncTime::T_1_16:
		return "1/16";
	case AudioEffectDelay::SyncTime::T_1_12:
		return "1/12";
	case AudioEffectDelay::SyncTime::T_3_32:
		return "3/32";
	case AudioEffectDelay::SyncTime::T_1_8:
		return "1/8";
	case AudioEffectDelay::SyncTime::T_1_6:
		return "1/6";
	case AudioEffectDelay::SyncTime::T_3_16:
		return "3/16";
	case AudioEffectDelay::SyncTime::T_1_4:
		return "1/4";
	case AudioEffectDelay::SyncTime::T_1_3:
		return "1/3";
	case AudioEffectDelay::SyncTime::T_3_8:
		return "3/8";
	case AudioEffectDelay::SyncTime::T_1_2:
		return "1/2";
	case AudioEffectDelay::SyncTime::T_2_3:
		return "2/3";
	case AudioEffectDelay::SyncTime::T_3_4:
		return "3/4";
	case AudioEffectDelay::SyncTime::T_1_1:
	default:
		return "1/1";
	}
}

#endif // _EFFECTS_H