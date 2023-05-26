// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

//
// fx_shimmer_helper.h
//
// Helper class for the FX Shimmer FX that is ported from Mutable Instruments
// Ported by: Vincent Gauché
//

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
