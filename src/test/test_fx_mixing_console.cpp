#include <gtest/gtest.h>
#include <cmath>

#include "test_fx_helper.h"
#include "wave.h"

#include "../mixing_console.hpp"

class MixingConsoleScenarioTest : public testing::TestWithParam<MixerOutput> {};

TEST_P(MixingConsoleScenarioTest, MixerOutputTest)
{
    MixerOutput v = this->GetParam();
    std::string str = toString(v);
    MixerOutput mo = toIndex(str.c_str());
    EXPECT_EQ(v, mo);
}

INSTANTIATE_TEST_SUITE_P(MixerOutputTest, MixingConsoleScenarioTest, testing::Range(MixerOutput::OutputStart, MixerOutput::kFXCount));

typedef MixingConsole<8> Mixer;

void setupMixingConsoleFX(Mixer* mixer)
{
    mixer->getTube()->setMute(false);
    mixer->getTube()->setOverdrive(0.25f);

    mixer->getChorus()->setMute(false);
    mixer->getChorus()->setRate(0.4f);
    mixer->getChorus()->setDepth(0.5f);
    
    mixer->getFlanger()->setMute(false);
    mixer->getFlanger()->setRate(0.03f);
    mixer->getFlanger()->setDepth(0.75f);
    mixer->getFlanger()->setFeedback(0.5f);

    mixer->getOrbitone()->setMute(false);
    mixer->getOrbitone()->setRate(0.4f);
    mixer->getOrbitone()->setDepth(0.5f);

    mixer->getPhaser()->setMute(false);
    mixer->getPhaser()->setRate(0.1f);
    mixer->getPhaser()->setDepth(1.0f);
    mixer->getPhaser()->setFeedback(0.5f);
    mixer->getPhaser()->setNbStages(12);

    mixer->getDelay()->setMute(false);
    mixer->getDelay()->setLeftDelayTime(0.5f);
    mixer->getDelay()->setLeftDelayTime(0.7f);
    mixer->getDelay()->setFeedback(0.7f);
    mixer->getDelay()->setFlutterRate(0.7f);
    mixer->getDelay()->setFlutterAmount(0.7f);

    mixer->getPlateReverb()->setMute(false);
    mixer->getPlateReverb()->set_bypass(false);
    mixer->getPlateReverb()->size(0.7f);
    mixer->getPlateReverb()->hidamp(0.5f);
    mixer->getPlateReverb()->lodamp(0.5f);
    mixer->getPlateReverb()->lowpass(0.3f);
    mixer->getPlateReverb()->diffusion(0.65f);
    mixer->getPlateReverb()->level(1.0f);

    mixer->getShimmerReverb()->setMute(false);
    mixer->getShimmerReverb()->setInputGain(0.65f);
    mixer->getShimmerReverb()->setTime(0.89f);
    mixer->getShimmerReverb()->setDiffusion(0.75f);
    mixer->getShimmerReverb()->setLP(0.8f);
}

#define ACTIVE_FX(scenarioId, fx) const bool b ## fx = ((scenarioId & MixerOutput::fx) == MixerOutput::fx)

void setupMixingConsoleFX(Mixer* mixer, int scenarioId)
{
    ACTIVE_FX(scenarioId, FX_Tube);
    ACTIVE_FX(scenarioId, FX_Chorus);
    ACTIVE_FX(scenarioId, FX_Flanger);
    ACTIVE_FX(scenarioId, FX_Orbitone);
    ACTIVE_FX(scenarioId, FX_Phaser);
    ACTIVE_FX(scenarioId, FX_Delay);
    ACTIVE_FX(scenarioId, FX_PlateReverb);
    ACTIVE_FX(scenarioId, FX_ShimmerReverb);

    mixer->setChannelLevel(0, 1.0f);
    mixer->setPan(0, 0.5f);


}

TEST(MixingConsole, ZeroSamplesTest)
{
    const size_t length = 4;
    Mixer* mixer = new Mixer(SAMPLING_FREQUENCY, length);
    EXPECT_EQ(0, FULL_INSPECT(mixer, true));

    mixer->reset();
    EXPECT_EQ(0, FULL_INSPECT(mixer, true));

    setupMixingConsoleFX(mixer);
    EXPECT_EQ(0, FULL_INSPECT(mixer, true));

    mixer->setChannelLevel(0, 1.0f);
    EXPECT_EQ(0, FULL_INSPECT(mixer, true));

    mixer->setPan(0, 0.5f);
    EXPECT_EQ(0, FULL_INSPECT(mixer, true));

    float32_t samples[] = {0.0f, 0.0f, 0.0f, 0.0f};
    mixer->setInputSampleBuffer(0, samples);
    EXPECT_EQ(0, FULL_INSPECT(mixer, true));

    mixer->setSendLevel(0, MixerOutput::MainOutput, 1.0f);
    EXPECT_EQ(0, FULL_INSPECT(mixer, true));

    float32_t outL[4];
    float32_t outR[4];
    mixer->process(outL, outR);
    EXPECT_EQ(0, FULL_INSPECT(mixer, true));

    mixer->setSendLevel(0, MixerOutput::FX_Tube, 1.0f);
    mixer->setSendLevel(0, MixerOutput::FX_Delay, 1.0f);
    mixer->setSendLevel(0, MixerOutput::FX_PlateReverb, 1.0f);

    mixer->setReturnLevel(MixerOutput::FX_Tube, MixerOutput::FX_Orbitone, 1.0f);
    mixer->setReturnLevel(MixerOutput::FX_Orbitone, MixerOutput::MainOutput, 0.5f);
    mixer->setReturnLevel(MixerOutput::FX_Orbitone, MixerOutput::FX_PlateReverb, 1.0f);
    mixer->setReturnLevel(MixerOutput::FX_Delay, MixerOutput::MainOutput, 0.5f);
    mixer->setReturnLevel(MixerOutput::FX_PlateReverb, MixerOutput::MainOutput, 0.5f);
    EXPECT_EQ(0, FULL_INSPECT(mixer, true));

    delete mixer;
}

TEST(MixingConsole, DryProcessing)
{
    const float32_t epsilon = 1e-7;
    const size_t length = 2;

    Mixer* mixer = new Mixer(SAMPLING_FREQUENCY, length);
    mixer->reset();

    mixer->setSendLevel(0, MixerOutput::FX_Tube, 0.0f);
    mixer->setSendLevel(0, MixerOutput::FX_Chorus, 0.0f);
    mixer->setSendLevel(0, MixerOutput::FX_Flanger, 0.0f);
    mixer->setSendLevel(0, MixerOutput::FX_Orbitone, 0.0f);
    mixer->setSendLevel(0, MixerOutput::FX_Phaser, 0.0f);
    mixer->setSendLevel(0, MixerOutput::FX_Delay, 0.0f);
    mixer->setSendLevel(0, MixerOutput::FX_PlateReverb, 0.0f);
    mixer->setSendLevel(0, MixerOutput::FX_ShimmerReverb, 0.0f);

    for(unsigned i = MixerOutput::OutputStart; i < (MixerOutput::kFXCount - 1); ++i)
    {
        mixer->setReturnLevel(static_cast<MixerOutput>(i), MixerOutput::MainOutput, 0.0f);
    }

    mixer->setChannelLevel(0, 1.0f);
    mixer->setPan(0, 0.5f);
    mixer->setSendLevel(0, MixerOutput::MainOutput, 1.0f);
    EXPECT_EQ(0, INSPECT(mixer, fullInspector));

    float32_t in[length] = {0.1, 0.2};
    float32_t out[StereoChannels::kNumChannels][length];
    for(size_t i = 0; i < StereoChannels::kNumChannels; ++i) memset(out[i], 0, length * sizeof(float32_t));

    mixer->setInputSampleBuffer(0, in);
    EXPECT_EQ(0, INSPECT(mixer, fullInspector));

    mixer->process(
        out[StereoChannels::Left ],
        out[StereoChannels::Right]
    );
    EXPECT_EQ(0, INSPECT(mixer, fullInspector));

    EXPECT_EQ(out[StereoChannels::Left ][0], out[StereoChannels::Right][0]);
    EXPECT_EQ(out[StereoChannels::Left ][1], out[StereoChannels::Right][1]);
    EXPECT_LT(std::abs(out[StereoChannels::Left ][0] - (sqrt(2.0f) / 20.0f)), epsilon);
    EXPECT_LT(std::abs(out[StereoChannels::Left ][1] - (sqrt(2.0f) / 10.0f)), epsilon);

    delete mixer;
}

TEST(MixingConsole, ShimmerProcessing)
{
    const float32_t epsilon = 1e-7;
    const size_t length = 2;

    Mixer* mixer = new Mixer(SAMPLING_FREQUENCY, length);
    mixer->reset();

    mixer->setSendLevel(0, MixerOutput::MainOutput, 0.0f);
    mixer->setSendLevel(0, MixerOutput::FX_ShimmerReverb, 1.0f);
    mixer->setReturnLevel(MixerOutput::FX_ShimmerReverb, MixerOutput::MainOutput, 1.0f);
    mixer->setChannelLevel(0, 1.0f);
    mixer->setPan(0, 0.5f);
    EXPECT_EQ(0, INSPECT(mixer, fullInspector));

    float32_t in[length] = {0.1, 0.2};
    float32_t out1[StereoChannels::kNumChannels][length];
    for(size_t i = 0; i < StereoChannels::kNumChannels; ++i) memset(out1[i], 0, length * sizeof(float32_t));

    mixer->setInputSampleBuffer(0, in);
    EXPECT_EQ(0, INSPECT(mixer, fullInspector));

    mixer->process(
        out1[StereoChannels::Left ],
        out1[StereoChannels::Right]
    );
    EXPECT_EQ(0, INSPECT(mixer, fullInspector));

    mixer->reset();
    EXPECT_EQ(0, INSPECT(mixer, fullInspector));

    float32_t out2[StereoChannels::kNumChannels][length];
    mixer->setInputSampleBuffer(0, in);
    EXPECT_EQ(0, INSPECT(mixer, fullInspector));

    mixer->process(
        out2[StereoChannels::Left ],
        out2[StereoChannels::Right]
    );
    EXPECT_EQ(0, INSPECT(mixer, fullInspector));

    EXPECT_EQ(out1[StereoChannels::Left ][0], out2[StereoChannels::Left ][0]);
    EXPECT_EQ(out1[StereoChannels::Right][0], out2[StereoChannels::Right][0]);
    EXPECT_EQ(out1[StereoChannels::Left ][1], out2[StereoChannels::Left ][1]);
    EXPECT_EQ(out1[StereoChannels::Right][1], out2[StereoChannels::Right][1]);

    delete mixer;
}

TEST(MixingConsole, ShimmerNoiseProcessing)
{
    const size_t length = 1024;

    Mixer* mixer = new Mixer(SAMPLING_FREQUENCY, length);
    mixer->reset();

    mixer->setSendLevel(0, MixerOutput::MainOutput, 0.0f);
    mixer->setSendLevel(0, MixerOutput::FX_ShimmerReverb, 1.0f);
    mixer->setReturnLevel(MixerOutput::FX_ShimmerReverb, MixerOutput::MainOutput, 1.0f);
    mixer->setChannelLevel(0, 1.0f);
    mixer->setPan(0, 0.5f);
    EXPECT_EQ(0, INSPECT(mixer, fullInspector));

    float32_t in[length];
    for(size_t i = 0; i < length; ++i) in[i] = getRandomValue();
    
    float32_t out[StereoChannels::kNumChannels][length];
    for(size_t i = 0; i < StereoChannels::kNumChannels; ++i) memset(out[i], 0, length * sizeof(float32_t));

    mixer->setInputSampleBuffer(0, in);
    EXPECT_EQ(0, INSPECT(mixer, fullInspector));

    mixer->process(
        out[StereoChannels::Left ],
        out[StereoChannels::Right]
    );
    EXPECT_EQ(0, INSPECT(mixer, fullInspector));

    delete mixer;
}

TEST(MixingConsole, StandardUsageProcessing)
{
    const testing::TestInfo* test_info = testing::UnitTest::GetInstance()->current_test_info();
    std::string full_test_name = test_info->test_case_name();
    full_test_name += ".";
    full_test_name += test_info->name();

    const unsigned nbRepeats = 2;
    size_t size;
    float32_t** samples = readWaveFile(AUDIO_SOURCE_FILE, size);
    float32_t* sampleOutL = new float32_t[size * nbRepeats];
    float32_t* sampleOutR = new float32_t[size * nbRepeats];
    memset(sampleOutL, 0, size * nbRepeats * sizeof(float32_t));
    memset(sampleOutR, 0, size * nbRepeats * sizeof(float32_t));

    Mixer* mixer = new Mixer(SAMPLING_FREQUENCY, size);

    setupMixingConsoleFX(mixer);

    mixer->getTube()->setOverdrive(0.15f);
    mixer->setSendLevel(0, MixerOutput::FX_Tube, 1.0f);
    mixer->setSendLevel(0, MixerOutput::FX_Phaser, 1.0f);
    // mixer->setReturnLevel(MixerOutput::FX_Tube, MixerOutput::MainOutput, 1.0f);
    // mixer->setSendLevel(0, MixerOutput::FX_Chorus, 1.0f);
    // mixer->setSendLevel(0, MixerOutput::FX_ShimmerReverb, 1.0f);
    mixer->setReturnLevel(MixerOutput::FX_Tube, MixerOutput::FX_Chorus, 1.0f);
    mixer->setReturnLevel(MixerOutput::FX_Chorus, MixerOutput::FX_ShimmerReverb, 1.0f);
    mixer->setReturnLevel(MixerOutput::FX_Phaser, MixerOutput::FX_Delay, 1.0f);

    mixer->setSendLevel(0, MixerOutput::MainOutput, 0.25f);
    mixer->setReturnLevel(MixerOutput::FX_Tube, MixerOutput::MainOutput, 0.1f);
    mixer->setReturnLevel(MixerOutput::FX_Chorus, MixerOutput::MainOutput, 0.15f);
    mixer->setReturnLevel(MixerOutput::FX_ShimmerReverb, MixerOutput::MainOutput, 0.3f);
    mixer->setReturnLevel(MixerOutput::FX_Delay, MixerOutput::MainOutput, 0.3f);

    for(unsigned j = 0; j < nbRepeats; ++j)
    {
        mixer->setInputSampleBuffer(0, samples[0], samples[1]);
        mixer->process(sampleOutL + j * size, sampleOutR + j * size);
        EXPECT_EQ(0, INSPECT(mixer, fullInspector));
    }
    saveWaveFile(getResultFile(full_test_name + ".wav", true), sampleOutL, sampleOutR, nbRepeats * size, static_cast<unsigned>(SAMPLING_FREQUENCY), 16);

    delete mixer;

    delete[] samples[0];
    delete[] samples[1];
    delete[] samples;
    delete[] sampleOutL;
    delete[] sampleOutR;
}

// TEST_P(FXScenarioTest, FXProcessingScenario)
// {
//     const testing::TestInfo* test_info = testing::UnitTest::GetInstance()->current_test_info();
//     std::string full_test_name = test_info->test_case_name();
//     full_test_name += ".";
//     full_test_name += test_info->name();

//     const unsigned nbRepeats = 2;
//     size_t size;
//     float32_t** samples = readWaveFile(AUDIO_SOURCE_FILE, size);
//     float32_t* sampleOutL = new float32_t[size * nbRepeats];
//     float32_t* sampleOutR = new float32_t[size * nbRepeats];
//     memset(sampleOutL, 0, size * nbRepeats * sizeof(float32_t));
//     memset(sampleOutR, 0, size * nbRepeats * sizeof(float32_t));

//     Mixer* mixer = new Mixer(SAMPLING_FREQUENCY, size);

//     setupMixingConsoleFX(mixer);

//     int scenarioId = this->GetParam();
//     setupMixingConsoleFXForScenario(mixer, scenarioId);

//     for(unsigned j = 0; j < nbRepeats; ++j)
//     {
//         mixer->setInputSampleBuffer(0, samples[0], samples[1]);
//         mixer->process(sampleOutL + j * size, sampleOutR + j * size);
//         EXPECT_EQ(0, INSPECT(mixer, fullInspector));
//     }
//     saveWaveFile(getResultFile(full_test_name + ".wav"), sampleOutL, sampleOutR, nbRepeats * size, static_cast<unsigned>(SAMPLING_FREQUENCY), 16);

//     delete mixer;

//     delete[] samples[0];
//     delete[] samples[1];
//     delete[] samples;
//     delete[] sampleOutL;
//     delete[] sampleOutR;
// }

// INSTANTIATE_TEST_SUITE_P(MixingConsole, FXScenarioTest, testing::Range(0, 1 << (MixerOutput::kFXCount - 1)));
