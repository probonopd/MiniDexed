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
#pragma once

#include "mixing_console_constants.h"
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

template<size_t nb_inputs>
class MixingConsole : public FXBase
{
    DISALLOW_COPY_AND_ASSIGN(MixingConsole);

public:
    MixingConsole(float32_t sampling_rate, size_t buffer_size);
    ~MixingConsole();

    // Send section
    inline void setChannelLevel(size_t in, float32_t lvl);
    inline void setPan(size_t in, float32_t pan);
    inline void setSendLevel(size_t in, MixerOutput fx, float32_t lvl);
    inline void setInputSample(size_t in, float32_t sampleL, float32_t sampleR);
    inline void setInputSampleBuffer(size_t in, float32_t* samples);
    inline void setInputSampleBuffer(size_t in, float32_t* samplesL, float32_t* samplesR);

    // Return section
    inline void setReturnLevel(MixerOutput ret, MixerOutput dest, float32_t lvl);
    inline void setReturnSample(MixerOutput ret, float32_t sampleL, float32_t sampleR);

    // Get FX
    inline FXElement* getFX(size_t fx);
    inline FXUnit2<Tube>* getTube();
    inline FXUnit2<Chorus>* getChorus();
    inline FXUnit2<Flanger>* getFlanger();
    inline FXUnit2<Orbitone>* getOrbitone();
    inline FXUnit2<Phaser>* getPhaser();
    inline FXUnit2<Delay>* getDelay();
    inline FXUnit2<AudioEffectPlateReverb>* getPlateReverb();
    inline FXUnit2<ShimmerReverb>* getShimmerReverb();
    inline FXUnit2<Dry>* getDry();

    // Processing
    inline void init();
    inline void reset() override;
    inline void processSample(float32_t& outL, float32_t& outR);
    void process(float32_t* outL, float32_t* outR);

protected:
    inline void updatePan(size_t in);
    inline void setLevel(size_t in, MixerOutput fx, float32_t lvl);
    inline void setSample(size_t in, float32_t sampleL, float32_t sampleR);

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

    IMPLEMENT_DUMP(
        const size_t space = 10;
        const size_t precision = 5;

        std::stringstream ss;

        out << "START " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;

        out << "\t" << "Input levels & Pan:" << std::endl;
        {
            SS_RESET(ss, precision, std::left);
            SS_SPACE(ss, ' ', space, std::left, '|');
            SS__TEXT(ss, ' ', space, std::left, '|', "Level");
            SS__TEXT(ss, ' ', space, std::left, '|', "Pan L");
            SS__TEXT(ss, ' ', space, std::left, '|', "Pan R");
            SS__TEXT(ss, ' ', space, std::left, '|', "Pan");
            out << "\t" << ss.str() << std::endl;

            SS_RESET(ss, precision, std::left);
            SS_SPACE(ss, '-', space, std::left, '+');
            SS_SPACE(ss, '-', space, std::left, '+');
            SS_SPACE(ss, '-', space, std::left, '+');
            SS_SPACE(ss, '-', space, std::left, '+');
            SS_SPACE(ss, '-', space, std::left, '+');
            out << "\t" << ss.str() << std::endl;

            for(size_t i = 0; i < nb_inputs; ++i)
            {
                std::stringstream s;
                s << "* Input ";
                s << (i + 1);

                SS_RESET(ss, precision, std::left);
                SS__TEXT(ss, ' ', space, std::left, '|', s.str());
                SS__TEXT(ss, ' ', space - 1, std::right, " |", this->channel_level_[i]);
                SS__TEXT(ss, ' ', space - 1, std::right, " |", this->pan_[StereoChannels::Left][i]);
                SS__TEXT(ss, ' ', space - 1, std::right, " |", this->pan_[StereoChannels::Right][i]);
                SS__TEXT(ss, ' ', space - 1, std::right, " |", this->pan_[StereoChannels::kNumChannels][i]);

                out << "\t" << ss.str() << std::endl;
            }
        }
        out << std::endl;

        out << "\t" << "Mixing Console input samples:" << std::endl;
        {
            SS_RESET(ss, precision, std::left);
            SS_SPACE(ss, ' ', space, std::left, '|');
            for(size_t i = 0; i < nb_inputs; ++i)
            {
                std::stringstream s;
                s << "Input ";
                s << (i + 1);

                SS__TEXT(ss, ' ', space, std::left, '|', s.str());
            }
            for(size_t i = 0; i < (MixerOutput::kFXCount - 1); ++i)
            {
                std::string s = toString(static_cast<MixerOutput>(i));
                s.resize(space);
                SS__TEXT(ss, ' ', space, std::left, '|', s.c_str());
            }
            out << "\t" << ss.str() << std::endl;

            SS_RESET(ss, precision, std::left);
            SS_SPACE(ss, '-', space, std::left, '+');
            for(size_t i = 0; i < nb_inputs; ++i)
            {
                SS_SPACE(ss, '-', space, std::left, '+');
            }
            for(size_t i = 0; i < (MixerOutput::kFXCount - 1); ++i)
            {
                SS_SPACE(ss, '-', space, std::left, '+');
            }
            out << "\t" << ss.str() << std::endl;

            const char* LR = "LR";
            for(size_t c = 0; c < StereoChannels::kNumChannels; ++c)
            {
                std::stringstream s;
                s << "* Input ";
                s << LR[c];

                SS_RESET(ss, precision, std::left);
                SS__TEXT(ss, ' ', space, std::left, '|', s.str());
                for(size_t i = 0; i < (nb_inputs + MixerOutput::kFXCount - 1); ++i)
                {
                    SS__TEXT(ss, ' ', space - 1, std::right, " |", this->input_samples_[c][i]);
                }
                out << "\t" << ss.str() << std::endl;
            }
        }
        out << std::endl;

        out << "\t" << "Mixing Console levels:" << std::endl;
        {
            SS_RESET(ss, precision, std::left);
            SS_SPACE(ss, ' ', space, std::left, '|');
            for(size_t i = 0; i < nb_inputs; ++i)
            {
                std::stringstream s;
                s << "Input ";
                s << (i + 1);

                SS__TEXT(ss, ' ', space, std::left, '|', s.str());
            }
            for(size_t i = 0; i < (MixerOutput::kFXCount - 1); ++i)
            {
                std::string s = toString(static_cast<MixerOutput>(i));
                s.resize(space);
                SS__TEXT(ss, ' ', space, std::left, '|', s.c_str());
            }
            out << "\t" << ss.str() << std::endl;

            SS_RESET(ss, precision, std::left);
            SS_SPACE(ss, '-', space, std::left, '+');
            for(size_t i = 0; i < nb_inputs; ++i)
            {
                SS_SPACE(ss, '-', space, std::left, '+');
            }
            for(size_t i = 0; i < (MixerOutput::kFXCount - 1); ++i)
            {
                SS_SPACE(ss, '-', space, std::left, '+');
            }
            out << "\t" << ss.str() << std::endl;

            for(size_t c = 0; c < MixerOutput::kFXCount; ++c)
            {
                SS_RESET(ss, precision, std::left);
                std::string s = toString(static_cast<MixerOutput>(c));
                s.resize(space);
                SS__TEXT(ss, ' ', space, std::left, '|', s.c_str());
                for(size_t i = 0; i < (nb_inputs + MixerOutput::kFXCount - 1); ++i)
                {
                    SS__TEXT(ss, ' ', space - 1, std::right, " |", this->levels_[c][i]);
                }
                out << "\t" << ss.str() << std::endl;
            }
        }
        out << std::endl;

        if(deepInspection)
        {
            this->tube_->dump(out, deepInspection, tag + ".tube_");
            this->chorus_->dump(out, deepInspection, tag + ".chorus_");
            this->flanger_->dump(out, deepInspection, tag + ".flanger_");
            this->orbitone_->dump(out, deepInspection, tag + ".orbitone_");
            this->phaser_->dump(out, deepInspection, tag + ".phaser_");
            this->delay_->dump(out, deepInspection, tag + ".delay_");
            this->plate_reverb_->dump(out, deepInspection, tag + ".plate_reverb_");
            this->shimmer_reverb_->dump(out, deepInspection, tag + ".shimmer_reverb_");
            this->dry_->dump(out, deepInspection, tag + ".dry_");
        }

        out << "END " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;
    )

    IMPLEMENT_INSPECT(
        size_t nb_errors = 0;

        for(size_t i = 0; i < nb_inputs; ++i)
        {
            nb_errors += inspector(tag + ".level[ input #" + std::to_string(i) + " ]" , this->channel_level_[i], -1.0f, 1.0f, deepInspection);
            nb_errors += inspector(tag + ".pan[ L ][ input #" + std::to_string(i) + " ]", this->pan_[StereoChannels::Left][i], -1.0f, 1.0f, deepInspection);
            nb_errors += inspector(tag + ".pan[ R ][ input #" + std::to_string(i) + " ]", this->pan_[StereoChannels::Right][i], -1.0f, 1.0f, deepInspection);
            nb_errors += inspector(tag + ".pan[ input #" + std::to_string(i) + " ]", this->pan_[StereoChannels::kNumChannels][i], -1.0f, 1.0f, deepInspection);
        }

        for(size_t i = 0; i < nb_inputs; ++i)
        {
            nb_errors += inspector(tag + ".input[ L ][ input #" + std::to_string(i) + " ]", this->input_samples_[StereoChannels::Left ][i], -1.0f, 1.0f, deepInspection);
            nb_errors += inspector(tag + ".input[ R ][ input #" + std::to_string(i) + " ]", this->input_samples_[StereoChannels::Right][i], -1.0f, 1.0f, deepInspection);
        }

        for(size_t i = nb_inputs; i < (nb_inputs + MixerOutput::kFXCount - 1); ++i)
        {
            nb_errors += inspector(tag + ".input[ L ][ input " + toString(static_cast<MixerOutput>(i - nb_inputs)) + " ]", this->input_samples_[StereoChannels::Left ][i], -1.0f, 1.0f, deepInspection);
            nb_errors += inspector(tag + ".input[ R ][ input " + toString(static_cast<MixerOutput>(i - nb_inputs)) + " ]", this->input_samples_[StereoChannels::Right][i], -1.0f, 1.0f, deepInspection);
        }

        for(size_t c = 0; c < MixerOutput::kFXCount; ++c)
        {
            for(size_t i = 0; i < (nb_inputs + MixerOutput::kFXCount - 1); ++i)
            {
                nb_errors += inspector(tag + ".levels[ " + std::to_string(c) + " ][ " + std::to_string(i) + " ]", this->levels_[c][i], -1.0f, 1.0f, deepInspection);
            }
        }

        if(deepInspection)
        {
            for(size_t i = 0; i < nb_inputs; ++i)
            {
                for(size_t k = 0; k < this->BufferSize; ++k)
                {
                    nb_errors += inspector(tag + ".input_sample_buffer_[ L ][ " + std::to_string(i) + " ][ " + std::to_string(k) +" ] ", this->input_sample_buffer_[StereoChannels::Left ][i][k], -1.0f, 1.0f, deepInspection);
                    nb_errors += inspector(tag + ".input_sample_buffer_[ R ][ " + std::to_string(i) + " ][ " + std::to_string(k) +" ] ", this->input_sample_buffer_[StereoChannels::Right][i][k], -1.0f, 1.0f, deepInspection);
                }
            }

            nb_errors += this->tube_->inspect(inspector, deepInspection, tag + ".tube_");
            nb_errors += this->chorus_->inspect(inspector, deepInspection, tag + ".chorus_");
            nb_errors += this->flanger_->inspect(inspector, deepInspection, tag + ".flanger_");
            nb_errors += this->orbitone_->inspect(inspector, deepInspection, tag + ".orbitone_");
            nb_errors += this->phaser_->inspect(inspector, deepInspection, tag + ".phaser_");
            nb_errors += this->delay_->inspect(inspector, deepInspection, tag + ".delay_");
            nb_errors += this->plate_reverb_->inspect(inspector, deepInspection, tag + ".plate_reverb_");
            nb_errors += this->shimmer_reverb_->inspect(inspector, deepInspection, tag + ".shimmer_reverb_");
            nb_errors += this->dry_->inspect(inspector, deepInspection, tag + ".dry_");
        }

        return nb_errors;
    )
};


template<size_t nb_inputs>
MixingConsole<nb_inputs>::MixingConsole(float32_t sampling_rate, size_t buffer_size) :
    FXBase(sampling_rate),
    BufferSize(buffer_size)
{
    for(size_t i = 0; i < nb_inputs; ++i)
    {
        this->input_sample_buffer_[StereoChannels::Left ][i] = new float32_t[this->BufferSize];
        this->input_sample_buffer_[StereoChannels::Right][i] = new float32_t[this->BufferSize];
        memset(this->input_sample_buffer_[StereoChannels::Left ][i], 0, sizeof(float32_t) * this->BufferSize);
        memset(this->input_sample_buffer_[StereoChannels::Right][i], 0, sizeof(float32_t) * this->BufferSize);
    }

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
    this->updatePan(in);
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
void MixingConsole<nb_inputs>::setSendLevel(size_t in, MixerOutput fx, float32_t lvl)
{
    assert(in < nb_inputs);
    assert(fx < MixerOutput::kFXCount);

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
    assert(ret < (MixerOutput::kFXCount - 1));
    assert(dest < MixerOutput::kFXCount);

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
    assert(ret < (MixerOutput::kFXCount - 1));

    this->setSample(nb_inputs + ret, sampleL, sampleR);
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
    memset(this->channel_level_, 0, nb_inputs * sizeof(float32_t));
    for(size_t i = 0; i <= StereoChannels::kNumChannels; ++i) memset(this->pan_[i], 0, nb_inputs * sizeof(float32_t));

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
        memset(this->input_sample_buffer_[StereoChannels::Left ][i], 0, this->BufferSize * sizeof(float32_t));
        memset(this->input_sample_buffer_[StereoChannels::Right][i], 0, this->BufferSize * sizeof(float32_t));
    }

    for(size_t i = 0; i < MixerOutput::kFXCount; ++i)
    {
        this->fx_[i]->reset();

        if(i != MixerOutput::MainOutput)
        {
            this->setReturnSample(static_cast<MixerOutput>(i), 0.0f, 0.0f);
        }
    }
}

template<size_t nb_inputs>
void MixingConsole<nb_inputs>::processSample(float32_t& outL, float32_t& outR)
{
    const size_t bufferSize = nb_inputs + MixerOutput::kFXCount - 1;

    float32_t fx_inputs_[MixerOutput::kFXCount][StereoChannels::kNumChannels];
    float32_t fx_outputs_[MixerOutput::kFXCount][StereoChannels::kNumChannels];
    for(size_t i = 0; i < MixerOutput::kFXCount; ++i)
    {
        // Compute the samples that will feed the MixerOutput and process MixerOutput
        fx_inputs_[i][StereoChannels::Left ] = arm_weighted_sum_f32(this->input_samples_[StereoChannels::Left ], this->levels_[i], bufferSize);
        fx_inputs_[i][StereoChannels::Right] = arm_weighted_sum_f32(this->input_samples_[StereoChannels::Right], this->levels_[i], bufferSize);

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
void MixingConsole<nb_inputs>::process(float32_t* outL, float32_t* outR)
{
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

template<size_t nb_inputs>
void MixingConsole<nb_inputs>::updatePan(size_t in)
{
    float32_t pan = mapfloat(this->pan_[StereoChannels::kNumChannels][in], 0.0f, 1.0f, 0.0, Constants::MPI_2);
    this->pan_[StereoChannels::Left ][in] = arm_cos_f32(pan) * this->channel_level_[in];
    this->pan_[StereoChannels::Right][in] = arm_sin_f32(pan) * this->channel_level_[in];
}

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
