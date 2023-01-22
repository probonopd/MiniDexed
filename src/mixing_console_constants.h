#pragma once

#include "extra_features.h"

#include <array>
#include <cstring>
#include <stdexcept>

enum StereoChannels
{
    Left = 0,
    Right,
    kNumChannels
};

enum MixerOutput
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

inline const char* toString(MixerOutput enum_val)
{
    static constexpr std::array<const char*, MixerOutput::kFXCount> names
    { 
        "Tube",
        "Chorus",
        "Flanger",
        "Orbitone",
        "Phaser",
        "Delay",
        "PlateReverb",
        "ShimmerReverb",
        "MainOutput"
    };
    static_assert(names.size() == MixerOutput::kFXCount, "Enum MixerOutput and string array size mismatch");

    return names[static_cast<size_t>(enum_val)];
}

inline MixerOutput toIndex(const char* str)
{
    if(strcmp(str, "Tube") == 0) return MixerOutput::FX_Tube;
    if(strcmp(str, "Chorus") == 0) return MixerOutput::FX_Chorus;
    if(strcmp(str, "Flanger") == 0) return MixerOutput::FX_Flanger;
    if(strcmp(str, "Orbitone") == 0) return MixerOutput::FX_Orbitone;
    if(strcmp(str, "Phaser") == 0) return MixerOutput::FX_Phaser;
    if(strcmp(str, "Delay") == 0) return MixerOutput::FX_Delay;
    if(strcmp(str, "PlateReverb") == 0) return MixerOutput::FX_PlateReverb;
    if(strcmp(str, "ShimmerReverb") == 0) return MixerOutput::FX_ShimmerReverb;
    if(strcmp(str, "MainOutput") == 0) return MixerOutput::MainOutput;

    throw std::invalid_argument("Invalid MixerOutput string");
}
