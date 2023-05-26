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
// test_fx_helper.h
//
// Set og helpers dedicated to code rationalization for the elaboration on unit tests. 
// Author: Vincent Gauché
//
#pragma once

#include <random>
#include <string>
#include <gtest/gtest.h>

#include "wave.h"
#include "../fx.h"

#define AUDIO_SOURCE_FILE "test3.wav"

#define SAMPLING_FREQUENCY 44100.0f

#define PREPARE_AUDIO_TEST(size, inSamples, outSamples, full_test_name)\
    const testing::TestInfo* test_info = testing::UnitTest::GetInstance()->current_test_info();\
    std::string full_test_name = test_info->test_case_name();\
    full_test_name += ".";\
    full_test_name += test_info->name();\
    WaveHeader hdr;\
    size_t size;\
    float32_t** inSamples = loadAudioTest(size, &hdr);\
    float32_t** outSamples = new float32_t*[2];\
    outSamples[0] = new float32_t[size]; memset(outSamples[0], 0, size * sizeof(float32_t));\
    outSamples[1] = new float32_t[size]; memset(outSamples[1], 0, size * sizeof(float32_t))

#define CLEANUP_AUDIO_TEST(inSamples, outSamples)\
    freeAudioSamples(inSamples, 2);\
    freeAudioSamples(outSamples, 2)

#define AUDIO_LOOP(inSamples, outSamples, size, inL, inR, outL, outR, code)\
    for(size_t i = 0; i < size; ++i)\
    {\
        float32_t inL = inSamples[0][i];\
        float32_t inR = inSamples[1][i];\
        float32_t outL;\
        float32_t outR;\
        code\
        outSamples[0][i] = outL;\
        outSamples[1][i] = outR;\
    } //

#define SIMPLE_AUDIO_LOOP(inSamples, outSamples, size, inL, inR, outL, outR, fx)\
    AUDIO_LOOP(inSamples, outSamples, size, inL, inR, outL, outR, fx.processSample(inL, inR, outL, outR);)

#define SAVE_AUDIO_RESULTS(filename, samples, size)\
    saveWaveFile(getResultFile(filename + ".wav", true), samples[0], samples[1], size, static_cast<unsigned>(SAMPLING_FREQUENCY), 16)

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
    FX__Reverberator,
    FX__PlateReverb,
    __kFXCount
};

void setupOuputStreamForCSV(std::ostream& out);

bool createFolderStructure(std::string& path);

std::string getResultFile(const std::string& filename, bool createPath);

float32_t getRandomValue();

class FXScenarioTest : public testing::TestWithParam<int> {};

float32_t** loadAudioTest(size_t& size, WaveHeader* hdr);
void freeAudioSamples(float32_t** samples, size_t size);