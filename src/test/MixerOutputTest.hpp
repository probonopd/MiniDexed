#pragma once

#include <gtest/gtest.h>

#include "../mixing_console_constants.h"

TEST(MixerOutputTest, GetMixerOutputStringForTube)
{
    auto v = getMixerOutputString(MixerOutput::FX_Tube);
    EXPECT_EQ(v, "Tube");
}

TEST(MixerOutputTest, GetMixerOutputStringForChorus)
{
    auto v = getMixerOutputString(MixerOutput::FX_Chorus);
    EXPECT_EQ(v, "Chorus");
}

TEST(MixerOutputTest, GetMixerOutputStringForFlanger)
{
    auto v = getMixerOutputString(MixerOutput::FX_Flanger);
    EXPECT_EQ(v, "Flanger");
}

TEST(MixerOutputTest, GetMixerOutputStringForOrbitone)
{
    auto v = getMixerOutputString(MixerOutput::FX_Orbitone);
    EXPECT_EQ(v, "Orbitone");
}

TEST(MixerOutputTest, GetMixerOutputStringForPhaser)
{
    auto v = getMixerOutputString(MixerOutput::FX_Phaser);
    EXPECT_EQ(v, "Phaser");
}

TEST(MixerOutputTest, GetMixerOutputStringForDelay)
{
    auto v = getMixerOutputString(MixerOutput::FX_Delay);
    EXPECT_EQ(v, "Delay");
}

TEST(MixerOutputTest, GetMixerOutputStringForPlateReverb)
{
    auto v = getMixerOutputString(MixerOutput::FX_PlateReverb);
    EXPECT_EQ(v, "PlateReverb");
}

TEST(MixerOutputTest, GetMixerOutputStringForShimmerReverb)
{
    auto v = getMixerOutputString(MixerOutput::FX_ShimmerReverb);
    EXPECT_EQ(v, "ShimmerReverb");
}

TEST(MixerOutputTest, GetMixerOutputFromStringTube)
{
    MixerOutput v = getMixerOutputFromString("Tube");
    EXPECT_EQ(v, MixerOutput::FX_Tube);
}

TEST(MixerOutputTest, GetMixerOutputFromStringChorus)
{
    MixerOutput v = getMixerOutputFromString("Chorus");
    EXPECT_EQ(v, MixerOutput::FX_Chorus);
}

TEST(MixerOutputTest, GetMixerOutputFromStringFlanger)
{
    MixerOutput v = getMixerOutputFromString("Flanger");
    EXPECT_EQ(v, MixerOutput::FX_Flanger);
}

TEST(MixerOutputTest, GetMixerOutputFromStringOrbitone)
{
    MixerOutput v = getMixerOutputFromString("Orbitone");
    EXPECT_EQ(v, MixerOutput::FX_Orbitone);
}

TEST(MixerOutputTest, GetMixerOutputFromStringPhaser)
{
    MixerOutput v = getMixerOutputFromString("Phaser");
    EXPECT_EQ(v, MixerOutput::FX_Phaser);
}

TEST(MixerOutputTest, GetMixerOutputFromStringDelay)
{
    MixerOutput v = getMixerOutputFromString("Delay");
    EXPECT_EQ(v, MixerOutput::FX_Delay);
}

TEST(MixerOutputTest, GetMixerOutputFromStringPlateReverb)
{
    MixerOutput v = getMixerOutputFromString("PlateReverb");
    EXPECT_EQ(v, MixerOutput::FX_PlateReverb);
}

TEST(MixerOutputTest, GetMixerOutputFromStringShimmerReverb)
{
    MixerOutput v = getMixerOutputFromString("ShimmerReverb");
    EXPECT_EQ(v, MixerOutput::FX_ShimmerReverb);
}
