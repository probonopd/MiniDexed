#include "fx_tape_delay.h"

#include <cmath>

#define MAX_DELAY_TIME 1.0f
#define MAX_FLUTTER_DELAY_TIME 0.01f

#define LPF_CUTOFF_REF 14000.0f
#define HPF_CUTOFF_REF 60.0f

TapeDelay::LowHighPassFilter::LowHighPassFilter(float32_t sampling_rate) :
    FXElement(sampling_rate),
    lpf_(sampling_rate, StateVariableFilter::Type::LPF, LPF_CUTOFF_REF),
    hpf_(sampling_rate, StateVariableFilter::Type::HPF, HPF_CUTOFF_REF)
{
    this->setCutoffChangeRatio(0.0f);
}

TapeDelay::LowHighPassFilter::~LowHighPassFilter()
{
}

void TapeDelay::LowHighPassFilter::setCutoffChangeRatio(float32_t ratio)
{
    ratio += 1.0f;

    this->lpf_.setCutoff(LPF_CUTOFF_REF * ratio);
    this->hpf_.setCutoff(HPF_CUTOFF_REF * ratio);
}

void TapeDelay::LowHighPassFilter::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    this->lpf_.processSample(inL, inR, outL, outR);
    this->hpf_.processSample(outL, outR, outL, outR);
}

TapeDelay::TapeDelay(const float32_t sampling_rate, float32_t default_delay_time, float32_t default_flutter_level, float32_t default_feedback_level) :
    FXElement(sampling_rate),
    MaxSampleDelayTime((MAX_DELAY_TIME + MAX_FLUTTER_DELAY_TIME) * sampling_rate * MAX_DELAY_TIME),
    read_pos_L_(0),
    read_pos_R_(0),
    filter_(sampling_rate),
    jitter_generator_(sampling_rate)
{
    this->buffer_L_ = new float32_t[this->MaxSampleDelayTime];
    this->buffer_R_ = new float32_t[this->MaxSampleDelayTime];

    memset(this->buffer_L_, 0, this->MaxSampleDelayTime * sizeof(float32_t));
    memset(this->buffer_R_, 0, this->MaxSampleDelayTime * sizeof(float32_t));

    this->setLeftDelayTime(default_delay_time);
    this->setRightDelayTime(default_delay_time);
    this->setFlutterLevel(default_flutter_level);
    this->setFeedbak(default_feedback_level);
}

TapeDelay::~TapeDelay()
{
    delete[] this->buffer_L_;
    delete[] this->buffer_R_;
}

void TapeDelay::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    // Calculate the fluttered delay time
    float32_t fluttered_delay_time = this->getFlutteredDelayTime();
    this->filter_.setCutoffChangeRatio(fluttered_delay_time);

    float32_t fluttered_delay_time_L = (MAX_DELAY_TIME * this->getLeftDelayTime()  * (1.0f + fluttered_delay_time)) * this->getSamplingRate();
    float32_t fluttered_delay_time_R = (MAX_DELAY_TIME * this->getRightDelayTime() * (1.0f + fluttered_delay_time)) * this->getSamplingRate();

    // Calculate write positions
    unsigned write_pos_L = static_cast<unsigned>(this->MaxSampleDelayTime + this->read_pos_L_ + fluttered_delay_time_L) % this->MaxSampleDelayTime;
    unsigned write_pos_R = static_cast<unsigned>(this->MaxSampleDelayTime + this->read_pos_R_ + fluttered_delay_time_R) % this->MaxSampleDelayTime;

    // Write input to delay buffers
    this->buffer_L_[write_pos_L] = inL;
    this->buffer_R_[write_pos_R] = inR;

    // Read from delay buffers and apply feedback
    this->filter_.processSample(
        this->buffer_L_[this->read_pos_L_],
        this->buffer_R_[this->read_pos_R_],
        outL,
        outR
    );

    this->buffer_L_[write_pos_L] += outL * this->getFeedbackLevel();
    this->buffer_R_[write_pos_R] += outR * this->getFeedbackLevel();

    // Increment read positions
    ++this->read_pos_L_;
    if(this->read_pos_L_ >= this->MaxSampleDelayTime)
    {
        this->read_pos_L_ -= this->MaxSampleDelayTime;
    }
    ++this->read_pos_R_;
    if(this->read_pos_R_ >= this->MaxSampleDelayTime)
    {
        this->read_pos_R_ -= this->MaxSampleDelayTime;
    }
}

void TapeDelay::setLeftDelayTime(float32_t delay_time)
{
    this->delay_time_L_ = constrain(delay_time, 0.0f, 1.0f);
}

float32_t TapeDelay::getLeftDelayTime() const
{
    return this->delay_time_L_;
}

void TapeDelay::setRightDelayTime(float32_t delay_time)
{
    this->delay_time_R_ = constrain(delay_time, 0.0f, 1.0f);
}

float32_t TapeDelay::getRightDelayTime() const
{
    return this->delay_time_R_;
}

void TapeDelay::setFlutterLevel(float32_t flutter_level)
{
    this->flutter_level_ = constrain(flutter_level, 0.0f, 1.0f);
    this->jitter_generator_.setSpeed(0.2f * (1.0f - this->flutter_level_));
    this->jitter_generator_.setMagnitude(this->flutter_level_ / 100.0f);
}

float32_t TapeDelay::getFlutterLevel() const
{
    return this->flutter_level_;
}

void TapeDelay::setFeedbak(float32_t feedback)
{
    this->feedback_ = constrain(feedback, 0.0, 1.0);
}

float32_t TapeDelay::getFeedbackLevel() const
{
    return this->feedback_;
}

float32_t TapeDelay::getFlutteredDelayTime()
{
    // Genarate a random number in the range [-1.0 , 1.0]
    float32_t r = this->jitter_generator_.process();

    // Scale and bias the random number to the desired flutter range
    return MAX_FLUTTER_DELAY_TIME * r * this->getFlutterLevel();
}
