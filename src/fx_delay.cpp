#include "fx_delay.h"

#include <cmath>

#define MAX_DELAY_TIME 1.0f
#define MAX_FLUTTER_DELAY_TIME 0.2f
#define MAX_FLUTTER_DELAY_AMOUNT 0.01f

#define LPF_CUTOFF_REF 12000.0f
#define HPF_CUTOFF_REF 80.0f

Delay::LowHighPassFilter::LowHighPassFilter(float32_t sampling_rate) :
    FXElement(sampling_rate),
    lpf_(sampling_rate, StateVariableFilter::FilterMode::LPF, LPF_CUTOFF_REF),
    hpf_(sampling_rate, StateVariableFilter::FilterMode::HPF, HPF_CUTOFF_REF),
    ratio_(1.0f)
{
    this->setCutoffChangeRatio(0.0f);
    this->lpf_.setGainDB(0.82f);
    this->hpf_.setGainDB(0.82f);
}

Delay::LowHighPassFilter::~LowHighPassFilter()
{
}

void Delay::LowHighPassFilter::setCutoffChangeRatio(float32_t ratio)
{
    static const float32_t weight = 4.0f;

    ratio = constrain(ratio, -1.0f, 1.0f);
    if(ratio != this->ratio_)
    {
        this->ratio_ = ratio;
        ratio /= 10.0f;
        this->lpf_.setCutoff(LPF_CUTOFF_REF * (1.0f - ratio / weight));
        this->hpf_.setCutoff(HPF_CUTOFF_REF * (1.0f + ratio * weight));
    }
}

void Delay::LowHighPassFilter::reset()
{
    this->lpf_.reset();
    this->hpf_.reset();
}

void Delay::LowHighPassFilter::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    this->lpf_.processSample(inL, inR, outL, outR);
    this->hpf_.processSample(outL, outR, outL, outR);
}

Delay::Delay(const float32_t sampling_rate, float32_t default_delay_time, float32_t default_flutter_level, float32_t default_feedback_level) :
    FXElement(sampling_rate, 2.2587f),
    MaxSampleDelayTime((MAX_DELAY_TIME + MAX_FLUTTER_DELAY_TIME) * sampling_rate),
    write_pos_L_(0),
    write_pos_R_(0),
    filter_(sampling_rate)
{
    this->buffer_L_ = new float32_t[this->MaxSampleDelayTime];
    this->buffer_R_ = new float32_t[this->MaxSampleDelayTime];

    this->setLeftDelayTime(default_delay_time);
    this->setRightDelayTime(default_delay_time);
    this->setFeedback(default_feedback_level);

    this->reset();
}

Delay::~Delay()
{
    delete[] this->buffer_L_;
    delete[] this->buffer_R_;
}

void Delay::reset()
{
    memset(this->buffer_L_, 0, this->MaxSampleDelayTime * sizeof(float32_t));
    memset(this->buffer_R_, 0, this->MaxSampleDelayTime * sizeof(float32_t));
    this->write_pos_L_ = 0;
    this->write_pos_R_ = 0;
    this->filter_.reset();
}

void Delay::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    // Write input to delay buffers
    this->buffer_L_[this->write_pos_L_] *= this->getFeedback();;
    this->buffer_L_[this->write_pos_L_] += inL;
    this->buffer_R_[this->write_pos_R_] *= this->getFeedback();;
    this->buffer_R_[this->write_pos_R_] += inR;

    // Calculate read positions
    float32_t delay_time_L = std::abs(MAX_DELAY_TIME * this->getLeftDelayTime() ) * this->getSamplingRate();
    float32_t delay_time_R = std::abs(MAX_DELAY_TIME * this->getRightDelayTime()) * this->getSamplingRate();
    float32_t signL = this->getLeftDelayTime()  >= 0 ? 1.0f : -1.0f;
    float32_t signR = this->getRightDelayTime() >= 0 ? 1.0f : -1.0f;
    unsigned read_pos_L = static_cast<unsigned>(this->MaxSampleDelayTime + signL * this->write_pos_L_ - delay_time_L) % this->MaxSampleDelayTime;
    unsigned read_pos_R = static_cast<unsigned>(this->MaxSampleDelayTime + signR * this->write_pos_R_ - delay_time_R) % this->MaxSampleDelayTime;


    // Read from delay buffers and apply feedback
    this->filter_.processSample(
        this->buffer_L_[read_pos_L],
        this->buffer_R_[read_pos_R],
        outL,
        outR
    );

    this->buffer_L_[this->write_pos_L_] += outL * this->getFeedback();
    this->buffer_R_[this->write_pos_R_] += outR * this->getFeedback();

    // Increment read positions
    ++ this->write_pos_L_;
    if(this->write_pos_L_ >= this->MaxSampleDelayTime)
    {
        this->write_pos_L_ -= this->MaxSampleDelayTime;
    }
    ++ this->write_pos_R_;
    if(this->write_pos_R_ >= this->MaxSampleDelayTime)
    {
        this->write_pos_R_ -= this->MaxSampleDelayTime;
    }

    outL *= this->OutputLevelCorrector;
    outR *= this->OutputLevelCorrector;
}

void Delay::setLeftDelayTime(float32_t delay_time)
{
    this->delay_time_L_ = constrain(delay_time, -1.0f, 1.0f);
}

float32_t Delay::getLeftDelayTime() const
{
    return this->delay_time_L_;
}

void Delay::setRightDelayTime(float32_t delay_time)
{
    this->delay_time_R_ = constrain(delay_time, -1.0f, 1.0f);
}

float32_t Delay::getRightDelayTime() const
{
    return this->delay_time_R_;
}

void Delay::setFeedback(float32_t feedback)
{
    this->feedback_ = constrain(feedback, 0.0, 1.0);
}

float32_t Delay::getFeedback() const
{
    return this->feedback_;
}
