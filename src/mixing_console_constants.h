#pragma once

#include "extra_features.h"

enum StereoChannels
{
    Left = 0,
    Right,
    kNumChannels
};

enum class MixerOutput
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

template<typename T>
std::string_view getMixerOutputString(T enum_val)
{
    static constexpr std::array<std::string_view, static_cast<size_t>(T::kFXCount)> names
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
    static_assert(names.size() == static_cast<size_t>(T::kFXCount),"Enum class and string array size mismatch");
    return names[static_cast<size_t>(enum_val)];
}

MixerOutput getMixerOutputFromString(std::string str)
{
    if(str == "Tube") return MixerOutput::FX_Tube;
    if(str == "Chorus") return MixerOutput::FX_Chorus;
    if(str == "Flanger") return MixerOutput::FX_Flanger;
    if(str == "Orbitone") return MixerOutput::FX_Orbitone;
    if(str == "Phaser") return MixerOutput::FX_Phaser;
    if(str == "Delay") return MixerOutput::FX_Delay;
    if(str == "PlateReverb") return MixerOutput::FX_PlateReverb;
    if(str == "ShimmerReverb") return MixerOutput::FX_ShimmerReverb;
    if(str == "MainOutput") return MixerOutput::MainOutput;

    throw std::invalid_argument("Invalid MixerOutput string");
}
