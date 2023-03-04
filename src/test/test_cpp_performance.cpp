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

TEST(CppPerformance, LFOPerformance_InterpolatedSineOscillator_FastLFO)
{
    const size_t NB = 10000000;
    float32_t freq = 0.1f;

    InterpolatedSineOscillator lfo1(SAMPLING_FREQUENCY, 0.0f, 10.0f, Constants::MPI_2);
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

TEST(CppPerformance, LFOPerformance_FastLFO_FastLFO2)
{
    const size_t NB = 10000000;
    float32_t freq = 0.1f;

    FastLFO  lfo1(SAMPLING_FREQUENCY, 0.0f, 10.0f, Constants::MPI_2);
    FastLFO2 lfo2(SAMPLING_FREQUENCY, 0.0f, 10.0f);

    lfo1.setFrequency(freq);
    lfo1.reset();
    LAP_TIME("lfo1");
    for(size_t i = 0; i < NB; ++i)
    {
        lfo1.process();
    }
    auto d1 = LAP_TIME("lfo1");

    lfo2.setFrequency(freq);
    lfo2.reset();
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
    float32_t freq = 1.5f;

    FastLFO lfo1(SAMPLING_FREQUENCY, freq, 440.0f);
    lfo1.setFrequency(freq);

    InterpolatedSineOscillator lfo2(SAMPLING_FREQUENCY, freq, 440.0f);
    lfo2.setFrequency(freq);

    ComplexLFO lfo3(SAMPLING_FREQUENCY, freq, 440.0f);
    lfo3.setFrequency(freq);

    std::ofstream out(getResultFile(full_test_name + ".data.csv", true));
    setupOuputStreamForCSV(out);
    out << "index;FastLFO;InterpolatedSineOscillator;ComplexLFO" << std::endl;
    for(size_t i = 0; i < NB; ++i)
    {
        out 
            << i << ";" 
            << lfo1.process() << ";" 
            << lfo2.process() << ";"
            << lfo3.process() << std::endl;
    }
    out.close();
}


TEST(CppPerformance, FastLFO2Tuning)
{
    const testing::TestInfo* test_info = testing::UnitTest::GetInstance()->current_test_info();
    std::string full_test_name = test_info->test_case_name();
    full_test_name += ".";
    full_test_name += test_info->name();
    
    size_t NB = static_cast<size_t>(1.0f * SAMPLING_FREQUENCY);
    float32_t freq = 1.5f;

    FastLFO2 lfo1(SAMPLING_FREQUENCY, freq, 440.0f);
    lfo1.setFrequency(freq);

    InterpolatedSineOscillator lfo2(SAMPLING_FREQUENCY, freq, 440.0f);
    lfo2.setFrequency(freq);

    ComplexLFO lfo3(SAMPLING_FREQUENCY, freq, 440.0f);
    lfo3.setFrequency(freq);

    std::ofstream out(getResultFile(full_test_name + ".data.csv", true));
    setupOuputStreamForCSV(out);
    out << "index;FastLFO;InterpolatedSineOscillator;ComplexLFO" << std::endl;
    for(size_t i = 0; i < NB; ++i)
    {
        out 
            << i << ";" 
            << lfo1.process() << ";" 
            << lfo2.process() << ";"
            << lfo3.process() << std::endl;
    }
    out.close();
}

TEST(CppPerformance, FastLFOsTuningOctave)
{
    const testing::TestInfo* test_info = testing::UnitTest::GetInstance()->current_test_info();
    std::string full_test_name = test_info->test_case_name();
    full_test_name += ".";
    full_test_name += test_info->name();
    
    size_t NB = static_cast<size_t>(1.0f * SAMPLING_FREQUENCY);
    float32_t freq = 1.5f;

    FastLFO lfo1(SAMPLING_FREQUENCY, freq, 440.0f);
    lfo1.setFrequency(freq);

    FastLFO2 lfo2(SAMPLING_FREQUENCY, freq, 440.0f);
    lfo2.setFrequency(freq);

    InterpolatedSineOscillator lfo3(SAMPLING_FREQUENCY, freq, 440.0f);
    lfo3.setFrequency(freq);

    ComplexLFO lfo4(SAMPLING_FREQUENCY, freq, 440.0f);
    lfo4.setFrequency(freq);

    std::stringstream ssLFO1;
    std::stringstream ssLFO2;
    std::stringstream ssLFO3;
    std::stringstream ssLFO4;

    for(size_t i = 0; i < NB; ++i)
    {
        ssLFO1 << lfo1.process() << (i == (NB - 1) ? "" : ", ");
        ssLFO2 << lfo2.process() << (i == (NB - 1) ? "" : ", ");
        ssLFO3 << lfo3.process() << (i == (NB - 1) ? "" : ", ");
        ssLFO4 << lfo4.process() << (i == (NB - 1) ? "" : ", ");
    }

    std::ofstream _lfo1(getResultFile(std::string("data/") + full_test_name + ".fast.data", true));
    _lfo1 << ssLFO1.str();
    _lfo1.close();

    std::ofstream _lfo2(getResultFile(std::string("data/") + full_test_name + ".fst2.data", true));
    _lfo2 << ssLFO2.str();
    _lfo2.close();

    std::ofstream _lfo3(getResultFile(std::string("data/") + full_test_name + ".intr.data", true));
    _lfo3 << ssLFO3.str();
    _lfo3.close();

    std::ofstream _lfo4(getResultFile(std::string("data/") + full_test_name + ".cplx.data", true));
    _lfo4 << ssLFO4.str();
    _lfo4.close();

    std::ofstream out(getResultFile(full_test_name + ".data.m", true));
    out << "# m file to tune FastLFO component" << std::endl << std::endl;
    out << "# Parameters:" << std::endl 
        << "# + frequency: " << freq << "Hz" << std::endl 
        << "# + # samples: " << NB << std::endl << std::endl;

    out << "time = 0 : " << (NB - 1) << ";" << std::endl;
    out << "fast_lfo = load(\"-ascii\", \"data/" << full_test_name  << ".fast.data\");" << std::endl;
    out << "fst2_lfo = load(\"-ascii\", \"data/" << full_test_name  << ".fst2.data\");" << std::endl;
    out << "intr_lfo = load(\"-ascii\", \"data/" << full_test_name  << ".intr.data\");" << std::endl;
    out << "cplx_lfo = load(\"-ascii\", \"data/" << full_test_name  << ".cplx.data\");" << std::endl;

    out << std::endl << std::endl;

    out << "plot(time, fast_lfo, '-r', 'LineWidth', 6, time, fst2_lfo, '-o', 'LineWidth', 6, time, intr_lfo, '-b', 'LineWidth', 4, cplx_lfo, '-g', 'LineWidth', 4);" << std::endl;
    out << "title('LFO tuning');" << std::endl;
    out << "legend('FastLFO2', 'InterpolatedSineOscillator', 'ComplexLFO');" << std::endl;

    out.close();
}
