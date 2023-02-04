#pragma once

#include <random>
#include <string>
#include <gtest/gtest.h>

#include "wave.h"
#include "../fx.h"

#define AUDIO_SOURCE_FILE "test2.wav"

#define SAMPLING_FREQUENCY 44100.0f

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
    FX__PlateReverb,
    __kFXCount
};

void setupOuputStreamForCSV(std::ostream& out);

bool createFolderStructure(std::string& path);

std::string getResultFile(const std::string& filename, bool createPath);

float32_t getRandomValue();

class FXScenarioTest : public testing::TestWithParam<int> {};
