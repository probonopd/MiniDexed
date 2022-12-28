#include "fx_tape_delay.h"

#include <cmath>
#include <algorithm>

TapeDelay::TapeDelay(const float32_t sampling_rate, float32_t default_delay_time, float32_t default_flutter_level, float32_t default_feedback_level) :
    FXElement(sampling_rate),
    MaxSampleDelayTime(2.0f * sampling_rate * MAX_DELAY_TIME),
    left_read_pos_(0),
    right_read_pos_(0)
{
    this->left_buffer_ = new float32_t[this->MaxSampleDelayTime];
    this->right_buffer_ = new float32_t[this->MaxSampleDelayTime];

    this->setLeftDelayTime(default_delay_time);
    this->setRightDelayTime(default_delay_time);
    this->setFlutterLevel(default_flutter_level);
    this->setFeedbakLevel(default_feedback_level);
}

TapeDelay::~TapeDelay()
{
    delete[] this->left_buffer_;
    delete[] this->right_buffer_;
}

void TapeDelay::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    // calculate the fluttered delay time
    float32_t fluttered_delay_time_L = (MAX_DELAY_TIME * this->getLeftDelayTime() + this->getFlutteredDelayTime()) * this->getSamplingRate();
    // Calculate write positions
    int left_write_pos = (this->MaxSampleDelayTime + this->left_read_pos_ - static_cast<int>(fluttered_delay_time_L)) % this->MaxSampleDelayTime;

    // calculate the fluttered delay time
    float32_t fluttered_delay_time_R = (MAX_DELAY_TIME * this->getRightDelayTime() + this->getFlutteredDelayTime()) * this->getSamplingRate();
    // Calculate write positions
    int right_write_pos = (this->MaxSampleDelayTime + this->right_read_pos_ - static_cast<int>(fluttered_delay_time_R)) % this->MaxSampleDelayTime;

    // Write input to delay buffers
    this->left_buffer_[left_write_pos] = inL;
    this->right_buffer_[right_write_pos] = inR;

    // Read from delay buffers and apply feedback
    outL = this->left_buffer_[this->left_read_pos_];
    outR = this->right_buffer_[this->right_read_pos_];
    this->left_buffer_[left_write_pos] += outL * this->getFeedbackLevel();
    this->right_buffer_[right_write_pos] += outR * this->getFeedbackLevel();

    // Increment read positions
    ++this->left_read_pos_;
    if(this->left_read_pos_ >= this->MaxSampleDelayTime)
    {
        this->left_read_pos_ -= this->MaxSampleDelayTime;
    }
    ++this->right_read_pos_;
    if(this->right_read_pos_ >= this->MaxSampleDelayTime)
    {
        this->right_read_pos_ -= this->MaxSampleDelayTime;
    }
}

void TapeDelay::setLeftDelayTime(float32_t delay_time)
{
    this->left_delay_time_ = constrain(delay_time, 0.0f, 1.0f);
}

float32_t TapeDelay::getLeftDelayTime() const
{
    return this->left_delay_time_;
}

void TapeDelay::setRightDelayTime(float32_t delay_time)
{
    this->right_delay_time_ = constrain(delay_time, 0.0f, 1.0f);
}

float32_t TapeDelay::getRightDelayTime() const
{
    return this->right_delay_time_;
}

void TapeDelay::setFlutterLevel(float32_t flutter_level)
{
    this->flutter_level_ = constrain(flutter_level, 0.0f, 0.1f); 
}

float32_t TapeDelay::getFlutterLevel() const
{
    return this->flutter_level_;
}

void TapeDelay::setFeedbakLevel(float32_t feedback)
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
    float32_t r = 
        static_cast<float32_t>(this->random_generator_()) /
        static_cast<float32_t>(this->random_generator_.max());

    // Scale and bias the random number to the desired flutter range
    return r * this->getFlutterLevel();
}
