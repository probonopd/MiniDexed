#include <gtest/gtest.h>

#include "test_fx_helper.h"

#include "../debug.hpp"
#include "../fx_base.h"

TEST(Framework, TestWaveIn)
{
    size_t size;
    float32_t** samples = readWaveFile(AUDIO_SOURCE_FILE, size);

    size_t nb_errors = 0;
    for(size_t i = 0; i < size; ++i)
    {
        nb_errors += fullInspector("L", samples[StereoChannels::Left ][i], -1.0f, 1.0f, true);
        nb_errors += fullInspector("R", samples[StereoChannels::Right][i], -1.0f, 1.0f, true);
    }

    ASSERT_EQ(nb_errors, 0) << "readWaveFile returns NaN of out of bounds samples: " << nb_errors << " out of " << size;
}
