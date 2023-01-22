#pragma once

#include "test_fixture.h"

#include "../mixing_console_constants.h"

#define AUDIO_SOURCE_FILE "test.wav"

#define SAMPLING_FREQUENCY 44100.0f

#define Active(scenarioKey, FxID) ((scenarioKey & (1 << FxID)) == (1 << FxID))

string getScenarioName(int scenario);

enum FXSwitch
{
    FX__Tube = 0,
    FX__Chorus,
    FX__Flanger,
    FX__Orbitone,
    FX__Phaser,
    FX__Delay,
    FX__ShimmerReverb,
    FX__PlateReverb
};

class FXScenarioTest : public testing::TestWithParam<int>
{
};
