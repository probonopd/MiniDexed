#include <gtest/gtest.h>

#include "test_fx_helper.h"

#include "../effect_platervbstereo.h"

TEST(FXPlateReverb, Migration)
{
    AudioEffectPlateReverb reverb(SAMPLING_FREQUENCY);
    reverb.set_bypass(false);
    reverb.size(0.7f);
    reverb.hidamp(0.5f);
    reverb.lodamp(0.5f);
    reverb.lowpass(0.3f);
    reverb.diffusion(0.65f);
    reverb.level(1.0f);

    PREPARE_AUDIO_TEST(size, inSamples, outSamples, full_test_name);
    for(size_t i = 0; i < size; ++i)
    {
        reverb.processSample(inSamples[0][i], inSamples[1][i], outSamples[0][i], outSamples[1][i]);
    }
    saveWaveFile(getResultFile(full_test_name + ".PlateReverb-new.wav", true), outSamples[0], outSamples[1], size, static_cast<unsigned>(SAMPLING_FREQUENCY), 16);

    size_t index = 0;
    size_t len = size;
    while(len > 0)
    {
        size_t grainSize = (len < 1024 ? len : 1024);

        reverb.doReverb(inSamples[0] + index, inSamples[1] + index, outSamples[0] + index, outSamples[1] + index, grainSize);

        index += grainSize;
        len -= grainSize;
    }
    saveWaveFile(getResultFile(full_test_name + ".PlateReverb-legacy.wav", true), outSamples[0], outSamples[1], size, static_cast<unsigned>(SAMPLING_FREQUENCY), 16);

    CLEANUP_AUDIO_TEST(inSamples, outSamples);
}
