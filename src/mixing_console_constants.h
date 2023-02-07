#pragma once

#include "extra_features.h"

#include <array>
#include <cstring>
#include <stdexcept>

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
    FX_Reverberator,
    MainOutput,
    kFXCount
};

inline std::string toString(MixerOutput enum_val)
{
    static const std::array<const char*, MixerOutput::kFXCount> names
    { 
        "Tube",
        "Chorus",
        "Flanger",
        "Orbitone",
        "Phaser",
        "Delay",
        "PlateReverb",
        "Reverberator",
        "MainOutput"
    };
    static_assert(names.size() == MixerOutput::kFXCount, "Enum MixerOutput and string array size mismatch");

    return std::string(names[static_cast<size_t>(enum_val)]);
}

#define TO_INDEX_CHECK(str, fx) if(std::strcmp(str, toString(fx).c_str()) == 0) return fx;

inline MixerOutput toIndex(const char* str)
{
    TO_INDEX_CHECK(str, MixerOutput::FX_Tube);
    TO_INDEX_CHECK(str, MixerOutput::FX_Chorus);
    TO_INDEX_CHECK(str, MixerOutput::FX_Flanger);
    TO_INDEX_CHECK(str, MixerOutput::FX_Orbitone);
    TO_INDEX_CHECK(str, MixerOutput::FX_Phaser);
    TO_INDEX_CHECK(str, MixerOutput::FX_Delay);
    TO_INDEX_CHECK(str, MixerOutput::FX_PlateReverb);
    TO_INDEX_CHECK(str, MixerOutput::FX_Reverberator);
    TO_INDEX_CHECK(str, MixerOutput::MainOutput);

    throw std::invalid_argument("Invalid MixerOutput string");
}
