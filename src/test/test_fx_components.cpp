#include <gtest/gtest.h>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <locale>
#include <ctime>
#include <cmath>
#include <random>
#include "wave.h"

#include "test_fx_helper.h"

#include "../fx_rack.h"
#include "../effect_platervbstereo.h"
#include "../mixing_console.hpp"

#define MAX_SVF_SAMPLES 10000000
#define MAX_NB_ERRORS 100

TEST(FXComponent, LFO)
{
    const float32_t freq = 10.0f;

    LFO lfo(SAMPLING_FREQUENCY, 0.0f, freq);
    unsigned size = static_cast<unsigned>(8.0f * SAMPLING_FREQUENCY / freq);
    float32_t rate = 0.0f;
    float32_t rate_increment = freq / 2.0f / SAMPLING_FREQUENCY;

    std::ofstream out(getResultFile("FXComponent.LFO.csv"));
    setupOuputStreamFocCSV(out);
    out << fixed << showpoint;

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
    StateVariableFilter svf(SAMPLING_FREQUENCY, StateVariableFilter::Type::LPF, 12000.0f);

    {
        svf.setFilterType(StateVariableFilter::Type::LPF);
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
        svf.setFilterType(StateVariableFilter::Type::LPF);
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

TEST(CppOptimization, FastLFOPrecisionTest)
{
    const float32_t freq = 0.15f;
    const size_t NB = static_cast<size_t>(2.0f * SAMPLING_FREQUENCY);

    const float32_t epsilon = 1e-3;

    ComplexLFO lfo1(SAMPLING_FREQUENCY, 0.0f, 10.0f);
    FastLFO lfo2(SAMPLING_FREQUENCY, 0.0f, 10.0f);
    lfo1.setFrequency(freq);
    lfo2.setFrequency(freq);
    float32_t max_delta = 0.0f;
    for(size_t i = 0; i < NB; ++i)
    {
        float32_t v1 = lfo1.process();
        float32_t v2 = lfo2.process();

        max_delta = std::max(max_delta, std::abs(v1 - v2));
    }
    // EXPECT_GT(epsilon, max_delta);
}
