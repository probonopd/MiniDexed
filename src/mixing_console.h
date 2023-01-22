//
// mixing_console.h
//
// MiniDexed - Dexed FM synthesizer for bare metal Raspberry Pi
// Copyright (C) 2022  The MiniDexed Team
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#pragma once

#include "mixing_console_constants.h"
#include "fx.h"
#include "fx_tube.h"
#include "fx_chorus.h"
#include "fx_flanger.h"
#include "fx_orbitone.h"
#include "fx_phaser.h"
#include "fx_delay.h"
#include "effect_platervbstereo.h"
#include "fx_shimmer_reverb.h"
#include "fx_dry.h"
#include "fx_unit2.hpp"

template<size_t nb_inputs = 8>
class MixingConsole : public FXBase
{
    DISALLOW_COPY_AND_ASSIGN(MixingConsole);

public:
    MixingConsole(float32_t sampling_rate, size_t buffer_size);
    ~MixingConsole();

    void setChannelLevel(size_t in, float32_t lvl);
    void setPan(size_t in, float32_t pan);
    void setSendLevel(size_t in, MixerOutput fx, float32_t lvl);
    void setInputSample(size_t in, float32_t sampleL, float32_t sampleR);
    void setInputSampleBuffer(size_t in, float32_t* samples);
    void setInputSampleBuffer(size_t in, float32_t* samplesL, float32_t* samplesR);

    void setReturnLevel(MixerOutput ret, MixerOutput dest, float32_t lvl);
    void setReturnSample(MixerOutput ret, float32_t _sampleL, float32_t _sampleR);

    FXElement* getFX(size_t fx);
    FXUnit2<Tube>* getTube();
    FXUnit2<Chorus>* getChorus();
    FXUnit2<Flanger>* getFlanger();
    FXUnit2<Orbitone>* getOrbitone();
    FXUnit2<Phaser>* getPhaser();
    FXUnit2<Delay>* getDelay();
    FXUnit2<AudioEffectPlateReverb>* getPlateReverb();
    FXUnit2<ShimmerReverb>* getShimmerReverb();
    FXUnit2<Dry>* getDry();

    void init();
    virtual void reset() override;
    virtual void prepare() override;
    void processSample(float32_t& outL, float32_t& outR);
    void process(float32_t* outL, float32_t* outR);

protected:
    void updatePan(size_t in);
    void setLevel(size_t in, MixerOutput fx, float32_t lvl);
    void setSample(size_t in, float32_t sampleL, float32_t sampleR);

private:
    const size_t BufferSize;

    float32_t channel_level_[nb_inputs];
    float32_t pan_[StereoChannels::kNumChannels + 1][nb_inputs];
    float32_t* input_sample_buffer_[StereoChannels::kNumChannels][nb_inputs];
    float32_t input_samples_[StereoChannels::kNumChannels][nb_inputs + MixerOutput::kFXCount - 1];
    float32_t levels_[MixerOutput::kFXCount][nb_inputs + MixerOutput::kFXCount - 1];

    FXElement* fx_[MixerOutput::kFXCount];
    FXUnit2<Tube>* tube_;
    FXUnit2<Chorus>* chorus_;
    FXUnit2<Flanger>* flanger_;
    FXUnit2<Orbitone>* orbitone_;
    FXUnit2<Phaser>* phaser_;
    FXUnit2<Delay>* delay_;
    FXUnit2<AudioEffectPlateReverb>* plate_reverb_;
    FXUnit2<ShimmerReverb>* shimmer_reverb_;
    FXUnit2<Dry>* dry_;
};

#include "mixing_console.cpp"
