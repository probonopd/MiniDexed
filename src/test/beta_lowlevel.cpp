#include <gtest/gtest.h>

#include <iostream>
#include <iomanip>

#include "test_fx_helper.h"
#include "../fx_engine.hpp"

#define PRINT_EXEC(ctx, x) \
    std::cout.fill(' '); \
    std::cout.width(80); \
    std::cout << std::left; \
    std::cout.precision(6); \
    std::cout << std::fixed; \
    std::cout << #x; \
    x \
    { \
        float32_t v = 0.0f; \
        ctx.write(v); \
        std::cout << " // accumulator_: " << showpos << v; \
    } \
    std::cout << std::endl

#define TAIL , -1

typedef FxEngine<16384, Format::FORMAT_FLOAT32, true> Engine;

void processDebugShimmerSample(
    Engine& engine_, size_t index, 
    float32_t& lp_decay_1_, float32_t& lp_decay_2_, 
    float32_t inL, float32_t inR, 
    float32_t& outL, float32_t& outR)
{
    // This is the Griesinger topology described in the Dattorro paper
    // (4 AP diffusers on the input, then a loop of 2x 2AP+1Delay).
    // Modulation is applied in the loop of the first diffuser AP for additional
    // smearing; and to the two long delays for a slow shimmer/chorus effect.
    typedef Engine::Reserve< 113,
            Engine::Reserve< 162,
            Engine::Reserve< 241,
            Engine::Reserve< 399,
            Engine::Reserve<1653,
            Engine::Reserve<2038,
            Engine::Reserve<3411,
            Engine::Reserve<1913,
            Engine::Reserve<1663,
            Engine::Reserve<4782> > > > > > > > > > Memory;
    Engine::DelayLine<Memory, 0> ap1;
    Engine::DelayLine<Memory, 1> ap2;
    Engine::DelayLine<Memory, 2> ap3;
    Engine::DelayLine<Memory, 3> ap4;
    Engine::DelayLine<Memory, 4> dap1a;
    Engine::DelayLine<Memory, 5> dap1b;
    Engine::DelayLine<Memory, 6> del1;
    Engine::DelayLine<Memory, 7> dap2a;
    Engine::DelayLine<Memory, 8> dap2b;
    Engine::DelayLine<Memory, 9> del2;
    Engine::Context c;

    const float32_t kap = 0.8f;
    const float32_t klp = 0.7f;
    const float32_t krt = 0.75f;
    const float32_t gain = 0.55f;

    float32_t lp_1 = lp_decay_1_;
    float32_t lp_2 = lp_decay_2_;

    float32_t wet = 0.0f;
    float32_t apout = 0.0f;
    engine_.start(&c);

    // Smear AP1 inside the loop.
    PRINT_EXEC(c, c.interpolate(ap1, 10.0f, Engine::LFOIndex::LFO_1, 60.0f, 1.0f););
    PRINT_EXEC(c, c.write(ap1, 100, 0.0f););
    PRINT_EXEC(c, c.read(inL + inR, gain););

    // Diffuse through 4 allpasses.
    PRINT_EXEC(c, c.read(ap1 TAIL, kap););
    PRINT_EXEC(c, c.writeAllPass(ap1, -kap););
    PRINT_EXEC(c, c.read(ap2 TAIL, kap););
    PRINT_EXEC(c, c.writeAllPass(ap2, -kap););
    PRINT_EXEC(c, c.read(ap3 TAIL, kap););
    PRINT_EXEC(c, c.writeAllPass(ap3, -kap););
    PRINT_EXEC(c, c.read(ap4 TAIL, kap););
    PRINT_EXEC(c, c.writeAllPass(ap4, -kap););
    PRINT_EXEC(c, c.write(apout););
      
    // Main reverb loop.
    PRINT_EXEC(c, c.load(apout););
    PRINT_EXEC(c, c.interpolate(del2, 4680.0f, Engine::LFOIndex::LFO_2, 100.0f, krt););
    PRINT_EXEC(c, c.lp(lp_1, klp););
    PRINT_EXEC(c, c.read(dap1a TAIL, -kap););
    PRINT_EXEC(c, c.writeAllPass(dap1a, kap););
    PRINT_EXEC(c, c.read(dap1b TAIL, kap););
    PRINT_EXEC(c, c.writeAllPass(dap1b, -kap););
    PRINT_EXEC(c, c.write(del1, 1.5f););
    PRINT_EXEC(c, c.write(wet, 0.0f););

    outL = wet;


    PRINT_EXEC(c, c.load(apout););
    // PRINT_EXEC(c, c.interpolate(del1, 4450.0f, Engine::LFOIndex::LFO_1, 50.0f, krt););
    PRINT_EXEC(c, c.read(del1 TAIL, krt););
    PRINT_EXEC(c, c.lp(lp_2, klp););
    PRINT_EXEC(c, c.read(dap2a TAIL, kap););
    PRINT_EXEC(c, c.writeAllPass(dap2a, -kap););
    PRINT_EXEC(c, c.read(dap2b TAIL, -kap););
    PRINT_EXEC(c, c.writeAllPass(dap2b, kap););
    PRINT_EXEC(c, c.write(del2, 1.5f););
    PRINT_EXEC(c, c.write(wet, 0.0f););

    outR = wet;


    lp_decay_1_ = lp_1;
    lp_decay_2_ = lp_2;

    std::cout << "Index # " << index << " - ( " << inL << ", " << inR << " ) ==> ( " << outL << ", " << outR << " )" << std::endl;
    std::cout << std::endl << "***********************************************************************************************************" << std::endl << std::endl;
}

TEST(LowLevel, TestDiracShimmerAlgo)
{
    const testing::TestInfo* test_info = testing::UnitTest::GetInstance()->current_test_info();
    std::string full_test_name = test_info->test_case_name();
    full_test_name += ".";
    full_test_name += test_info->name();

    Engine engine_(SAMPLING_FREQUENCY);
    engine_.setLFOFrequency(Engine::LFOIndex::LFO_1, 0.5f);
    engine_.setLFOFrequency(Engine::LFOIndex::LFO_2, 0.3f);
    engine_.reset();

    float32_t lp1 = 0.0f;
    float32_t lp2 = 0.0f;

    const size_t size = static_cast<float32_t>(SAMPLING_FREQUENCY) * 4;
    float32_t* inSamples = new float32_t[size];
    memset(inSamples, 0, size * sizeof(float32_t));
    inSamples[0] = 1.0f;

    float32_t* outSamplesL = new float32_t[size];
    float32_t* outSamplesR = new float32_t[size];
    memset(outSamplesL, 0, size * sizeof(float32_t));
    memset(outSamplesR, 0, size * sizeof(float32_t));

    for(size_t i = 0; i < size; ++i)
    {
        processDebugShimmerSample(engine_, i, lp1, lp2, inSamples[i], inSamples[i], outSamplesL[i], outSamplesR[i]);
    }

    saveWaveFile(getResultFile(full_test_name + ".wav", true), outSamplesL, outSamplesR, size, SAMPLING_FREQUENCY, 16);

    delete[] outSamplesL;
    delete[] outSamplesR;

    delete[] inSamples;
}

void processShimmerSample(
    Engine& engine_L_, Engine& engine_R_, size_t index, 
    float32_t& lp_decay_1_, float32_t& lp_decay_2_, 
    float32_t inL, float32_t inR, 
    float32_t& outL, float32_t& outR)
{
    // This is the Griesinger topology described in the Dattorro paper
    // (4 AP diffusers on the input, then a loop of 2x 2AP+1Delay).
    // Modulation is applied in the loop of the first diffuser AP for additional
    // smearing; and to the two long delays for a slow shimmer/chorus effect.
    typedef Engine::Reserve< 113,
            Engine::Reserve< 162,
            Engine::Reserve< 241,
            Engine::Reserve< 399,
            Engine::Reserve<1653,
            Engine::Reserve<2038,
            Engine::Reserve<3411,
            Engine::Reserve<1913,
            Engine::Reserve<1663,
            Engine::Reserve<4782> > > > > > > > > > Memory;
    Engine::DelayLine<Memory, 0> ap1;
    Engine::DelayLine<Memory, 1> ap2;
    Engine::DelayLine<Memory, 2> ap3;
    Engine::DelayLine<Memory, 3> ap4;
    Engine::DelayLine<Memory, 4> dap1a;
    Engine::DelayLine<Memory, 5> dap1b;
    Engine::DelayLine<Memory, 6> del1;
    Engine::DelayLine<Memory, 7> dap2a;
    Engine::DelayLine<Memory, 8> dap2b;
    Engine::DelayLine<Memory, 9> del2;
    Engine::Context cL;
    Engine::Context cR;

    const float32_t kap = 0.8f;
    const float32_t klp = 0.7f;
    const float32_t krt = 0.75f;
    const float32_t gain = 0.55f;

    float32_t lp_1 = lp_decay_1_;
    float32_t lp_2 = lp_decay_2_;

    float32_t wet = 0.0f;
    float32_t apout = 0.0f;
    engine_L_.start(&cL);
    engine_R_.start(&cR);

    // Smear AP1 inside the loop.
    cL.interpolate(ap1, 10.0f, Engine::LFOIndex::LFO_1, 60.0f, 1.0f);
    cL.write(ap1, 100, 0.0f);
    cL.read(inL, gain);

    // Diffuse through 4 allpasses.
    cL.read(ap1 TAIL, kap);
    cL.writeAllPass(ap1, -kap);
    cL.read(ap2 TAIL, kap);
    cL.writeAllPass(ap2, -kap);
    cL.read(ap3 TAIL, kap);
    cL.writeAllPass(ap3, -kap);
    cL.read(ap4 TAIL, kap);
    cL.writeAllPass(ap4, -kap);
    cL.write(apout);
      
    // Main reverb loop.
    cL.load(apout);
    cL.interpolate(del2, 4680.0f, Engine::LFOIndex::LFO_2, 100.0f, krt);
    cL.lp(lp_1, klp);
    cL.read(dap1a TAIL, -kap);
    cL.writeAllPass(dap1a, kap);
    cL.read(dap1b TAIL, kap);
    cL.writeAllPass(dap1b, -kap);
    cL.write(del1, 1.5f);
    cL.write(wet, 0.0f);

    outL = wet;


    // Smear AP1 inside the loop.
    cR.interpolate(ap1, 10.0f, Engine::LFOIndex::LFO_1, 60.0f, 1.0f);
    cR.write(ap1, 100, 0.0f);
    cR.read(inL + inR, gain);

    // Diffuse through 4 allpasses.
    cR.read(ap1 TAIL, kap);
    cR.writeAllPass(ap1, -kap);
    cR.read(ap2 TAIL, kap);
    cR.writeAllPass(ap2, -kap);
    cR.read(ap3 TAIL, kap);
    cR.writeAllPass(ap3, -kap);
    cR.read(ap4 TAIL, kap);
    cR.writeAllPass(ap4, -kap);
    cR.write(apout);
      
    // Main reverb loop.
    cR.load(apout);
    // cR.interpolate(del1, 4450.0f, Engine::LFOIndex::LFO_1, 50.0f, krt);
    cR.read(del1 TAIL, krt);
    cR.lp(lp_2, klp);
    cR.read(dap2a TAIL, kap);
    cR.writeAllPass(dap2a, -kap);
    cR.read(dap2b TAIL, -kap);
    cR.writeAllPass(dap2b, kap);
    cR.write(del2, 1.5f);
    cR.write(wet, 0.0f);

    outR = wet;


    lp_decay_1_ = lp_1;
    lp_decay_2_ = lp_2;
}

TEST(LowLevel, TestStereoShimmerAlgo)
{
    const testing::TestInfo* test_info = testing::UnitTest::GetInstance()->current_test_info();
    std::string full_test_name = test_info->test_case_name();
    full_test_name += ".";
    full_test_name += test_info->name();

    Engine engine_L_(SAMPLING_FREQUENCY);
    Engine engine_R_(SAMPLING_FREQUENCY);
    engine_L_.setLFOFrequency(Engine::LFOIndex::LFO_1, 0.5f);
    engine_L_.setLFOFrequency(Engine::LFOIndex::LFO_2, 0.3f);
    engine_L_.reset();
    engine_R_.setLFOFrequency(Engine::LFOIndex::LFO_1, 0.5f);
    engine_R_.setLFOFrequency(Engine::LFOIndex::LFO_2, 0.3f);
    engine_R_.reset();

    float32_t lp1 = 0.0f;
    float32_t lp2 = 0.0f;

    size_t size = 0;
    float32_t** inSamples = readWaveFile(AUDIO_SOURCE_FILE, size);

    float32_t* outSamplesL = new float32_t[size];
    float32_t* outSamplesR = new float32_t[size];
    memset(outSamplesL, 0, size * sizeof(float32_t));
    memset(outSamplesR, 0, size * sizeof(float32_t));

    for(size_t i = 0; i < size; ++i)
    {
        processShimmerSample(engine_L_, engine_R_, i, lp1, lp2, inSamples[0][i], inSamples[1][i], outSamplesL[i], outSamplesR[i]);
    }

    saveWaveFile(getResultFile(full_test_name + ".wav", true), outSamplesL, outSamplesR, size, SAMPLING_FREQUENCY, 16);

    delete[] outSamplesL;
    delete[] outSamplesR;

    delete[] inSamples[0];
    delete[] inSamples[1];
    delete[] inSamples;
}

void processShimmerSample(
    Engine& engine_, size_t index, 
    float32_t& lp_decay_1_, float32_t& lp_decay_2_, 
    float32_t inL, float32_t inR, 
    float32_t& outL, float32_t& outR)
{
    // This is the Griesinger topology described in the Dattorro paper
    // (4 AP diffusers on the input, then a loop of 2x 2AP+1Delay).
    // Modulation is applied in the loop of the first diffuser AP for additional
    // smearing; and to the two long delays for a slow shimmer/chorus effect.
    typedef Engine::Reserve< 113,
            Engine::Reserve< 162,
            Engine::Reserve< 241,
            Engine::Reserve< 399,
            Engine::Reserve<1653,
            Engine::Reserve<2038,
            Engine::Reserve<3411,
            Engine::Reserve<1913,
            Engine::Reserve<1663,
            Engine::Reserve<4782> > > > > > > > > > Memory;
    Engine::DelayLine<Memory, 0> ap1;
    Engine::DelayLine<Memory, 1> ap2;
    Engine::DelayLine<Memory, 2> ap3;
    Engine::DelayLine<Memory, 3> ap4;
    Engine::DelayLine<Memory, 4> dap1a;
    Engine::DelayLine<Memory, 5> dap1b;
    Engine::DelayLine<Memory, 6> del1;
    Engine::DelayLine<Memory, 7> dap2a;
    Engine::DelayLine<Memory, 8> dap2b;
    Engine::DelayLine<Memory, 9> del2;
    Engine::Context c;

    const float32_t kap = 0.8f;
    const float32_t klp = 0.7f;
    const float32_t krt = 0.75f;
    const float32_t gain = 0.55f;

    float32_t lp_1 = lp_decay_1_;
    float32_t lp_2 = lp_decay_2_;

    float32_t wet = 0.0f;
    float32_t apout = 0.0f;
    engine_.start(&c);

    // Smear AP1 inside the loop.
    c.interpolate(ap1, 10.0f, Engine::LFOIndex::LFO_1, 60.0f, 1.0f);
    c.write(ap1, 100, 0.0f);
    c.read(inL + inR, gain);

    // Diffuse through 4 allpasses.
    c.read(ap1 TAIL, kap);
    c.writeAllPass(ap1, -kap);
    c.read(ap2 TAIL, kap);
    c.writeAllPass(ap2, -kap);
    c.read(ap3 TAIL, kap);
    c.writeAllPass(ap3, -kap);
    c.read(ap4 TAIL, kap);
    c.writeAllPass(ap4, -kap);
    c.write(apout);
      
    // Main reverb loop.
    c.load(apout);
    c.interpolate(del2, 4680.0f, Engine::LFOIndex::LFO_2, 100.0f, krt);
    c.lp(lp_1, klp);
    c.read(dap1a TAIL, -kap);
    c.writeAllPass(dap1a, kap);
    c.read(dap1b TAIL, kap);
    c.writeAllPass(dap1b, -kap);
    c.write(del1, 1.5f);
    c.write(wet, 0.0f);

    outL = wet;


    c.load(apout);
    c.read(del1 TAIL, krt);
    c.lp(lp_2, klp);
    c.read(dap2a TAIL, kap);
    c.writeAllPass(dap2a, -kap);
    c.read(dap2b TAIL, -kap);
    c.writeAllPass(dap2b, kap);
    c.write(del2, 1.5f);
    c.write(wet, 0.0f);

    outR = wet;


    lp_decay_1_ = lp_1;
    lp_decay_2_ = lp_2;
}

TEST(LowLevel, TestMonoShimmerAlgo)
{
    const testing::TestInfo* test_info = testing::UnitTest::GetInstance()->current_test_info();
    std::string full_test_name = test_info->test_case_name();
    full_test_name += ".";
    full_test_name += test_info->name();

    Engine engine_(SAMPLING_FREQUENCY);
    engine_.setLFOFrequency(Engine::LFOIndex::LFO_1, 0.5f);
    engine_.setLFOFrequency(Engine::LFOIndex::LFO_2, 0.3f);
    engine_.reset();

    float32_t lp1 = 0.0f;
    float32_t lp2 = 0.0f;

    size_t size = 0;
    float32_t** inSamples = readWaveFile(AUDIO_SOURCE_FILE, size);

    float32_t* outSamplesL = new float32_t[size];
    float32_t* outSamplesR = new float32_t[size];
    memset(outSamplesL, 0, size * sizeof(float32_t));
    memset(outSamplesR, 0, size * sizeof(float32_t));

    for(size_t i = 0; i < size; ++i)
    {
        processShimmerSample(engine_, i, lp1, lp2, inSamples[0][i], inSamples[1][i], outSamplesL[i], outSamplesR[i]);
    }

    saveWaveFile(getResultFile(full_test_name + ".wav", true), outSamplesL, outSamplesR, size, SAMPLING_FREQUENCY, 16);

    delete[] outSamplesL;
    delete[] outSamplesR;

    delete[] inSamples[0];
    delete[] inSamples[1];
    delete[] inSamples;
}
