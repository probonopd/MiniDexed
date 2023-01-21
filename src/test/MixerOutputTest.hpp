#pragma once

#include <gtest/gtest.h>

#include "../mixing_console.h"

TEST(MixerOutputTest, toStringForTube)
{
    auto v = toString(MixerOutput::FX_Tube);
    EXPECT_EQ(v, "Tube");
}

TEST(MixerOutputTest, toStringForChorus)
{
    auto v = toString(MixerOutput::FX_Chorus);
    EXPECT_EQ(v, "Chorus");
}

TEST(MixerOutputTest, toStringForFlanger)
{
    auto v = toString(MixerOutput::FX_Flanger);
    EXPECT_EQ(v, "Flanger");
}

TEST(MixerOutputTest, toStringForOrbitone)
{
    auto v = toString(MixerOutput::FX_Orbitone);
    EXPECT_EQ(v, "Orbitone");
}

TEST(MixerOutputTest, toStringForPhaser)
{
    auto v = toString(MixerOutput::FX_Phaser);
    EXPECT_EQ(v, "Phaser");
}

TEST(MixerOutputTest, toStringForDelay)
{
    auto v = toString(MixerOutput::FX_Delay);
    EXPECT_EQ(v, "Delay");
}

TEST(MixerOutputTest, toStringForPlateReverb)
{
    auto v = toString(MixerOutput::FX_PlateReverb);
    EXPECT_EQ(v, "PlateReverb");
}

TEST(MixerOutputTest, toStringForShimmerReverb)
{
    auto v = toString(MixerOutput::FX_ShimmerReverb);
    EXPECT_EQ(v, "ShimmerReverb");
}

TEST(MixerOutputTest, toIndexTube)
{
    MixerOutput v = toIndex("Tube");
    EXPECT_EQ(v, MixerOutput::FX_Tube);
}

TEST(MixerOutputTest, toIndexChorus)
{
    MixerOutput v = toIndex("Chorus");
    EXPECT_EQ(v, MixerOutput::FX_Chorus);
}

TEST(MixerOutputTest, toIndexFlanger)
{
    MixerOutput v = toIndex("Flanger");
    EXPECT_EQ(v, MixerOutput::FX_Flanger);
}

TEST(MixerOutputTest, toIndexOrbitone)
{
    MixerOutput v = toIndex("Orbitone");
    EXPECT_EQ(v, MixerOutput::FX_Orbitone);
}

TEST(MixerOutputTest, toIndexPhaser)
{
    MixerOutput v = toIndex("Phaser");
    EXPECT_EQ(v, MixerOutput::FX_Phaser);
}

TEST(MixerOutputTest, toIndexDelay)
{
    MixerOutput v = toIndex("Delay");
    EXPECT_EQ(v, MixerOutput::FX_Delay);
}

TEST(MixerOutputTest, toIndexPlateReverb)
{
    MixerOutput v = toIndex("PlateReverb");
    EXPECT_EQ(v, MixerOutput::FX_PlateReverb);
}

TEST(MixerOutputTest, toIndexShimmerReverb)
{
    MixerOutput v = toIndex("ShimmerReverb");
    EXPECT_EQ(v, MixerOutput::FX_ShimmerReverb);
}
