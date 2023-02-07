#pragma once

#include "fx.h"

extern const float lut_pitch_ratio_high[257];
extern const float lut_pitch_ratio_low[257];

inline float32_t semitoneToRatio(float32_t semitones)
{
    float32_t pitch = semitones + 128.0f;
    MAKE_INTEGRAL_FRACTIONAL(pitch);

    return lut_pitch_ratio_high[pitch_integral] * lut_pitch_ratio_low[static_cast<int32_t>(pitch_fractional * 256.0f)];
}
