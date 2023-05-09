#include <gtest/gtest.h>
#include <cmath>

#include "test_fx_helper.h"
#include "wave.h"

#include "../mixing_console.hpp"

#define NB_MIXER_CHANNELS 8

class MixingConsoleScenarioTest : public testing::TestWithParam<MixerOutput> {};

typedef MixingConsole<NB_MIXER_CHANNELS> Mixer;

void setupMixingConsoleFX(Mixer* mixer);
void setupMixingConsoleFX(Mixer* mixer, int scenarioId, size_t channel = 0);

TEST_P(MixingConsoleScenarioTest, MixerOutputTest)
{
    MixerOutput v = this->GetParam();
    std::string str = toString(v);
    MixerOutput mo = toIndex(str.c_str());
    ASSERT_EQ(v, mo);
}

INSTANTIATE_TEST_SUITE_P(MixerOutputTest, MixingConsoleScenarioTest, testing::Range(MixerOutput::OutputStart, MixerOutput::kFXCount));

void setupMixingConsoleFX(Mixer* mixer)
{
    mixer->setChannelLevel(0, 1.0f);
    mixer->setPan(0, 0.5f);

    mixer->getTube()->setMute(false);
    mixer->getTube()->setOverdrive(0.85f);

    mixer->getChorus()->setMute(false);
    mixer->getChorus()->setRate(0.4f);
    mixer->getChorus()->setDepth(0.7f);
    
    mixer->getFlanger()->setMute(false);
    mixer->getFlanger()->setRate(0.03f);
    mixer->getFlanger()->setDepth(0.75f);
    mixer->getFlanger()->setFeedback(0.7f);

    mixer->getOrbitone()->setMute(false);
    mixer->getOrbitone()->setRate(0.4f);
    mixer->getOrbitone()->setDepth(0.8f);

    mixer->getPhaser()->setMute(false);
    mixer->getPhaser()->setRate(0.1f);
    mixer->getPhaser()->setDepth(1.0f);
    mixer->getPhaser()->setFeedback(0.7f);
    mixer->getPhaser()->setNbStages(12);

    mixer->getDelay()->setMute(false);
    mixer->getDelay()->setLeftDelayTime(0.15f);
    mixer->getDelay()->setLeftDelayTime(0.20f);
    mixer->getDelay()->setFeedback(0.7f);
    mixer->getDelay()->setFlutterRate(0.2f);
    mixer->getDelay()->setFlutterAmount(0.5f);

    mixer->getPlateReverb()->setMute(false);
    mixer->getPlateReverb()->set_bypass(false);
    mixer->getPlateReverb()->size(0.7f);
    mixer->getPlateReverb()->hidamp(0.5f);
    mixer->getPlateReverb()->lodamp(0.5f);
    mixer->getPlateReverb()->lowpass(0.3f);
    mixer->getPlateReverb()->diffusion(0.65f);
    mixer->getPlateReverb()->level(1.0f);

    mixer->getReverberator()->setMute(false);
    mixer->getReverberator()->setInputGain(0.65f);
    mixer->getReverberator()->setTime(0.89f);
    mixer->getReverberator()->setDiffusion(0.75f);
    mixer->getReverberator()->setLP(0.8f);
}

#define ACTIVE_FX(activity, scenarioId, fx) activity[MixerOutput::fx] = ((scenarioId & (1 << MixerOutput::fx)) == (1 << MixerOutput::fx))

void setupMixingConsoleFX(Mixer* mixer, int scenarioId, size_t channel)
{
    mixer->setChannelLevel(channel, 1.0f);
    mixer->setPan(channel, 0.5f);

    bool fxActivity[MixerOutput::kFXCount - 1];
    ACTIVE_FX(fxActivity, scenarioId, FX_Tube);
    ACTIVE_FX(fxActivity, scenarioId, FX_Chorus);
    ACTIVE_FX(fxActivity, scenarioId, FX_Flanger);
    ACTIVE_FX(fxActivity, scenarioId, FX_Orbitone);
    ACTIVE_FX(fxActivity, scenarioId, FX_Phaser);
    ACTIVE_FX(fxActivity, scenarioId, FX_Delay);
    ACTIVE_FX(fxActivity, scenarioId, FX_PlateReverb);
    ACTIVE_FX(fxActivity, scenarioId, FX_Reverberator);

    size_t nbActiveFX = 0;
    MixerOutput previousActivatedFX = MixerOutput::MainOutput;
    
    for(size_t i = 0; i < (MixerOutput::kFXCount - 1); ++i)
    {
        if(fxActivity[i])
        {
            nbActiveFX++; 
            if(nbActiveFX == 1)
            {
                mixer->setSendLevel(channel, static_cast<MixerOutput>(i), 1.0f);
            }
            else
            {
                mixer->setReturnLevel(previousActivatedFX, static_cast<MixerOutput>(i), 1.0f);
            }
            previousActivatedFX = static_cast<MixerOutput>(i);
        }
    }

    if(previousActivatedFX == MixerOutput::MainOutput)
    {
        mixer->setSendLevel(channel, MixerOutput::MainOutput, 1.0f);
    }
    else
    {
        mixer->setSendLevel(channel, MixerOutput::MainOutput, 0.25f);
        mixer->setReturnLevel(previousActivatedFX, MixerOutput::MainOutput, 0.75f);
    }
}

TEST(MixingConsole, ZeroSamplesTest)
{
    static const size_t length = 4;

    Mixer mixer(SAMPLING_FREQUENCY, length);
    ASSERT_EQ(0, FULL_INSPECT((&mixer), true));

    mixer.reset();
    ASSERT_EQ(0, FULL_INSPECT((&mixer), true));

    setupMixingConsoleFX(&mixer);
    ASSERT_EQ(0, FULL_INSPECT((&mixer), true));

    mixer.setChannelLevel(0, 1.0f);
    ASSERT_EQ(0, FULL_INSPECT((&mixer), true));

    mixer.setPan(0, 0.5f);
    ASSERT_EQ(0, FULL_INSPECT((&mixer), true));

    float32_t samples[] = {0.0f, 0.0f, 0.0f, 0.0f};
    mixer.setInputSampleBuffer(0, samples);
    mixer.preProcessInputSampleBuffer(0, 4);
    ASSERT_EQ(0, FULL_INSPECT((&mixer), true));

    mixer.setSendLevel(0, MixerOutput::MainOutput, 1.0f);
    ASSERT_EQ(0, FULL_INSPECT((&mixer), true));

    float32_t outL[4];
    float32_t outR[4];
    mixer.process(outL, outR);
    ASSERT_EQ(0, FULL_INSPECT((&mixer), true));

    mixer.setSendLevel(0, MixerOutput::FX_Tube, 1.0f);
    mixer.setSendLevel(0, MixerOutput::FX_Delay, 1.0f);
    mixer.setSendLevel(0, MixerOutput::FX_PlateReverb, 1.0f);

    mixer.setReturnLevel(MixerOutput::FX_Tube, MixerOutput::FX_Orbitone, 1.0f);
    mixer.setReturnLevel(MixerOutput::FX_Orbitone, MixerOutput::MainOutput, 0.5f);
    mixer.setReturnLevel(MixerOutput::FX_Orbitone, MixerOutput::FX_PlateReverb, 1.0f);
    mixer.setReturnLevel(MixerOutput::FX_Delay, MixerOutput::MainOutput, 0.5f);
    mixer.setReturnLevel(MixerOutput::FX_PlateReverb, MixerOutput::MainOutput, 0.5f);
    ASSERT_EQ(0, FULL_INSPECT((&mixer), true));
}

TEST(MixingConsole, DryProcessing)
{
    static const float32_t epsilon = 1e-4;
    static const size_t length = 2;

    Mixer mixer(SAMPLING_FREQUENCY, length);
    mixer.reset();
    mixer.setChannelLevel(0, 1.0f);
    mixer.setPan(0, 0.5f);

    mixer.setSendLevel(0, MixerOutput::FX_Tube, 0.0f);
    mixer.setSendLevel(0, MixerOutput::FX_Chorus, 0.0f);
    mixer.setSendLevel(0, MixerOutput::FX_Flanger, 0.0f);
    mixer.setSendLevel(0, MixerOutput::FX_Orbitone, 0.0f);
    mixer.setSendLevel(0, MixerOutput::FX_Phaser, 0.0f);
    mixer.setSendLevel(0, MixerOutput::FX_Delay, 0.0f);
    mixer.setSendLevel(0, MixerOutput::FX_PlateReverb, 0.0f);
    mixer.setSendLevel(0, MixerOutput::FX_Reverberator, 0.0f);

    for(size_t i = MixerOutput::OutputStart; i < (MixerOutput::kFXCount - 1); ++i)
    {
        mixer.setReturnLevel(static_cast<MixerOutput>(i), MixerOutput::MainOutput, 0.0f);
    }

    mixer.setSendLevel(0, MixerOutput::MainOutput, 1.0f);
    ASSERT_EQ(0, INSPECT((&mixer), fullInspector));

    float32_t in[length] = {0.1, 0.2};
    float32_t out[StereoChannels::kNumChannels][length];
    for(size_t i = 0; i < StereoChannels::kNumChannels; ++i) memset(out[i], 0, length * sizeof(float32_t));

    mixer.setInputSampleBuffer(0, in);
    mixer.preProcessInputSampleBuffer(0, 2);
    ASSERT_EQ(0, INSPECT((&mixer), fullInspector));

    mixer.process(
        out[StereoChannels::Left ],
        out[StereoChannels::Right]
    );
    ASSERT_EQ(0, INSPECT((&mixer), fullInspector));

    EXPECT_FLOAT_EQ(out[StereoChannels::Left ][0], out[StereoChannels::Right][0]);
    EXPECT_FLOAT_EQ(out[StereoChannels::Left ][1], out[StereoChannels::Right][1]);
    EXPECT_NEAR(out[StereoChannels::Left ][0], sqrt(2.0f) / 20.0f, epsilon);
    EXPECT_NEAR(out[StereoChannels::Left ][1], sqrt(2.0f) / 10.0f, epsilon);
}

TEST(MixingConsole, ReverberatorProcessing)
{
    static const float32_t epsilon = 1e-7;
    static const size_t length = 2;

    Mixer mixer(SAMPLING_FREQUENCY, length);
    mixer.reset();
    mixer.setChannelLevel(0, 1.0f);
    mixer.setPan(0, 0.5f);

    mixer.setSendLevel(0, MixerOutput::MainOutput, 0.0f);
    mixer.setSendLevel(0, MixerOutput::FX_Reverberator, 1.0f);
    mixer.setReturnLevel(MixerOutput::FX_Reverberator, MixerOutput::MainOutput, 1.0f);
    ASSERT_EQ(0, INSPECT((&mixer), fullInspector));

    float32_t in[length] = {0.1, 0.2};
    float32_t out1[StereoChannels::kNumChannels][length];
    for(size_t i = 0; i < StereoChannels::kNumChannels; ++i) memset(out1[i], 0, length * sizeof(float32_t));

    mixer.setInputSampleBuffer(0, in);
    ASSERT_EQ(0, INSPECT((&mixer), fullInspector));

    mixer.process(
        out1[StereoChannels::Left ],
        out1[StereoChannels::Right]
    );
    ASSERT_EQ(0, INSPECT((&mixer), fullInspector));

    mixer.reset();
    ASSERT_EQ(0, INSPECT((&mixer), fullInspector));

    float32_t out2[StereoChannels::kNumChannels][length];
    mixer.setInputSampleBuffer(0, in);
    ASSERT_EQ(0, INSPECT((&mixer), fullInspector));

    mixer.process(
        out2[StereoChannels::Left ],
        out2[StereoChannels::Right]
    );
    ASSERT_EQ(0, INSPECT((&mixer), fullInspector));

    EXPECT_NEAR(out1[StereoChannels::Left ][0], out2[StereoChannels::Left ][0], epsilon);
    EXPECT_NEAR(out1[StereoChannels::Right][0], out2[StereoChannels::Right][0], epsilon);
    EXPECT_NEAR(out1[StereoChannels::Left ][1], out2[StereoChannels::Left ][1], epsilon);
    EXPECT_NEAR(out1[StereoChannels::Right][1], out2[StereoChannels::Right][1], epsilon);
}

TEST(MixingConsole, ReverberatorNoiseProcessing)
{
    static const size_t length = 1024;

    Mixer mixer(SAMPLING_FREQUENCY, length);
    mixer.reset();
    mixer.setChannelLevel(0, 1.0f);
    mixer.setPan(0, 0.5f);

    mixer.setSendLevel(0, MixerOutput::MainOutput, 0.0f);
    mixer.setSendLevel(0, MixerOutput::FX_Reverberator, 1.0f);
    mixer.setReturnLevel(MixerOutput::FX_Reverberator, MixerOutput::MainOutput, 1.0f);
    ASSERT_EQ(0, INSPECT((&mixer), fullInspector));

    float32_t in[length];
    for(size_t i = 0; i < length; ++i) in[i] = getRandomValue();
    
    float32_t out[StereoChannels::kNumChannels][length];
    for(size_t i = 0; i < StereoChannels::kNumChannels; ++i) memset(out[i], 0, length * sizeof(float32_t));

    mixer.setInputSampleBuffer(0, in);
    ASSERT_EQ(0, INSPECT((&mixer), fullInspector));

    mixer.process(
        out[StereoChannels::Left ],
        out[StereoChannels::Right]
    );
    ASSERT_EQ(0, INSPECT((&mixer), fullInspector));
}

TEST(MixingConsole, StandardUsageProcessingByInjection)
{
    PREPARE_AUDIO_TEST(size, inSamples, outSamples, full_test_name);

    Mixer mixer(SAMPLING_FREQUENCY, size);
    setupMixingConsoleFX(&mixer);

    mixer.getTube()->setOverdrive(0.15f);

    mixer.setSendLevel(0, MixerOutput::FX_Tube, 1.0f);
    mixer.setSendLevel(0, MixerOutput::FX_Phaser, 1.0f);
    // mixer.setReturnLevel(MixerOutput::FX_Tube, MixerOutput::MainOutput, 1.0f);
    // mixer.setSendLevel(0, MixerOutput::FX_Chorus, 1.0f);
    // mixer.setSendLevel(0, MixerOutput::FX_Reverberator, 1.0f);
    mixer.setReturnLevel(MixerOutput::FX_Tube, MixerOutput::FX_Chorus, 1.0f);
    mixer.setReturnLevel(MixerOutput::FX_Chorus, MixerOutput::FX_Reverberator, 1.0f);
    mixer.setReturnLevel(MixerOutput::FX_Phaser, MixerOutput::FX_Delay, 1.0f);

    mixer.setSendLevel(0, MixerOutput::MainOutput, 0.25f);
    mixer.setReturnLevel(MixerOutput::FX_Tube, MixerOutput::MainOutput, 0.1f);
    mixer.setReturnLevel(MixerOutput::FX_Chorus, MixerOutput::MainOutput, 0.15f);
    mixer.setReturnLevel(MixerOutput::FX_Reverberator, MixerOutput::MainOutput, 0.3f);
    mixer.setReturnLevel(MixerOutput::FX_Delay, MixerOutput::MainOutput, 0.3f);

    mixer.injectInputSamples(0, inSamples[StereoChannels::Left], inSamples[StereoChannels::Right], size);
    mixer.process(outSamples[0], outSamples[1]);
    ASSERT_EQ(0, INSPECT((&mixer), fullInspector));
    saveWaveFile(getResultFile(full_test_name + ".wav", true), outSamples[0], outSamples[1], size, static_cast<unsigned>(SAMPLING_FREQUENCY), 16);

    CLEANUP_AUDIO_TEST(inSamples, outSamples);
}

TEST(MixingConsole, StandardUsageProcessing)
{
    static const size_t MAX_BUFFER_SIZE = 4096;
    static const size_t BUFFER_SIZE = 256;

    PREPARE_AUDIO_TEST(size, inSamples, outSamples, full_test_name);

    Mixer mixer(SAMPLING_FREQUENCY, MAX_BUFFER_SIZE);

    float32_t channelBuffer[MAX_BUFFER_SIZE];
    memset(channelBuffer, 0, MAX_BUFFER_SIZE * sizeof(float32_t));
    mixer.setInputSampleBuffer(0, channelBuffer);

    setupMixingConsoleFX(&mixer);

    mixer.getTube()->setOverdrive(0.15f);

    mixer.setSendLevel(0, MixerOutput::FX_Tube, 1.0f);
    mixer.setSendLevel(0, MixerOutput::FX_Phaser, 1.0f);
    // mixer.setReturnLevel(MixerOutput::FX_Tube, MixerOutput::MainOutput, 1.0f);
    // mixer.setSendLevel(0, MixerOutput::FX_Chorus, 1.0f);
    // mixer.setSendLevel(0, MixerOutput::FX_Reverberator, 1.0f);
    mixer.setReturnLevel(MixerOutput::FX_Tube, MixerOutput::FX_Chorus, 1.0f);
    mixer.setReturnLevel(MixerOutput::FX_Chorus, MixerOutput::FX_Reverberator, 1.0f);
    mixer.setReturnLevel(MixerOutput::FX_Phaser, MixerOutput::FX_Delay, 1.0f);

    mixer.setSendLevel(0, MixerOutput::MainOutput, 0.25f);
    mixer.setReturnLevel(MixerOutput::FX_Tube, MixerOutput::MainOutput, 0.1f);
    mixer.setReturnLevel(MixerOutput::FX_Chorus, MixerOutput::MainOutput, 0.15f);
    mixer.setReturnLevel(MixerOutput::FX_Reverberator, MixerOutput::MainOutput, 0.3f);
    mixer.setReturnLevel(MixerOutput::FX_Delay, MixerOutput::MainOutput, 0.3f);

    float32_t* inS = inSamples[StereoChannels::Left];
    float32_t* outS[StereoChannels::kNumChannels];
    outS[StereoChannels::Left ] = outSamples[StereoChannels::Left ];
    outS[StereoChannels::Right] = outSamples[StereoChannels::Right];
    size_t s = size;

    while(s > 0)
    {
        size_t nb = (s > BUFFER_SIZE) ? BUFFER_SIZE : s;

        memcpy(channelBuffer, inS, nb * sizeof(float32_t));

        mixer.preProcessInputSampleBuffer(0, nb);
        mixer.process(outS[StereoChannels::Left ], outS[StereoChannels::Right]);
        // ASSERT_EQ(0, INSPECT((&mixer), fullInspector));

        inS += nb;
        outS[StereoChannels::Left ] += nb;
        outS[StereoChannels::Right] += nb;
        s -= nb;
    }
    
    saveWaveFile(getResultFile(full_test_name + ".wav", true), outSamples[0], outSamples[1], size, static_cast<unsigned>(SAMPLING_FREQUENCY), 16);

    CLEANUP_AUDIO_TEST(inSamples, outSamples);
}

TEST(MixingConsole, StandardUsageProcessingAllMixerChannels)
{
    static const size_t MAX_BUFFER_SIZE = 4096;
    static const size_t BUFFER_SIZE = 256;

    PREPARE_AUDIO_TEST(size, inSamples, outSamples, full_test_name);

    Mixer mixer(SAMPLING_FREQUENCY, MAX_BUFFER_SIZE);

    float32_t channelBuffer[NB_MIXER_CHANNELS][MAX_BUFFER_SIZE];
    for(size_t i = 0; i < NB_MIXER_CHANNELS; ++i)
    {
        memset(channelBuffer[i], 0, MAX_BUFFER_SIZE * sizeof(float32_t));
        mixer.setInputSampleBuffer(i, channelBuffer[i]);
    }

    setupMixingConsoleFX(&mixer);

    mixer.getTube()->setOverdrive(0.15f);

    mixer.setSendLevel(0, MixerOutput::FX_Tube, 1.0f);
    mixer.setSendLevel(0, MixerOutput::FX_Phaser, 1.0f);
    // mixer.setReturnLevel(MixerOutput::FX_Tube, MixerOutput::MainOutput, 1.0f);
    // mixer.setSendLevel(0, MixerOutput::FX_Chorus, 1.0f);
    // mixer.setSendLevel(0, MixerOutput::FX_Reverberator, 1.0f);
    mixer.setReturnLevel(MixerOutput::FX_Tube, MixerOutput::FX_Chorus, 1.0f);
    mixer.setReturnLevel(MixerOutput::FX_Chorus, MixerOutput::FX_Reverberator, 1.0f);
    mixer.setReturnLevel(MixerOutput::FX_Phaser, MixerOutput::FX_Delay, 1.0f);

    mixer.setSendLevel(0, MixerOutput::MainOutput, 0.25f);
    mixer.setReturnLevel(MixerOutput::FX_Tube, MixerOutput::MainOutput, 0.1f);
    mixer.setReturnLevel(MixerOutput::FX_Chorus, MixerOutput::MainOutput, 0.15f);
    mixer.setReturnLevel(MixerOutput::FX_Reverberator, MixerOutput::MainOutput, 0.3f);
    mixer.setReturnLevel(MixerOutput::FX_Delay, MixerOutput::MainOutput, 0.3f);

    float32_t* inS = inSamples[StereoChannels::Left];
    float32_t* outS[StereoChannels::kNumChannels];
    outS[StereoChannels::Left ] = outSamples[StereoChannels::Left ];
    outS[StereoChannels::Right] = outSamples[StereoChannels::Right];
    size_t s = size;

    while(s > 0)
    {
        size_t nb = (s > BUFFER_SIZE) ? BUFFER_SIZE : s;

        memcpy(channelBuffer[0], inS, nb * sizeof(float32_t));

        for(size_t i = 0; i < mixer.getChannelNumber(); ++i)
        {
            mixer.preProcessInputSampleBuffer(i, nb);
        }
        mixer.process(outS[StereoChannels::Left ], outS[StereoChannels::Right]);

        inS += nb;
        outS[StereoChannels::Left ] += nb;
        outS[StereoChannels::Right] += nb;
        s -= nb;
    }
    
    saveWaveFile(getResultFile(full_test_name + ".wav", true), outSamples[0], outSamples[1], size, static_cast<unsigned>(SAMPLING_FREQUENCY), 16);

    CLEANUP_AUDIO_TEST(inSamples, outSamples);
}

TEST(MixingConsole, StandardUsageProcessingAllMixerChannels2)
{
    static const size_t MAX_BUFFER_SIZE = 4096;
    static const size_t BUFFER_SIZE = 256;

    PREPARE_AUDIO_TEST(size, inSamples, outSamples, full_test_name);

    Mixer mixer(SAMPLING_FREQUENCY, MAX_BUFFER_SIZE);

    float32_t channelBuffer[NB_MIXER_CHANNELS][MAX_BUFFER_SIZE];
    for(size_t i = 0; i < NB_MIXER_CHANNELS; ++i)
    {
        memset(channelBuffer[i], 0, MAX_BUFFER_SIZE * sizeof(float32_t));
        mixer.setInputSampleBuffer(i, channelBuffer[i]);
    }

    setupMixingConsoleFX(&mixer);

    for(size_t i = 0; i < NB_MIXER_CHANNELS; ++i)
    {
        mixer.setSendLevel(i, static_cast<MixerOutput>(i), 1.0f);
        mixer.setReturnLevel(static_cast<MixerOutput>(i), MixerOutput::MainOutput, 0.5f);
        mixer.setSendLevel(i, MixerOutput::MainOutput, 0.5f);
    }

    float32_t* inS = inSamples[StereoChannels::Left];
    float32_t* outS[StereoChannels::kNumChannels];
    outS[StereoChannels::Left ] = outSamples[StereoChannels::Left ];
    outS[StereoChannels::Right] = outSamples[StereoChannels::Right];
    size_t s = size;

    while(s > 0)
    {
        size_t nb = (s > BUFFER_SIZE) ? BUFFER_SIZE : s;

        for(size_t i = 0; i < mixer.getChannelNumber(); ++i)
        {
            memcpy(channelBuffer[i], inS, nb * sizeof(float32_t));
            mixer.preProcessInputSampleBuffer(i, nb);
        }
        mixer.process(outS[StereoChannels::Left ], outS[StereoChannels::Right]);

        inS += nb;
        outS[StereoChannels::Left ] += nb;
        outS[StereoChannels::Right] += nb;
        s -= nb;
    }
    
    saveWaveFile(getResultFile(full_test_name + ".wav", true), outSamples[0], outSamples[1], size, static_cast<unsigned>(SAMPLING_FREQUENCY), 16);

    CLEANUP_AUDIO_TEST(inSamples, outSamples);
}

TEST_P(FXScenarioTest, FXProcessingScenario)
{
    static const size_t MAX_BUFFER_SIZE = 4096;
    static const size_t BUFFER_SIZE = 256;

    PREPARE_AUDIO_TEST(size, inSamples, outSamples, full_test_name);

    Mixer mixer(SAMPLING_FREQUENCY, MAX_BUFFER_SIZE);

    float32_t channelBuffer[NB_MIXER_CHANNELS][MAX_BUFFER_SIZE];
    for(size_t i = 0; i < NB_MIXER_CHANNELS; ++i)
    {
        memset(channelBuffer[i], 0, MAX_BUFFER_SIZE * sizeof(float32_t));
        mixer.setInputSampleBuffer(i, channelBuffer[i]);
    }

    setupMixingConsoleFX(&mixer);

    int scenarioId = this->GetParam();
    setupMixingConsoleFX((&mixer), scenarioId);

    float32_t* inS = inSamples[StereoChannels::Left];
    float32_t* outS[StereoChannels::kNumChannels];
    outS[StereoChannels::Left ] = outSamples[StereoChannels::Left ];
    outS[StereoChannels::Right] = outSamples[StereoChannels::Right];
    size_t s = size;

    while(s > 0)
    {
        size_t nb = (s > BUFFER_SIZE) ? BUFFER_SIZE : s;

        for(size_t i = 0; i < mixer.getChannelNumber(); ++i)
        {
            memcpy(channelBuffer[i], inS, nb * sizeof(float32_t));
            mixer.preProcessInputSampleBuffer(i, nb);
        }
        mixer.process(outS[StereoChannels::Left ], outS[StereoChannels::Right]);

        inS += nb;
        outS[StereoChannels::Left ] += nb;
        outS[StereoChannels::Right] += nb;
        s -= nb;
    }
    
    std::string filename = "";
    for(size_t i = 0; i < (MixerOutput::kFXCount - 1); ++i)
    {
        int k = 1 << i;
        if((scenarioId & k) == 0)
        {
            continue;
        }

        if(filename.size() > 0)
        {
            filename += ", ";
        }
        filename += toString(static_cast<MixerOutput>(i));
    }

    saveWaveFile(getResultFile(full_test_name + " mixing-console[ " + filename  + " ].wav", true), outSamples[0], outSamples[1], size, static_cast<unsigned>(SAMPLING_FREQUENCY), 16);

    CLEANUP_AUDIO_TEST(inSamples, outSamples);
}

INSTANTIATE_TEST_SUITE_P(MixingConsole, FXScenarioTest, testing::Range(0, 1 << (MixerOutput::kFXCount - 1)));
