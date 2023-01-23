#include <gtest/gtest.h>

#include "test_fx_helper.h"
#include "wave.h"

#include "../effect_platervbstereo.h"

TEST(FXElement, PlateReverbMigration)
{
    const unsigned nbRepeats = 4;

    AudioEffectPlateReverb* reverb = new AudioEffectPlateReverb(SAMPLING_FREQUENCY);
    reverb->set_bypass(false);
    reverb->size(0.7f);
    reverb->hidamp(0.5f);
    reverb->lodamp(0.5f);
    reverb->lowpass(0.3f);
    reverb->diffusion(0.65f);
    reverb->level(1.0f);

    unsigned size;
    float32_t** samples = readWaveFile(AUDIO_SOURCE_FILE, size);
    float32_t* sampleOutL = new float32_t[size * nbRepeats];
    float32_t* sampleOutR = new float32_t[size * nbRepeats];
    memset(sampleOutL, 0, size * nbRepeats * sizeof(float32_t));
    memset(sampleOutR, 0, size * nbRepeats * sizeof(float32_t));

    unsigned index = 0;
    for(unsigned i = 0; i < nbRepeats; ++i)
    {
        for(unsigned j = 0; j < size; ++j)
        {
            reverb->processSample(samples[0][j], samples[1][j], sampleOutL[index], sampleOutR[index]);
            ++index;
        }
    }
    saveWaveFile(getResultFile("result-PlateReverb-new.wav"), sampleOutL, sampleOutR, nbRepeats * size, static_cast<unsigned>(SAMPLING_FREQUENCY), 16);

    unsigned indexOut = 0;
    for (unsigned i = 0; i < nbRepeats; ++i)
    {
        unsigned len = size;
        unsigned indexIn = 0;

        while(len > 0)
        {
            unsigned grainSize = (len < 1024 ? len : 1024);

            reverb->doReverb(samples[0] + indexIn, samples[1] + indexIn, sampleOutL + indexOut, sampleOutR + indexOut, grainSize);

            indexIn += grainSize;
            indexOut += grainSize;
            len -= grainSize;
        }

    }
    saveWaveFile(getResultFile("result-PlateReverb-legacy.wav"), sampleOutL, sampleOutR, nbRepeats * size, static_cast<unsigned>(SAMPLING_FREQUENCY), 16);

    delete[] sampleOutL;
    delete[] sampleOutR;
    delete[] samples[0];
    delete[] samples[1];
    delete[] samples;

    delete reverb;
}
