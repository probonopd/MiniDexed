#include "fx_shimmer_reverb.h"

#include <cmath>
#include <algorithm>

#define SHIMMER_MAX_DELAY_TIME 2.0f

ShimmerReverb::ShimmerReverb(float32_t sampling_rate,
                             float32_t left_delay_time,
                             float32_t right_delay_time,
                             float32_t shimmer_frequency,
                             float32_t shimmer_amplitude,
                             float32_t decay_time) : FXElement(sampling_rate),
                                                     DelayLineLength(static_cast<unsigned>(2.0f * SHIMMER_MAX_DELAY_TIME * sampling_rate)),
                                                     write_pos_L_(0),
                                                     write_pos_R_(0),
                                                     shimmer_phase_(0.0f)
{
    this->delay_line_L_ = new float32_t[this->DelayLineLength];
    this->delay_line_R_ = new float32_t[this->DelayLineLength];

    memset(this->delay_line_L_, 0, this->DelayLineLength * sizeof(float32_t));
    memset(this->delay_line_R_, 0, this->DelayLineLength * sizeof(float32_t));

    this->setLeftDelayTime(left_delay_time);
    this->setRightDelayTime(right_delay_time);
    this->setShimmerFrequency(shimmer_frequency);
    this->setShimmerAmplitude(shimmer_amplitude);
    this->setDecayTime(decay_time);
}

ShimmerReverb::~ShimmerReverb()
{
    delete[] this->delay_line_L_;
    delete[] this->delay_line_R_;
}

void ShimmerReverb::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    const static float32_t M2PI = 2.0f * PI;

    // Calculate shimmer offset based on current phase
    float32_t shimmerOffsetL = this->getShimmerAmplitude() * sin(this->shimmer_phase_);
    float32_t shimmerOffsetR = this->getShimmerAmplitude() * cos(this->shimmer_phase_);

    // Calculate read position for left and right channel delay lines
    unsigned readPosL = static_cast<unsigned>(this->DelayLineLength + this->write_pos_L_ - (this->delay_time_L_ + shimmerOffsetL) * this->getSamplingRate()) % this->DelayLineLength;
    unsigned readPosR = static_cast<unsigned>(this->DelayLineLength + this->write_pos_R_ - (this->delay_time_R_ + shimmerOffsetR) * this->getSamplingRate()) % this->DelayLineLength;

    // Read32_t left and right channel delay line samples
    float32_t delaySampleL = this->delay_line_L_[readPosL];
    float32_t delaySampleR = this->delay_line_R_[readPosR];

    // Calculate reverb decay factor
    float32_t decay = std::pow(0.001f, 1.0f / (this->decay_time_ * this->getSamplingRate()));

    // Calculate output samples
    outL = inL + delaySampleL * decay;
    outR = inR + delaySampleR * decay;
    
    // Write input samples to delay lines
    this->delay_line_L_[this->write_pos_L_] = outL;
    this->delay_line_R_[this->write_pos_R_] = outR;

    // Increment write position and wrap around the end of the delay line if necessary
    this->write_pos_L_ = (this->write_pos_L_ + 1) % this->DelayLineLength;
    this->write_pos_R_ = (this->write_pos_R_ + 1) % this->DelayLineLength;

    // Increment shimmer phase
    this->shimmer_phase_ += this->shimmer_phase_increment_;
    if(this->shimmer_phase_ > M2PI) 
    {
        this->shimmer_phase_ -= M2PI;
    }
}

void ShimmerReverb::setLeftDelayTime(float32_t delay_time_L) 
{
    this->delay_time_L_ = constrain(delay_time_L, 0.0f, 1.0f);
}

float32_t ShimmerReverb::getLeftDelayTime() const 
{
    return this->delay_time_L_;
}

void ShimmerReverb::setRightDelayTime(float32_t delay_time_R) 
{
    this->delay_time_R_ = constrain(delay_time_R, 0.0f, 1.0f);
}

float32_t ShimmerReverb::getRightDelayTime() const 
{
    return this->delay_time_R_;
}

void ShimmerReverb::setShimmerFrequency(float32_t frequency) 
{
    this->shimmer_frequency_ = constrain(frequency, 0.0f, 1.0f);
    this->shimmer_phase_increment_ = Constants::M2PI * mapfloat(this->shimmer_frequency_, 0.0f, 1.0f, 20.0f, 20000.0f) / this->getSamplingRate();
}

float32_t ShimmerReverb::getShimmerFrequency() const 
{
    return this->shimmer_frequency_;
}

void ShimmerReverb::setShimmerAmplitude(float32_t amplitude) 
{
    this->shimmer_amplitude_ = constrain(amplitude, 0.0f, 1.0f);
}

float32_t ShimmerReverb::getShimmerAmplitude() const 
{
    return this->shimmer_amplitude_;
}

void ShimmerReverb::setDecayTime(float32_t decay_time) 
{
    this->decay_time_ = constrain(decay_time, 0.0f, 1.0f);
}

float32_t ShimmerReverb::getDecayTime() const 
{
    return this->decay_time_;
}
