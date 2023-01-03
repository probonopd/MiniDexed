#include "fx_shimmer_reverb2.h"

#include <cmath>
#include <algorithm>

ShimmerReverb2::ShimmerReverb2( float32_t sampling_rate, 
                                float32_t decay, 
                                float32_t diffusion, 
                                float32_t pitch_shift) : 
    FXElement(sampling_rate),
    FXUnitModule(),
    reverb_buffer_index_(0)
{
    this->setDecay(decay);
    this->setDiffusion(diffusion);
    this->setPitchShift(pitch_shift);
}

ShimmerReverb2::~ShimmerReverb2()
{
}

void ShimmerReverb2::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    if(!this->isEnable())
    {
        outL = inL;
        outR = inR;
        return;
    }

    // Processing left channel
    {
        // Read the sample from the reverb buffer
        float32_t reverb_sample = this->reverb_buffer_L_[this->reverb_buffer_index_];

        // Calculate the pitch-shifted sample
        float32_t pitch_shift_sample = reverb_sample * this->getPitchShift();

        // Interpolate the pitch-shifted sample to the original pitch
        float32_t pitch_shift_frac = pitch_shift_sample - std::floor(pitch_shift_sample);
        unsigned pitch_shift_index = static_cast<unsigned>(SHIMMER_BUFFER_SIZE + std::floor(pitch_shift_sample)) % SHIMMER_BUFFER_SIZE;
        float32_t pitch_shift_interp = 
                    (1.0f - pitch_shift_frac) * this->reverb_buffer_L_[pitch_shift_index] +
                             pitch_shift_frac * this->reverb_buffer_L_[(pitch_shift_index + 1) % SHIMMER_BUFFER_SIZE];

        // Calculate the wet (reverb) and dry (original) samples
        float32_t dry_level = 1.0f - this->getWetLevel();
        float32_t wet_sample = dry_level * inL + this->getWetLevel() * pitch_shift_interp;
        float32_t dry_sample = this->getWetLevel() * inL + dry_level * pitch_shift_interp;

        outL = dry_sample;

        // Write the wet sample to the reverb buffer with the diffusion coefficient applied
        this->reverb_buffer_L_[this->reverb_buffer_index_] = wet_sample + (reverb_sample * (1.0f - this->getDiffusion() / this->getSamplingRate() / this->getDecay()));
    }

    // Processing right channel
    {
        // Read the sample from the reverb buffer
        float32_t reverb_sample = this->reverb_buffer_R_[this->reverb_buffer_index_];

        // Calculate the pitch-shifted sample
        float32_t pitch_shift_sample = reverb_sample * this->getPitchShift();

        // Interpolate the pitch-shifted sample to the original pitch
        float32_t pitch_shift_frac = pitch_shift_sample - std::floor(pitch_shift_sample);
        unsigned pitch_shift_index = static_cast<unsigned>(SHIMMER_BUFFER_SIZE + std::floor(pitch_shift_sample)) % SHIMMER_BUFFER_SIZE;
        float32_t pitch_shift_interp = 
                    (1.0f - pitch_shift_frac) * this->reverb_buffer_R_[pitch_shift_index] +
                             pitch_shift_frac * this->reverb_buffer_R_[(pitch_shift_index + 1) % SHIMMER_BUFFER_SIZE];

        // Calculate the wet (reverb) and dry (original) samples
        float32_t dry_level = 1.0f - this->getWetLevel();
        float32_t wet_sample = dry_level * inR + this->getWetLevel() * pitch_shift_interp;
        float32_t dry_sample = this->getWetLevel() * inR + dry_level * pitch_shift_interp;

        outR = dry_sample;

        // Write the wet sample to the reverb buffer with the diffusion coefficient applied
        this->reverb_buffer_R_[this->reverb_buffer_index_] = wet_sample + (reverb_sample * (1.0f - this->getDiffusion() / this->getSamplingRate() / this->getDecay()));
    }

    // Increment the buffer index and wrap around if necessary
    this->reverb_buffer_index_ = (this->reverb_buffer_index_ + 1) % SHIMMER_BUFFER_SIZE;    
}

void ShimmerReverb2::setDecay(float32_t decay) 
{
    this->decay_  = constrain(decay, SHIMMER_MIN_DECAY, SHIMMER_MAX_DECAY);
}

float32_t ShimmerReverb2::getDecay() const 
{
    return this->decay_;
}

void ShimmerReverb2::setDiffusion(float32_t diffusion) 
{
    this->diffusion_ = constrain(diffusion, 0.0f, 1.0f);
}

float32_t ShimmerReverb2::getDiffusion() const 
{
    return this->diffusion_;
}

void ShimmerReverb2::setPitchShift(float32_t pitch_shift) 
{
    this->pitch_shift_ = constrain(pitch_shift, SHIMMER_MIN_PITCH_RATIO, SHIMMER_MAX_PITCH_RATIO);
}

float32_t ShimmerReverb2::getPitchShift() const 
{
    return this->pitch_shift_;
}
