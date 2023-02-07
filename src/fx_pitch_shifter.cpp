#include "fx_pitch_shifter.h"
#include "fx_shimmer_helper.h" 

#include <cmath>
#include <algorithm>

#define PITCH_SHIFT_BOUND 36.0f

#define ONE_POLE(out, in, coefficient) out += (coefficient) * ((in) - out);

#define TAIL , -1

PitchShifter::PitchShifter(float32_t sampling_rate) : 
    FXElement(sampling_rate),
    engine_(sampling_rate),
    phase_(0.0f),
    transpose_(0.0f),
    ratio_(0.0f),
    size_(-1.0f),
    sample_size_(0.0f)
{
    this->setTranspose(0.0f);
    this->setSize(0.5f);
}

PitchShifter::~PitchShifter()
{
}

void PitchShifter::reset()
{
    this->engine_.reset();
}

void PitchShifter::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    typedef Engine::Reserve<2047, Engine::Reserve<2047> > Memory;
    Engine::DelayLine<Memory, 0> left;
    Engine::DelayLine<Memory, 1> right;
    Engine::Context c;

    this->engine_.start(&c);
    
    this->phase_ += (1.0f - this->ratio_) / this->sample_size_;
    if(this->phase_ >= 1.0f)
    {
      phase_ -= 1.0f;
    }
    if(this->phase_ <= 0.0f)
    {
      this->phase_ += 1.0f;
    }

    float tri = 2.0f * (this->phase_ >= 0.5f ? 1.0f - phase_ : phase_);
    float phase = this->phase_ * this->sample_size_;
    float half = phase + this->sample_size_ * 0.5f;
    if(half >= this->sample_size_)
    {
      half -= this->sample_size_;
    }
    
    c.load(inL);
    c.writeAndLoad(left, 0.0f);
    c.interpolate(left, phase, tri);
    c.interpolate(left, half, 1.0f - tri);
    c.writeAndLoad(outL, 0.0f);

    c.load(inR);
    c.writeAndLoad(right, 0.0f);
    c.interpolate(right, phase, tri);
    c.interpolate(right, half, 1.0f - tri);
    c.writeAndLoad(outR, 0.0f);
}

void PitchShifter::setTranspose(float32_t transpose)
{
    transpose = constrain(transpose, -PITCH_SHIFT_BOUND, PITCH_SHIFT_BOUND);
    if(this->transpose_ != transpose)
    {
        this->transpose_ = transpose;
        this->ratio_ = semitoneToRatio(transpose);
    }
}

float32_t PitchShifter::getTranspose() const
{
    return this->transpose_;
}

void PitchShifter::setSize(float32_t size)
{
    size = constrain(size, 0.0f, 1.0f);
    if(size != this->size_)
    {
        this->size_ = size;

        float32_t target_size = 128.0f + (2047.0f - 128.0f) * size * size * size;
        ONE_POLE(this->sample_size_, target_size, 0.05f);
    }
}

float32_t PitchShifter::getSize() const
{
    return this->size_;
}