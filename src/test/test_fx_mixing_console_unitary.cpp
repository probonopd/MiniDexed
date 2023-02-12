#include "test_fx_helper.h"

#include "../mixing_console.hpp"

typedef MixingConsole<1> Mixer;

TEST(MixingConsole, ShortBuffer)
{
    static const float32_t SINPI_4 = std::sqrt(2.0f) / 2.0f;

    const testing::TestInfo* test_info = testing::UnitTest::GetInstance()->current_test_info();
    std::string full_test_name = test_info->test_case_name();
    full_test_name += ".";
    full_test_name += test_info->name();

    const size_t size = 10;
    Mixer* mixer = new Mixer(SAMPLING_FREQUENCY, size);

    mixer->setChannelLevel(0, 1.0f);
    mixer->setPan(0, 0.5f);

    mixer->setSendLevel(0, MixerOutput::MainOutput, 1.0f);

    float32_t inSamples[size];
    for(size_t s = 0; s < size; ++s) inSamples[s] = getRandomValue();
    float32_t outSamples[2][size];
    memset(outSamples[0], 0, size * sizeof(float32_t));
    memset(outSamples[1], 0, size * sizeof(float32_t));

    mixer->setInputSampleBuffer(0, inSamples);
    ASSERT_EQ(0, FULL_INSPECT(mixer, true)) << full_test_name << " Mixer.setInputSampleBuffer";

    mixer->process(outSamples[0], outSamples[1]);
    ASSERT_EQ(0, FULL_INSPECT(mixer, true)) << full_test_name << " Mixer.process";
    for(size_t s = 0; s < size; ++s)
    {
        EXPECT_FLOAT_EQ(outSamples[0][s], outSamples[1][s]);
        EXPECT_FLOAT_EQ(outSamples[0][s], inSamples[s] * SINPI_4);
    }

    delete mixer;
}

TEST(MixingConsole, ReverberatorShortBuffer)
{
    const testing::TestInfo* test_info = testing::UnitTest::GetInstance()->current_test_info();
    std::string full_test_name = test_info->test_case_name();
    full_test_name += ".";
    full_test_name += test_info->name();

    const size_t size = 10;
    Mixer* mixer = new Mixer(SAMPLING_FREQUENCY, size);

    mixer->setChannelLevel(0, 1.0f);
    mixer->setPan(0, 0.5f);

    mixer->getReverberator()->setInputGain(0.35f);
    mixer->getReverberator()->setTime(0.69f);
    mixer->getReverberator()->setDiffusion(0.7f);
    mixer->getReverberator()->setLP(0.8f);

    mixer->setSendLevel(0, MixerOutput::MainOutput, 0.4f);
    mixer->setSendLevel(0, MixerOutput::FX_Reverberator, 1.0f);
    mixer->setReturnLevel(MixerOutput::FX_Reverberator, MixerOutput::MainOutput, 0.6f);

    float32_t inSamples[size];
    for(size_t s = 0; s < size; ++s) inSamples[s] = getRandomValue();
    float32_t outSamples[2][size];
    memset(outSamples[0], 0, size * sizeof(float32_t));
    memset(outSamples[1], 0, size * sizeof(float32_t));

    mixer->setInputSampleBuffer(0, inSamples);
    ASSERT_EQ(0, FULL_INSPECT(mixer, true)) << full_test_name << " Mixer.setInputSampleBuffer";

    mixer->process(outSamples[0], outSamples[1]);
    ASSERT_EQ(0, FULL_INSPECT(mixer, true)) << full_test_name << " Mixer.process";

    delete mixer;
}

TEST(MixingConsole, DrySamplesBoundariesTest)
{
    const testing::TestInfo* test_info = testing::UnitTest::GetInstance()->current_test_info();
    std::string full_test_name = test_info->test_case_name();
    full_test_name += ".";
    full_test_name += test_info->name();

    size_t size;
    float32_t** inSamples =readWaveFile(AUDIO_SOURCE_FILE, size);

    Mixer* mixer = new Mixer(SAMPLING_FREQUENCY, size);
    mixer->reset();
    FULL_INSPECT2(mixer, true, "Mixer.reset");

    mixer->setChannelLevel(0, 1.0f);
    mixer->setPan(0, 0.5f);
    mixer->setSendLevel(0, MixerOutput::MainOutput, 1.0f);

    mixer->setInputSampleBuffer(0, inSamples[0]);

    float32_t** outSamples = new float32_t*[2];
    outSamples[0] = new float32_t[size];
    outSamples[1] = new float32_t[size];
    memset(outSamples[0], 0, size * sizeof(float32_t));
    memset(outSamples[1], 0, size * sizeof(float32_t));

    mixer->process(outSamples[0], outSamples[1]);

    size_t nb_errors = 0;
    for(size_t i = 0; i < size; ++i)
    {
        nb_errors += fullInspector(full_test_name + ".outputSampleTest", inSamples[0][i], -1.0f, 1.0f, true);
        nb_errors += fullInspector(full_test_name + ".outputSampleTest", inSamples[1][i], -1.0f, 1.0f, true);
    }
    ASSERT_EQ(0, nb_errors) << full_test_name << ".outputSampleTest";

    delete[] inSamples[0];
    delete[] inSamples[1];
    delete[] inSamples;

    delete[] outSamples[0];
    delete[] outSamples[1];
    delete[] outSamples;

    delete mixer;
}

TEST(MixingConsole, ReverberatorSamplesBoundariesTest)
{
    const testing::TestInfo* test_info = testing::UnitTest::GetInstance()->current_test_info();
    std::string full_test_name = test_info->test_case_name();
    full_test_name += ".";
    full_test_name += test_info->name();

    size_t size;
    float32_t** inSamples =readWaveFile(AUDIO_SOURCE_FILE, size);

    float32_t** outSamples = new float32_t*[2];
    outSamples[0] = new float32_t[size];
    outSamples[1] = new float32_t[size];
    memset(outSamples[0], 0, size * sizeof(float32_t));
    memset(outSamples[1], 0, size * sizeof(float32_t));

    Mixer* mixer = new Mixer(SAMPLING_FREQUENCY, size);
    mixer->reset();
    mixer->setChannelLevel(0, 1.0f);
    mixer->setPan(0, 0.5f);

    mixer->setSendLevel(0, MixerOutput::MainOutput, 0.4f);
    mixer->setSendLevel(0, MixerOutput::FX_Reverberator, 1.0f);
    mixer->setReturnLevel(MixerOutput::FX_Reverberator, MixerOutput::MainOutput, 0.6f);

    mixer->getReverberator()->setMute(false);
    mixer->getReverberator()->setInputGain(0.35);
    mixer->getReverberator()->setTime(0.65);
    mixer->getReverberator()->setDiffusion(0.8);
    mixer->getReverberator()->setLP(0.7f);

    mixer->setInputSampleBuffer(0, inSamples[0]);
    mixer->process(outSamples[0], outSamples[1]);
    ASSERT_EQ(0, FULL_INSPECT2(mixer, true, full_test_name + "Mixer.process")) << full_test_name << " Mixer.process";

    saveWaveFile(getResultFile(full_test_name + ".wav", true), outSamples[0], outSamples[1], size, static_cast<unsigned>(SAMPLING_FREQUENCY), 16);

    size_t nb_errors = 0;
    for(size_t i = 0; i < size; ++i)
    {
        nb_errors += fullInspector(full_test_name + ".outputSampleTest", inSamples[0][i], -1.0f, 1.0f, true);
        nb_errors += fullInspector(full_test_name + ".outputSampleTest", inSamples[1][i], -1.0f, 1.0f, true);
    }
    ASSERT_EQ(0, nb_errors) << full_test_name << ".outputSampleTest";

    delete[] inSamples[0];
    delete[] inSamples[1];
    delete[] inSamples;

    delete[] outSamples[0];
    delete[] outSamples[1];
    delete[] outSamples;

    delete mixer;
}
