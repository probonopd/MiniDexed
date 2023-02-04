#include <gtest/gtest.h>

#include <fstream>

#include "test_fx_helper.h"
#include "../fx_components.h"

TEST(CppPerformance, LFOPerformance_ComplexLFO_InterpolatedSineOscillator)
{
    const size_t NB = 10000000;
    float32_t freq = 0.1f;

    ComplexLFO lfo1(SAMPLING_FREQUENCY, 0.0f, 10.0f);
    InterpolatedSineOscillator lfo2(SAMPLING_FREQUENCY, 0.0f, 10.0f);

    lfo1.setFrequency(freq);
    LAP_TIME("lfo1");
    for(size_t i = 0; i < NB; ++i)
    {
        lfo1.process();
    }
    auto d1 = LAP_TIME("lfo1");

    lfo2.setFrequency(freq);
    LAP_TIME("lfo2");
    for(size_t i = 0; i < NB; ++i)
    {
        lfo2.process();
    }
    auto d2 = LAP_TIME("lfo2");

    EXPECT_GE(d1, d2);
}

TEST(CppPerformance, LFOPerformance_ComplexLFO_FastLFO)
{
    const size_t NB = 10000000;
    float32_t freq = 0.1f;

    ComplexLFO lfo1(SAMPLING_FREQUENCY, 0.0f, 10.0f, Constants::MPI_2);
    FastLFO lfo2(SAMPLING_FREQUENCY, 0.0f, 10.0f);

    lfo1.setFrequency(freq);
    LAP_TIME("lfo1");
    for(size_t i = 0; i < NB; ++i)
    {
        lfo1.process();
    }
    auto d1 = LAP_TIME("lfo1");

    lfo2.setFrequency(freq);
    LAP_TIME("lfo2");
    for(size_t i = 0; i < NB; ++i)
    {
        lfo2.process();
    }
    auto d2 = LAP_TIME("lfo2");

    EXPECT_GE(d1, d2);
}

TEST(CppPerformance, FastLFOTuning)
{
    const testing::TestInfo* test_info = testing::UnitTest::GetInstance()->current_test_info();
    std::string full_test_name = test_info->test_case_name();
    full_test_name += ".";
    full_test_name += test_info->name();
    
    size_t NB = static_cast<size_t>(1.0f * SAMPLING_FREQUENCY);
    float32_t freq = 5.0f;

    FastLFO lfo1(SAMPLING_FREQUENCY, freq, 440.0f);
    lfo1.setFrequency(freq);

    ComplexLFO lfo2(SAMPLING_FREQUENCY, freq, 440.0f);
    lfo2.setFrequency(freq);

    std::ofstream out(getResultFile(full_test_name + ".FastLFOTuning-data.csv", true));
    setupOuputStreamForCSV(out);
    out << "index;FastLFO;ComplexLFO" << std::endl;
    for(size_t i = 0; i < NB; ++i)
    {
        out 
            << i << ";" 
            << lfo1.process() << ";" 
            << lfo2.process() << std::endl;
    }
    out.close();
}
