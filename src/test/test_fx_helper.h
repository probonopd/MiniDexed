#pragma once

#include <random>
#include <string>
#include <gtest/gtest.h>

#include "../fx.h"
#include "../mixing_console_constants.h"

#define AUDIO_SOURCE_FILE "test.wav"

#define SAMPLING_FREQUENCY 44100.0f

#define stringify( x ) # x

#define Active(scenarioKey, FxID) ((scenarioKey & (1 << FxID)) == (1 << FxID))

std::string getScenarioName(int scenario);

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

void setupOuputStreamFocCSV(std::ostream& out);

std::string getResultFile(const string& filename);

float32_t getRandomValue();

class FXScenarioTest : public testing::TestWithParam<int> {};
