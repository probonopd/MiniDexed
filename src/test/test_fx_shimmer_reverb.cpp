#include <gtest/gtest.h>

#include "test_fx_helper.h"
#include "../fx_reverberator.h"

TEST(FXReverberator, TransientSilence)
{
    const testing::TestInfo* test_info = testing::UnitTest::GetInstance()->current_test_info();
    std::string full_test_name = test_info->test_case_name();
    full_test_name += ".";
    full_test_name += test_info->name();

    const size_t size = static_cast<size_t>(SAMPLING_FREQUENCY);
    float32_t* inSamples = new float32_t[size];
    memset(inSamples, 0, size * sizeof(float32_t));

    float32_t* outSamplesL = new float32_t[size];
    float32_t* outSamplesR = new float32_t[size];
    memset(outSamplesL, 0, size * sizeof(float32_t));
    memset(outSamplesR, 0, size * sizeof(float32_t));

    Reverberator* shimmer = new Reverberator(SAMPLING_FREQUENCY);

    shimmer->setInputGain(0.55f);
    shimmer->setTime(0.75f);
    shimmer->setDiffusion(0.8f);
    shimmer->setLP(0.7f);

    shimmer->reset();
    for(size_t i = 0; i < size; ++i)
    {
        shimmer->processSample(
            inSamples[i], 
            inSamples[i], 
            outSamplesL[i], 
            outSamplesR[i]
        );
    }

    saveWaveFile(getResultFile(full_test_name + ".wav", true), outSamplesL, outSamplesR, size, SAMPLING_FREQUENCY, 16);

    delete shimmer;

    delete[] inSamples;

    delete[] outSamplesL;
    delete[] outSamplesR;
}

TEST(FXReverberator, TransientSilenceWithDirac)
{
    const testing::TestInfo* test_info = testing::UnitTest::GetInstance()->current_test_info();
    std::string full_test_name = test_info->test_case_name();
    full_test_name += ".";
    full_test_name += test_info->name();

    const size_t size = 4 * static_cast<size_t>(SAMPLING_FREQUENCY);
    float32_t* inSamples = new float32_t[size];
    memset(inSamples, 0, size * sizeof(float32_t));
    inSamples[0] = 1.0f;

    float32_t* outSamplesL = new float32_t[size];
    float32_t* outSamplesR = new float32_t[size];
    memset(outSamplesL, 0, size * sizeof(float32_t));
    memset(outSamplesR, 0, size * sizeof(float32_t));

    Reverberator* shimmer = new Reverberator(SAMPLING_FREQUENCY);

    shimmer->setInputGain(0.55f);
    shimmer->setTime(0.75f);
    shimmer->setDiffusion(0.8f);
    shimmer->setLP(0.7f);

    shimmer->reset();
    for(size_t i = 0; i < size; ++i)
    {
        shimmer->processSample(
            inSamples[i], 
            inSamples[i], 
            outSamplesL[i], 
            outSamplesR[i]
        );
    }

    saveWaveFile(getResultFile(full_test_name + ".wav", true), outSamplesL, outSamplesR, size, SAMPLING_FREQUENCY, 16);

    delete shimmer;

    delete[] inSamples;

    delete[] outSamplesL;
    delete[] outSamplesR;
}

TEST(FXReverberator, TransientNoise)
{
    const testing::TestInfo* test_info = testing::UnitTest::GetInstance()->current_test_info();
    std::string full_test_name = test_info->test_case_name();
    full_test_name += ".";
    full_test_name += test_info->name();

    const size_t size = static_cast<size_t>(SAMPLING_FREQUENCY);
    float32_t* inSamples = new float32_t[size];
    for(size_t i = 0; i < size; ++i) inSamples[i] = getRandomValue();

    float32_t* outSamplesL = new float32_t[size];
    float32_t* outSamplesR = new float32_t[size];
    memset(outSamplesL, 0, size * sizeof(float32_t));
    memset(outSamplesR, 0, size * sizeof(float32_t));

    Reverberator* shimmer = new Reverberator(SAMPLING_FREQUENCY);

    shimmer->setInputGain(0.55f);
    shimmer->setTime(0.75f);
    shimmer->setDiffusion(0.8f);
    shimmer->setLP(0.7f);

    shimmer->reset();
    for(size_t i = 0; i < size; ++i)
    {
        shimmer->processSample(
            inSamples[i], 
            inSamples[i], 
            outSamplesL[i], 
            outSamplesR[i]
        );
    }

    saveWaveFile(getResultFile(full_test_name + ".wav", true), outSamplesL, outSamplesR, size, SAMPLING_FREQUENCY, 16);

    delete shimmer;

    delete[] inSamples;

    delete[] outSamplesL;
    delete[] outSamplesR;
}

TEST(FXReverberator, TransientMusic)
{
    const testing::TestInfo* test_info = testing::UnitTest::GetInstance()->current_test_info();
    std::string full_test_name = test_info->test_case_name();
    full_test_name += ".";
    full_test_name += test_info->name();

    size_t size;
    float32_t** inSamples = readWaveFile(AUDIO_SOURCE_FILE, size);

    float32_t* outSamplesL = new float32_t[size];
    float32_t* outSamplesR = new float32_t[size];
    memset(outSamplesL, 0, size * sizeof(float32_t));
    memset(outSamplesR, 0, size * sizeof(float32_t));

    Reverberator* shimmer = new Reverberator(SAMPLING_FREQUENCY);

    shimmer->setInputGain(0.55f);
    shimmer->setTime(0.75f);
    shimmer->setDiffusion(0.8f);
    shimmer->setLP(0.7f);

    shimmer->reset();
    for(size_t i = 0; i < size; ++i)
    {
        shimmer->processSample(
            inSamples[0][i], 
            inSamples[1][i], 
            outSamplesL[i], 
            outSamplesR[i]
        );
    }

    saveWaveFile(getResultFile(full_test_name + ".wav", true), outSamplesL, outSamplesR, size, SAMPLING_FREQUENCY, 16);

    delete shimmer;

    delete[] inSamples[0];
    delete[] inSamples[1];
    delete[] inSamples;

    delete[] outSamplesL;
    delete[] outSamplesR;
}
