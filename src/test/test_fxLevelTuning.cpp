#include "test_fx_helper.h"

#include "../fx_tube.h"
#include "../fx_chorus.h"
#include "../fx_flanger.h"
#include "../fx_orbitone.h"
#include "../fx_phaser.h"
#include "../fx_delay.h"
#include "../effect_platervbstereo.h"
#include "../fx_reverberator.h"

TEST(LevelTuning, Tube)
{
    Tube fx(SAMPLING_FREQUENCY);
    fx.reset();
    fx.setOverdrive(0.75f);
    
    PREPARE_AUDIO_TEST(size, inSamples, outSamples, full_test_name);
    float32_t sumIn = 0.0f;
    float32_t sumOut = 0.0f;
    size_t nb_errors = 0;
    AUDIO_LOOP(inSamples, outSamples, size, inL, inR, outL, outR,
        sumIn += inL * inL;

        fx.processSample(inL, inR, outL, outR);

        sumOut += outL * outL;

        nb_errors += std::abs(outL) > 1.0f ? 1 : 0;
        nb_errors += std::abs(outR) > 1.0f ? 1 : 0;
    );

    CLEANUP_AUDIO_TEST(inSamples, outSamples);

    float32_t ratio = std::sqrt(sumOut / sumIn);

    ASSERT_EQ(nb_errors, 0) << "Sample value error for Tube";
    EXPECT_GE(ratio, 0.9f);
    EXPECT_LE(1.0f / ratio, 1.1f);
}

TEST(LevelTuning, Chorus)
{
    Chorus fx(SAMPLING_FREQUENCY);
    fx.reset();
    fx.setRate(0.4f);
    fx.setDepth(0.5f);

    PREPARE_AUDIO_TEST(size, inSamples, outSamples, full_test_name);
    float32_t sumIn = 0.0f;
    float32_t sumOut = 0.0f;
    size_t nb_errors = 0;
    AUDIO_LOOP(inSamples, outSamples, size, inL, inR, outL, outR,
        sumIn += inL * inL;

        fx.processSample(inL, inR, outL, outR);

        sumOut += outL * outL;

        nb_errors += std::abs(outL) > 1.0f ? 1 : 0;
        nb_errors += std::abs(outR) > 1.0f ? 1 : 0;
    );

    CLEANUP_AUDIO_TEST(inSamples, outSamples);

    float32_t ratio = std::sqrt(sumOut / sumIn);

    ASSERT_EQ(nb_errors, 0) << "Sample value error for Chorus";
    EXPECT_LE(ratio, 1.0f);
    EXPECT_GE(ratio, 0.9f);
    EXPECT_LE(1.0f / ratio, 1.1f);
}

TEST(LevelTuning, Flanger)
{
    Flanger fx(SAMPLING_FREQUENCY);
    fx.reset();
    fx.setRate(0.03f);
    fx.setDepth(0.75f);
    fx.setFeedback(0.5f);

    PREPARE_AUDIO_TEST(size, inSamples, outSamples, full_test_name);
    float32_t sumIn = 0.0f;
    float32_t sumOut = 0.0f;
    size_t nb_errors = 0;
    AUDIO_LOOP(inSamples, outSamples, size, inL, inR, outL, outR,
        sumIn += inL * inL;

        fx.processSample(inL, inR, outL, outR);

        sumOut += outL * outL;

        nb_errors += std::abs(outL) > 1.0f ? 1 : 0;
        nb_errors += std::abs(outR) > 1.0f ? 1 : 0;
    );

    CLEANUP_AUDIO_TEST(inSamples, outSamples);

    float32_t ratio = std::sqrt(sumOut / sumIn);

    ASSERT_EQ(nb_errors, 0) << "Sample value error for Flanger";
    EXPECT_LE(ratio, 1.0f);
    EXPECT_GE(ratio, 0.9f);
    EXPECT_LE(1.0f / ratio, 1.1f);
}

TEST(LevelTuning, Orbitone)
{
    Orbitone fx(SAMPLING_FREQUENCY);
    fx.reset();
    fx.setRate(0.4f);
    fx.setDepth(0.5f);

    PREPARE_AUDIO_TEST(size, inSamples, outSamples, full_test_name);
    float32_t sumIn = 0.0f;
    float32_t sumOut = 0.0f;
    size_t nb_errors = 0;
    AUDIO_LOOP(inSamples, outSamples, size, inL, inR, outL, outR,
        sumIn += inL * inL;

        fx.processSample(inL, inR, outL, outR);

        sumOut += outL * outL;

        nb_errors += std::abs(outL) > 1.0f ? 1 : 0;
        nb_errors += std::abs(outR) > 1.0f ? 1 : 0;
    );

    CLEANUP_AUDIO_TEST(inSamples, outSamples);

    float32_t ratio = std::sqrt(sumOut / sumIn);

    ASSERT_EQ(nb_errors, 0) << "Sample value error for Orbitone";
    EXPECT_LE(ratio, 1.0f);
    EXPECT_GE(ratio, 0.9f);
    EXPECT_LE(1.0f / ratio, 1.1f);
}

TEST(LevelTuning, Phaser)
{
    Phaser fx(SAMPLING_FREQUENCY);
    fx.reset();
    fx.setRate(0.1f);
    fx.setDepth(1.0f);
    fx.setFeedback(0.5f);
    fx.setNbStages(12);

    PREPARE_AUDIO_TEST(size, inSamples, outSamples, full_test_name);
    float32_t sumIn = 0.0f;
    float32_t sumOut = 0.0f;
    size_t nb_errors = 0;
    AUDIO_LOOP(inSamples, outSamples, size, inL, inR, outL, outR,
        sumIn += inL * inL;

        fx.processSample(inL, inR, outL, outR);

        sumOut += outL * outL;

        nb_errors += std::abs(outL) > 1.0f ? 1 : 0;
        nb_errors += std::abs(outR) > 1.0f ? 1 : 0;
    );

    CLEANUP_AUDIO_TEST(inSamples, outSamples);

    float32_t ratio = std::sqrt(sumOut / sumIn);

    ASSERT_EQ(nb_errors, 0) << "Sample value error for Phaser";
    EXPECT_LE(ratio, 1.0f);
    EXPECT_GE(ratio, 0.9f);
    EXPECT_LE(1.0f / ratio, 1.1f);
}

TEST(LevelTuning, Delay)
{
    Delay fx(SAMPLING_FREQUENCY);
    fx.reset();
    fx.setLeftDelayTime(0.15f);
    fx.setLeftDelayTime(0.2f);
    fx.setFeedback(0.35f);

    PREPARE_AUDIO_TEST(size, inSamples, outSamples, full_test_name);
    float32_t sumIn = 0.0f;
    float32_t sumOut = 0.0f;
    size_t nb_errors = 0;
    AUDIO_LOOP(inSamples, outSamples, size, inL, inR, outL, outR,
        sumIn += inL * inL;

        fx.processSample(inL, inR, outL, outR);

        sumOut += outL * outL;

        nb_errors += std::abs(outL) > 1.0f ? 1 : 0;
        nb_errors += std::abs(outR) > 1.0f ? 1 : 0;
    );

    CLEANUP_AUDIO_TEST(inSamples, outSamples);

    float32_t ratio = std::sqrt(sumOut / sumIn);

    ASSERT_EQ(nb_errors, 0) << "Sample value error for Delay";
    EXPECT_LE(ratio, 1.0f);
    EXPECT_GE(ratio, 0.9f);
    EXPECT_LE(1.0f / ratio, 1.1f);
}

TEST(LevelTuning, PlateReverb)
{
    AudioEffectPlateReverb fx(SAMPLING_FREQUENCY);
    fx.reset();
    fx.set_bypass(false);
    fx.size(0.7f);
    fx.hidamp(0.5f);
    fx.lodamp(0.5f);
    fx.lowpass(0.3f);
    fx.diffusion(0.65f);
    fx.level(1.0f);

    PREPARE_AUDIO_TEST(size, inSamples, outSamples, full_test_name);
    float32_t sumIn = 0.0f;
    float32_t sumOut = 0.0f;
    size_t nb_errors = 0;
    AUDIO_LOOP(inSamples, outSamples, size, inL, inR, outL, outR,
        sumIn += inL * inL;

        fx.processSample(inL, inR, outL, outR);

        sumOut += outL * outL;

        nb_errors += std::abs(outL) > 1.0f ? 1 : 0;
        nb_errors += std::abs(outR) > 1.0f ? 1 : 0;
    );

    CLEANUP_AUDIO_TEST(inSamples, outSamples);

    float32_t ratio = std::sqrt(sumOut / sumIn);

    ASSERT_EQ(nb_errors, 0) << "Sample value error for PlateReverb";
    EXPECT_GE(ratio, 0.9f);
    EXPECT_LE(1.0f / ratio, 1.1f);
}

TEST(LevelTuning, Reverberator)
{
    Reverberator fx(SAMPLING_FREQUENCY);
    fx.reset();
    fx.setInputGain(0.35f);
    fx.setTime(0.89f);
    fx.setDiffusion(0.75f);
    fx.setLP(0.8f);

    PREPARE_AUDIO_TEST(size, inSamples, outSamples, full_test_name);
    float32_t sumIn = 0.0f;
    float32_t sumOut = 0.0f;
    size_t nb_errors = 0;
    AUDIO_LOOP(inSamples, outSamples, size, inL, inR, outL, outR,
        sumIn += inL * inL;

        fx.processSample(inL, inR, outL, outR);

        sumOut += outL * outL;

        nb_errors += std::abs(outL) > 1.0f ? 1 : 0;
        nb_errors += std::abs(outR) > 1.0f ? 1 : 0;
    );

    CLEANUP_AUDIO_TEST(inSamples, outSamples);

    float32_t ratio = std::sqrt(sumOut / sumIn);

    ASSERT_EQ(nb_errors, 0) << "Sample value error for Reverberator";
    EXPECT_GE(ratio, 0.9f);
    EXPECT_LE(1.0f / ratio, 1.1f);
}
