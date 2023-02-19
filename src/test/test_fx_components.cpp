#include <gtest/gtest.h>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>

#include "test_fx_helper.h"

#include "../fx_rack.h"
#include "../effect_platervbstereo.h"

#define MAX_SVF_SAMPLES 10000000
#define MAX_NB_ERRORS 100

TEST(FXComponent, LFO)
{
    const testing::TestInfo* test_info = testing::UnitTest::GetInstance()->current_test_info();
    std::string full_test_name = test_info->test_case_name();
    full_test_name += ".";
    full_test_name += test_info->name();
    
    const float32_t freq = 10.0f;

    LFO lfo(SAMPLING_FREQUENCY, 0.0f, freq);
    unsigned size = static_cast<unsigned>(8.0f * SAMPLING_FREQUENCY / freq);
    float32_t rate = 0.0f;
    float32_t rate_increment = freq / 2.0f / SAMPLING_FREQUENCY;

    std::ofstream out(getResultFile(full_test_name + ".FXComponent.LFO.csv", true));
    setupOuputStreamForCSV(out);
    out << std::fixed << std::showpoint;

    out << "index;LFO" << std::endl;
    for(unsigned i = 0; i < size; ++i)
    {
        lfo.setNormalizedFrequency(rate);
        out << i << ";" << lfo.process() << std::endl;
        rate += rate_increment;

        if(rate >= 1.0f || rate <= 0.0f)
        {
            rate_increment *= -1.0f;
        }
    }
}

TEST(FXComponent, Flutter)
{
    JitterGenerator jg(SAMPLING_FREQUENCY);
    jg.setSpeed(1.0f);
    jg.setMagnitude(0.1f);

    for (unsigned i = 0; i < 1000; ++i)
    {
        jg.process();
    }
}

TEST(FXComponent, SVF)
{
    float32_t inL, inR;
    float32_t outL, outR;
    StateVariableFilter svf(SAMPLING_FREQUENCY, StateVariableFilter::FilterMode::LPF, 12000.0f);

    {
        svf.setFilterMode(StateVariableFilter::FilterMode::LPF);
        svf.setCutoff(12000.0f);
        svf.setResonance(0.0f);
        unsigned nbSamples = 0;
        unsigned nbErrors = 0;
        while(nbErrors < MAX_NB_ERRORS && nbSamples < MAX_SVF_SAMPLES)
        {
            nbSamples++;
            inL = getRandomValue();
            inR = getRandomValue();
            svf.processSample(inL, inR, outL, outR);

            if(std::abs(outL) > 1.0f)
                nbErrors++;
            if(std::abs(outR) > 1.0f)
                nbErrors++;
        }
        EXPECT_LT(nbErrors, MAX_NB_ERRORS);
    }

    {
        svf.setFilterMode(StateVariableFilter::FilterMode::LPF);
        svf.setCutoff(60.0f);
        svf.setResonance(0.0f);
        unsigned nbSamples = 0;
        unsigned nbErrors = 0;
        while(nbErrors < MAX_NB_ERRORS && nbSamples < MAX_SVF_SAMPLES)
        {
            nbSamples++;
            inL = getRandomValue();
            inR = getRandomValue();
            svf.processSample(inL, inR, outL, outR);

            if(std::abs(outL) > 1.0f)
                nbErrors++;
            if(std::abs(outR) > 1.0f)
                nbErrors++;
        }
        EXPECT_LT(nbErrors, MAX_NB_ERRORS);
    }
}

TEST(CppOptimization, InterpolatedSineOscillatorPrecisionTest)
{
    const float32_t freq = 0.15f;
    const size_t NB = static_cast<size_t>(2.0f * SAMPLING_FREQUENCY);

    const float32_t epsilon = 1e-3;

    ComplexLFO lfo1(SAMPLING_FREQUENCY, 0.0f, 10.0f);
    InterpolatedSineOscillator lfo2(SAMPLING_FREQUENCY, 0.0f, 10.0f);
    lfo1.setFrequency(freq);
    lfo2.setFrequency(freq);
    float32_t max_delta = 0.0f;
    for(size_t i = 0; i < NB; ++i)
    {
        float32_t v1 = lfo1.process();
        float32_t v2 = lfo2.process();

        max_delta = std::max(max_delta, std::abs(v1 - v2));
    }
    EXPECT_GT(epsilon, max_delta);
}

void testFastLFOPrecision(float32_t freq, float32_t init_phase)
{
    const size_t NB = static_cast<size_t>(2.0f * SAMPLING_FREQUENCY);
    const float32_t init_phase_deg = init_phase * 180.0f / PI;

    const float32_t epsilon = 40e-3;

    ComplexLFO  lfo1(SAMPLING_FREQUENCY, 0.0f, 220.0f, init_phase, true);
    FastLFO     lfo2(SAMPLING_FREQUENCY, 0.0f, 220.0f, init_phase, true);
    lfo1.setFrequency(freq);
    lfo2.setFrequency(freq);

    std::string file1 = string("testFastLFOPrecision.ComplexLFO.") + std::to_string(freq) + "Hz-" + std::to_string(init_phase_deg) + ".data";
    std::string file2 = string("testFastLFOPrecision.FastLFO.") + std::to_string(freq) + "Hz-" + std::to_string(init_phase_deg) + ".data";
    std::string file3 = string("testFastLFOPrecision.") + std::to_string(freq) + "Hz-" + std::to_string(init_phase_deg) + ".data.m";

    std::ofstream _lfo1(getResultFile(file1, true));
    std::ofstream _lfo2(getResultFile(file2, true));
    std::ofstream _m(getResultFile(file3, true));

    float32_t max_delta = 0.0f;
    for(size_t i = 0; i < NB; ++i)
    {
        float32_t v1 = lfo1.process();
        float32_t v2 = lfo2.process();

        _lfo1 << std::setprecision(6) << std::fixed << v1 << ((i == (NB - 1)) ? "" : ", ");
        _lfo2 << std::setprecision(6) << std::fixed << v2 << ((i == (NB - 1)) ? "" : ", ");

        max_delta = std::max(max_delta, std::abs(v1 - v2));
    }

    _lfo1.close();
    _lfo2.close();

    _m << "# Tests of FastLFO precision:" << std::endl;
    _m << std::setprecision(2) << std::fixed << "# + frequency: " << freq << " Hz" << std::endl;
    _m << std::setprecision(2) << std::fixed << "# + initial phase: " << init_phase << std::endl;
    _m << std::endl;
    _m << "time = 0 : " << (NB - 1) << ";" << std::endl;
    _m << "cplx_lfo = load(\"-ascii\", \"" << file1 << "\");" << std::endl;
    _m << "fast_lfo = load(\"-ascii\", \"" << file2 << "\");" << std::endl;
    _m << "plot(time, cplx_lfo, '-b', 'LineWidth', 6, time, fast_lfo, '-r', 'LineWidth', 4);" << std::endl;
    _m << "title('LFO tuning @ " << freq << "Hz / " << init_phase_deg << "°');" << std::endl;
    _m << "legend('ComplexLFO', 'FastLFO');" << std::endl;
    _m.close();

    EXPECT_GT(epsilon, max_delta);
}

TEST(CppOptimization, FastLFOPrecisionTest0_01Hz)
{
    const float32_t freq = 0.01f;
    testFastLFOPrecision(freq, 0.0f);
    testFastLFOPrecision(freq, PI / 6.0f);
    testFastLFOPrecision(freq, PI / 3.0f);
    testFastLFOPrecision(freq, PI / 2.0f);
    testFastLFOPrecision(freq, 2.0f * PI / 3.0f);
    testFastLFOPrecision(freq, 3.0f * PI / 4.0f);
    testFastLFOPrecision(freq, 3.0f * PI / 2.0f);
}

TEST(CppOptimization, FastLFOPrecisionTest0_15Hz)
{
    const float32_t freq = 0.15f;
    testFastLFOPrecision(freq, 0.0f);
    testFastLFOPrecision(freq, PI / 6.0f);
    testFastLFOPrecision(freq, PI / 3.0f);
    testFastLFOPrecision(freq, PI / 2.0f);
    testFastLFOPrecision(freq, 2.0f * PI / 3.0f);
    testFastLFOPrecision(freq, 3.0f * PI / 4.0f);
    testFastLFOPrecision(freq, 3.0f * PI / 2.0f);
}

TEST(CppOptimization, FastLFOPrecisionTest0_2Hz)
{
    const float32_t freq = 0.2f;
    testFastLFOPrecision(freq, 0.0f);
    testFastLFOPrecision(freq, PI / 6.0f);
    testFastLFOPrecision(freq, PI / 3.0f);
    testFastLFOPrecision(freq, PI / 2.0f);
    testFastLFOPrecision(freq, 2.0f * PI / 3.0f);
    testFastLFOPrecision(freq, 3.0f * PI / 4.0f);
    testFastLFOPrecision(freq, 3.0f * PI / 2.0f);
}

TEST(CppOptimization, FastLFOPrecisionTest0_3Hz)
{
    const float32_t freq = 0.3f;
    testFastLFOPrecision(freq, 0.0f);
    testFastLFOPrecision(freq, PI / 6.0f);
    testFastLFOPrecision(freq, PI / 3.0f);
    testFastLFOPrecision(freq, PI / 2.0f);
    testFastLFOPrecision(freq, 2.0f * PI / 3.0f);
    testFastLFOPrecision(freq, 3.0f * PI / 4.0f);
    testFastLFOPrecision(freq, 3.0f * PI / 2.0f);
}

TEST(CppOptimization, FastLFOPrecisionTest0_5Hz)
{
    const float32_t freq = 0.5f;
    testFastLFOPrecision(freq, 0.0f);
    testFastLFOPrecision(freq, PI / 6.0f);
    testFastLFOPrecision(freq, PI / 3.0f);
    testFastLFOPrecision(freq, PI / 2.0f);
    testFastLFOPrecision(freq, 2.0f * PI / 3.0f);
    testFastLFOPrecision(freq, 3.0f * PI / 4.0f);
    testFastLFOPrecision(freq, 3.0f * PI / 2.0f);
}

TEST(CppOptimization, FastLFOPrecisionTest1Hz)
{
    const float32_t freq = 1.0f;
    testFastLFOPrecision(freq, 0.0f);
    testFastLFOPrecision(freq, PI / 6.0f);
    testFastLFOPrecision(freq, PI / 3.0f);
    testFastLFOPrecision(freq, PI / 2.0f);
    testFastLFOPrecision(freq, 2.0f * PI / 3.0f);
    testFastLFOPrecision(freq, 3.0f * PI / 4.0f);
    testFastLFOPrecision(freq, 3.0f * PI / 2.0f);
}

TEST(CppOptimization, FastLFOPrecisionTest2_15Hz)
{
    const float32_t freq = 2.15f;
    testFastLFOPrecision(freq, 0.0f);
    testFastLFOPrecision(freq, PI / 6.0f);
    testFastLFOPrecision(freq, PI / 3.0f);
    testFastLFOPrecision(freq, PI / 2.0f);
    testFastLFOPrecision(freq, 2.0f * PI / 3.0f);
    testFastLFOPrecision(freq, 3.0f * PI / 4.0f);
    testFastLFOPrecision(freq, 3.0f * PI / 2.0f);
}

TEST(CppOptimization, FastLFOPrecisionTest5Hz)
{
    const float32_t freq = 5.0f;
    testFastLFOPrecision(freq, 0.0f);
    testFastLFOPrecision(freq, PI / 6.0f);
    testFastLFOPrecision(freq, PI / 3.0f);
    testFastLFOPrecision(freq, PI / 2.0f);
    testFastLFOPrecision(freq, 2.0f * PI / 3.0f);
    testFastLFOPrecision(freq, 3.0f * PI / 4.0f);
    testFastLFOPrecision(freq, 3.0f * PI / 2.0f);
}

TEST(CppOptimization, FastLFOPrecisionTest10_5Hz)
{
    const float32_t freq = 10.5f;
    testFastLFOPrecision(freq, 0.0f);
    testFastLFOPrecision(freq, PI / 6.0f);
    testFastLFOPrecision(freq, PI / 3.0f);
    testFastLFOPrecision(freq, PI / 2.0f);
    testFastLFOPrecision(freq, 2.0f * PI / 3.0f);
    testFastLFOPrecision(freq, 3.0f * PI / 4.0f);
    testFastLFOPrecision(freq, 3.0f * PI / 2.0f);
}

TEST(CppOptimization, FastLFOPrecisionTest120_5Hz)
{
    const float32_t freq = 120.5f;
    testFastLFOPrecision(freq, 0.0f);
    testFastLFOPrecision(freq, PI / 6.0f);
    testFastLFOPrecision(freq, PI / 3.0f);
    testFastLFOPrecision(freq, PI / 2.0f);
    testFastLFOPrecision(freq, 2.0f * PI / 3.0f);
    testFastLFOPrecision(freq, 3.0f * PI / 4.0f);
    testFastLFOPrecision(freq, 3.0f * PI / 2.0f);
}
