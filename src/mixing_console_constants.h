#pragma once

#include "extra_features.h"

enum StereoChannels : std::size_t
{
    Left = 0,
    Right,
    kNumChannels
};

enum MixerOutput : std::size_t
{
    OutputStart = 0,
    FX_Tube = 0,
    FX_Chorus,
    FX_Flanger,
    FX_Orbitone,
    FX_Phaser,
    FX_Delay,
    FX_PlateReverb,
    FX_ShimmerReverb,
    MainOutput,
    kFXCount
};

extern std::string_view toString(MixerOutput enum_val);
extern MixerOutput toIndex(std::string str);
