//
// mixing_console.hpp
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

// Implementation of the MixingConsole class defined in mixing_console.h

#include "mixing_console.h"

template<size_t nb_inputs>
MixingConsole<nb_inputs>::MixingConsole(float32_t sampling_rate, size_t buffer_size) :
    FXBase(sampling_rate),
    BufferSize(buffer_size)
{
    for(size_t i = 0; i < nb_inputs; ++i)
    {
        this->input_sample_buffer_[StereoChannels::Left ][i] = new float32_t[this->BufferSize];
        this->input_sample_buffer_[StereoChannels::Right][i] = new float32_t[this->BufferSize];
        memset(this->input_sample_buffer_[StereoChannels::Left ][i], 0, this->BufferSize);
        memset(this->input_sample_buffer_[StereoChannels::Right][i], 0, this->BufferSize);
    }

    memset(this->fx_, 0, MixerOutput::kFXCount * sizeof(FXElement*));

    this->fx_[MixerOutput::FX_Tube] = this->tube_ = new FXUnit2<Tube>(sampling_rate);
    this->fx_[MixerOutput::FX_Chorus] = this->chorus_ = new FXUnit2<Chorus>(sampling_rate);
    this->fx_[MixerOutput::FX_Flanger] = this->flanger_ = new FXUnit2<Flanger>(sampling_rate);
    this->fx_[MixerOutput::FX_Orbitone] = this->orbitone_ = new FXUnit2<Orbitone>(sampling_rate);
    this->fx_[MixerOutput::FX_Phaser] = this->phaser_ = new FXUnit2<Phaser>(sampling_rate);
    this->fx_[MixerOutput::FX_Delay] = this->delay_  = new FXUnit2<Delay>(sampling_rate);
    this->fx_[MixerOutput::FX_PlateReverb] = this->plate_reverb_ = new FXUnit2<AudioEffectPlateReverb>(sampling_rate);
    this->fx_[MixerOutput::FX_ShimmerReverb] = this->shimmer_reverb_ = new FXUnit2<ShimmerReverb>(sampling_rate);
    this->fx_[MixerOutput::MainOutput] = this->dry_ = new FXUnit2<Dry>(sampling_rate);

    this->init();
}

template<size_t nb_inputs>
MixingConsole<nb_inputs>::~MixingConsole()
{
    for(size_t i = 0; i < nb_inputs; ++i)
    {
        delete this->input_sample_buffer_[StereoChannels::Left ][i];
        delete this->input_sample_buffer_[StereoChannels::Right][i];
    }

    for(size_t i = 0; i < MixerOutput::kFXCount; ++i)
    {
        delete this->fx_[i];
    }
}

// Send section

template<size_t nb_inputs>
void MixingConsole<nb_inputs>::setChannelLevel(size_t in, float32_t lvl)
{
    assert(in < nb_inputs);

    lvl = constrain(lvl, 0.0f, 1.0f);
    if(lvl == this->channel_level_[in]) return;

    this->channel_level_[in] = lvl;
    this->updatePan();
}

template<size_t nb_inputs>
void MixingConsole<nb_inputs>::setPan(size_t in, float32_t pan)
{
    assert(in < nb_inputs);

    pan = constrain(pan, 0.0f, 1.0f);
    
    if(pan == this->pan_[StereoChannels::kNumChannels][in]) return;

    this->pan_[StereoChannels::kNumChannels][in] = pan;
    this->updatePan(in);
}

template<size_t nb_inputs>
void MixingConsole<nb_inputs>::updatePan(size_t in)
{
    float32_t pan = mapfloat(this->pan_[StereoChannels::kNumChannels][in], 0.0f, 1.0f, 0.0, Constants::MPI_2);
    this->pan_[StereoChannels::Left ][in] = arm_sin_f32(pan) * this->channel_level_[in];
    this->pan_[StereoChannels::Right][in] = arm_cos_f32(pan) * this->channel_level_[in];
}

template<size_t nb_inputs>
void MixingConsole<nb_inputs>::setSendLevel(size_t in, MixerOutput fx, float32_t lvl)
{
    assert(in < nb_inputs);
    assert(fx < kFXCount);

    this->setLevel(in, fx, lvl);
}

template<size_t nb_inputs>
void MixingConsole<nb_inputs>::setInputSample(size_t in, float32_t sampleL, float32_t sampleR)
{
    assert(in < nb_inputs);

    this->setSample(in, sampleL, sampleR);
}

template<size_t nb_inputs>
void MixingConsole<nb_inputs>::setInputSampleBuffer(size_t in, float32_t* samples)
{
    assert(in < nb_inputs);

    if(samples != nullptr)
    {
        arm_scale_f32(samples, this->pan_[StereoChannels::Left ][in], this->input_sample_buffer_[StereoChannels::Left ][in], this->BufferSize);
        arm_scale_f32(samples, this->pan_[StereoChannels::Right][in], this->input_sample_buffer_[StereoChannels::Right][in], this->BufferSize);
    }
    else
    {
        memset(this->input_sample_buffer_[StereoChannels::Left ][in], 0, this->BufferSize * sizeof(float32_t));
        memset(this->input_sample_buffer_[StereoChannels::Right][in], 0, this->BufferSize * sizeof(float32_t));
    }
}

template<size_t nb_inputs>
void MixingConsole<nb_inputs>::setInputSampleBuffer(size_t in, float32_t* samplesL, float32_t* samplesR)
{
    assert(in < nb_inputs);
    if(samplesL != nullptr)
    {
        memcpy(this->input_sample_buffer_[StereoChannels::Left ][in], samplesL, this->BufferSize * sizeof(float32_t));
    }
    else
    {
        memset(this->input_sample_buffer_[StereoChannels::Left ][in], 0, this->BufferSize * sizeof(float32_t));
    }

    if(samplesR != nullptr)
    {
        memcpy(this->input_sample_buffer_[StereoChannels::Right][in], samplesR, this->BufferSize * sizeof(float32_t));
    }
    else
    {
        memset(this->input_sample_buffer_[StereoChannels::Right][in], 0, this->BufferSize * sizeof(float32_t));
    }
}

// Return section

template<size_t nb_inputs>
void MixingConsole<nb_inputs>::setReturnLevel(MixerOutput ret, MixerOutput dest, float32_t lvl)
{
    assert(ret < (kFXCount - 1));
    assert(dest < kFXCount);

    if(ret == dest)
    {
        // An FX cannot feedback on itself
        return;
    }

    this->setLevel(nb_inputs + ret, dest, lvl);
}

template<size_t nb_inputs>
void MixingConsole<nb_inputs>::setReturnSample(MixerOutput ret, float32_t sampleL, float32_t sampleR)
{
    assert(ret < (kFXCount - 1));

    this->setSample(nb_inputs + ret, sampleL, sampleR);
}

// Global section

template<size_t nb_inputs>
void MixingConsole<nb_inputs>::setLevel(size_t in, MixerOutput fx, float32_t lvl)
{
    assert(in < (nb_inputs + MixerOutput::kFXCount - 1));
    assert(fx < MixerOutput::kFXCount);

    this->levels_[fx][in] = constrain(lvl, 0.0f, 1.0f);
}

template<size_t nb_inputs>
void MixingConsole<nb_inputs>::setSample(size_t in, float32_t sampleL, float32_t sampleR)
{
    assert(in < (nb_inputs + MixerOutput::kFXCount - 1));
    this->input_samples_[StereoChannels::Left ][in] = sampleL;
    this->input_samples_[StereoChannels::Right][in] = sampleR;
}

// Get FX
template<size_t nb_inputs>
FXElement* MixingConsole<nb_inputs>::getFX(size_t fx)
{
    assert(fx < MixerOutput::kFXCount);
    return this->fx_[fx];
}

template<size_t nb_inputs>
FXUnit2<Tube>* MixingConsole<nb_inputs>::getTube()
{
    return this->tube_;
}

template<size_t nb_inputs>
FXUnit2<Chorus>* MixingConsole<nb_inputs>::getChorus()
{
    return this->chorus_;
}

template<size_t nb_inputs>
FXUnit2<Flanger>* MixingConsole<nb_inputs>::getFlanger()
{
    return this->flanger_;
}

template<size_t nb_inputs>
FXUnit2<Orbitone>* MixingConsole<nb_inputs>::getOrbitone()
{
    return this->orbitone_;
}

template<size_t nb_inputs>
FXUnit2<Phaser>* MixingConsole<nb_inputs>::getPhaser()
{
    return this->phaser_;
}

template<size_t nb_inputs>
FXUnit2<Delay>* MixingConsole<nb_inputs>::getDelay()
{
    return this->delay_;
}

template<size_t nb_inputs>
FXUnit2<AudioEffectPlateReverb>* MixingConsole<nb_inputs>::getPlateReverb()
{
    return this->plate_reverb_;
}

template<size_t nb_inputs>
FXUnit2<ShimmerReverb>* MixingConsole<nb_inputs>::getShimmerReverb()
{
    return this->shimmer_reverb_;
}

template<size_t nb_inputs>
FXUnit2<Dry>* MixingConsole<nb_inputs>::getDry()
{
    return this->dry_;
}

// Processing

template<size_t nb_inputs>
void MixingConsole<nb_inputs>::init()
{
    for(size_t i = 0; i < MixerOutput::kFXCount; ++i)
        memset(this->levels_[i], 0, (nb_inputs + MixerOutput::kFXCount - 1) * sizeof(float32_t));
    
    for(size_t i = 0; i < StereoChannels::kNumChannels; ++i) 
        memset(this->input_samples_[i], 0, (nb_inputs + MixerOutput::kFXCount - 1) * sizeof(float32_t));

    this->reset();
}

template<size_t nb_inputs>
void MixingConsole<nb_inputs>::reset()
{
    for(size_t i = 0; i < nb_inputs; ++i)
    {
        memset(this->input_sample_buffer_[StereoChannels::Left ][i], 0, this->BufferSize);
        memset(this->input_sample_buffer_[StereoChannels::Right][i], 0, this->BufferSize);
    }

    for(size_t i = 0; i < MixerOutput::kFXCount; ++i)
    {
        this->fx_[i]->reset();
    }

    for(size_t i = 0; i < MixerOutput::MainOutput; ++i)
    {
        this->setReturnSample(static_cast<MixerOutput>(i), 0.0f, 0.0f);
    }
}

template<size_t nb_inputs>
void MixingConsole<nb_inputs>::processSample(float32_t& outL, float32_t& outR)
{
    float32_t fx_inputs_[MixerOutput::kFXCount][StereoChannels::kNumChannels];
    float32_t fx_outputs_[MixerOutput::kFXCount][StereoChannels::kNumChannels];

    for(size_t i = 0; i < MixerOutput::kFXCount; ++i)
    {
        // Compute the samples that will feed the MixerOutput and process MixerOutput
        fx_inputs_[i][StereoChannels::Left ] = arm_weighted_sum_f32(this->input_samples_[StereoChannels::Left ], this->levels_[i], nb_inputs + MixerOutput::kFXCount - 1);
        fx_inputs_[i][StereoChannels::Right] = arm_weighted_sum_f32(this->input_samples_[StereoChannels::Right], this->levels_[i], nb_inputs + MixerOutput::kFXCount - 1);

        // Process the FX
        this->fx_[i]->processSample(
            fx_inputs_[i][StereoChannels::Left],
            fx_inputs_[i][StereoChannels::Right],
            fx_outputs_[i][StereoChannels::Left],
            fx_outputs_[i][StereoChannels::Right]
        );

        if(i != MixerOutput::MainOutput)
        {
            // Feedback the resulting samples except for the main output
            this->setReturnSample(
                static_cast<MixerOutput>(i), 
                fx_outputs_[i][StereoChannels::Left],
                fx_outputs_[i][StereoChannels::Right]
            );
        }
    }

    // Return this main output sample
    outL = fx_inputs_[MixerOutput::MainOutput][StereoChannels::Left];
    outR = fx_inputs_[MixerOutput::MainOutput][StereoChannels::Right];
}

template<size_t nb_inputs>
void MixingConsole<nb_inputs>::prepare()
{
    for(size_t i = 0; i < MixerOutput::kFXCount; ++i)
    {
        this->fx_[i]->prepare();
    }
}

template<size_t nb_inputs>
void MixingConsole<nb_inputs>::process(float32_t* outL, float32_t* outR)
{
    this->prepare();

    for(size_t s = 0; s < this->BufferSize; ++s)
    {
        for(size_t in = 0; in < nb_inputs; ++in)
        {
            this->setSample(
                in, 
                this->input_sample_buffer_[StereoChannels::Left ][in][s], 
                this->input_sample_buffer_[StereoChannels::Right][in][s]
            );
        }

        this->processSample(*outL, *outR);
        ++outL;
        ++outR;
    }
}