#include <gtest/gtest.h>

#include "test_fx_helper.h"
#include "wave.h"

#include "../debug.hpp"
#include "../fx_base.h"

TEST(Framework, TestWaveIn)
{
    unsigned size;
    float32_t** samples = readWaveFile(AUDIO_SOURCE_FILE, size);

    size_t nb_errors = 0;
    for(size_t i = 0; i < size; ++i)
    {
        nb_errors += fullInspector("L", samples[StereoChannels::Left ][i], -1.0f, 1.0f, true);
        nb_errors += fullInspector("R", samples[StereoChannels::Right][i], -1.0f, 1.0f, true);
    }

    EXPECT_EQ(nb_errors, 0);
}