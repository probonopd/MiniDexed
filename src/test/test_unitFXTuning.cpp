#include "test_fx_helper.h"

#include "../fx_dry.h"
#include "../fx_tube.h"
#include "../fx_chorus.h"
#include "../fx_flanger.h"
#include "../fx_orbitone.h"
#include "../fx_phaser.h"
#include "../fx_delay.h"
#include "../effect_platervbstereo.h"
#include "../fx_shimmer_reverb.h"

TEST(UnitFXTuning, Dry)
{
    Dry fx(SAMPLING_FREQUENCY);

    PREPARE_AUDIO_TEST(size, inSamples, outSamples, full_test_name);
    SIMPLE_AUDIO_LOOP(inSamples, outSamples, size, inL, inR, outL, outR, fx);
    SAVE_AUDIO_RESULTS(full_test_name, outSamples, size);
    CLEANUP_AUDIO_TEST(inSamples, outSamples);
}

TEST(UnitFXTuning, Tube)
{
    Tube fx(SAMPLING_FREQUENCY);
    fx.setOverdrive(0.5f);

    PREPARE_AUDIO_TEST(size, inSamples, outSamples, full_test_name);
    SIMPLE_AUDIO_LOOP(inSamples, outSamples, size, inL, inR, outL, outR, fx);
    SAVE_AUDIO_RESULTS(full_test_name, outSamples, size);
    CLEANUP_AUDIO_TEST(inSamples, outSamples);
}

TEST(UnitFXTuning, Chorus)
{
    Chorus fx(SAMPLING_FREQUENCY);
    fx.setRate(0.4f);
    fx.setDepth(0.7f);

    PREPARE_AUDIO_TEST(size, inSamples, outSamples, full_test_name);
    SIMPLE_AUDIO_LOOP(inSamples, outSamples, size, inL, inR, outL, outR, fx);
    SAVE_AUDIO_RESULTS(full_test_name, outSamples, size);
    CLEANUP_AUDIO_TEST(inSamples, outSamples);
}

TEST(UnitFXTuning, Flanger)
{
    Flanger fx(SAMPLING_FREQUENCY);
    fx.setRate(0.03f);
    fx.setDepth(0.75f);
    fx.setFeedback(0.5f);

    PREPARE_AUDIO_TEST(size, inSamples, outSamples, full_test_name);
    SIMPLE_AUDIO_LOOP(inSamples, outSamples, size, inL, inR, outL, outR, fx);
    SAVE_AUDIO_RESULTS(full_test_name, outSamples, size);
    CLEANUP_AUDIO_TEST(inSamples, outSamples);
}

TEST(UnitFXTuning, Orbitone)
{
    Orbitone fx(SAMPLING_FREQUENCY);
    fx.setRate(0.4f);
    fx.setDepth(0.7f);

    PREPARE_AUDIO_TEST(size, inSamples, outSamples, full_test_name);
    SIMPLE_AUDIO_LOOP(inSamples, outSamples, size, inL, inR, outL, outR, fx);
    SAVE_AUDIO_RESULTS(full_test_name, outSamples, size);
    CLEANUP_AUDIO_TEST(inSamples, outSamples);
}

TEST(UnitFXTuning, Phaser)
{
    Phaser fx(SAMPLING_FREQUENCY);
    fx.setRate(0.1f);
    fx.setDepth(1.0f);
    fx.setFeedback(0.5f);
    fx.setNbStages(12);

    PREPARE_AUDIO_TEST(size, inSamples, outSamples, full_test_name);
    SIMPLE_AUDIO_LOOP(inSamples, outSamples, size, inL, inR, outL, outR, fx);
    SAVE_AUDIO_RESULTS(full_test_name, outSamples, size);
    CLEANUP_AUDIO_TEST(inSamples, outSamples);
}

TEST(UnitFXTuning, Delay)
{
    Delay fx(SAMPLING_FREQUENCY);
    fx.setLeftDelayTime(0.25f);
    fx.setLeftDelayTime(0.40f);
    fx.setFeedback(0.55f);
    fx.setFlutterRate(0.01f);
    fx.setFlutterAmount(0.05f);

    PREPARE_AUDIO_TEST(size, inSamples, outSamples, full_test_name);
    SIMPLE_AUDIO_LOOP(inSamples, outSamples, size, inL, inR, outL, outR, fx);
    SAVE_AUDIO_RESULTS(full_test_name, outSamples, size);
    CLEANUP_AUDIO_TEST(inSamples, outSamples);
}

TEST(UnitFXTuning, PlateReverb)
{
    AudioEffectPlateReverb fx(SAMPLING_FREQUENCY);
    fx.set_bypass(false);
    fx.size(0.7f);
    fx.hidamp(0.5f);
    fx.lodamp(0.5f);
    fx.lowpass(0.3f);
    fx.diffusion(0.65f);
    fx.level(1.0f);

    PREPARE_AUDIO_TEST(size, inSamples, outSamples, full_test_name);
    SIMPLE_AUDIO_LOOP(inSamples, outSamples, size, inL, inR, outL, outR, fx);
    SAVE_AUDIO_RESULTS(full_test_name, outSamples, size);
    CLEANUP_AUDIO_TEST(inSamples, outSamples);
}

TEST(UnitFXTuning, ShimmerReverb)
{
    ShimmerReverb fx(SAMPLING_FREQUENCY);
    fx.setInputGain(0.65f);
    fx.setTime(0.89f);
    fx.setDiffusion(0.75f);
    fx.setLP(0.8f);

    PREPARE_AUDIO_TEST(size, inSamples, outSamples, full_test_name);
    SIMPLE_AUDIO_LOOP(inSamples, outSamples, size, inL, inR, outL, outR, fx);
    SAVE_AUDIO_RESULTS(full_test_name, outSamples, size);
    CLEANUP_AUDIO_TEST(inSamples, outSamples);
}

