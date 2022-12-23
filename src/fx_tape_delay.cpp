#include "fx_tape_delay.h"

#include <cmath>
#include <algorithm>

TapeDelay::TapeDelay(const float32_t sampling_rate, float32_t default_delay_time, float32_t default_flutter_level, float32_t default_feedback_level) :
    FX(sampling_rate),
    MaxSampleDelayTime(sampling_rate * MAX_DELAY_TIME),
    left_read_pos_(0),
    right_read_pos_(0)
{
    this->left_buffer_ = new float32_t[this->MaxSampleDelayTime];
    this->right_buffer_ = new float32_t[this->MaxSampleDelayTime];

    this->setDelayTime(default_delay_time);
    this->setFlutterLevel(default_flutter_level);
    this->setFeedbakLevel(default_feedback_level);
}

TapeDelay::~TapeDelay()
{
    delete[] this->left_buffer_;
    delete[] this->right_buffer_;
}

void TapeDelay::process(float32_t* left_input, float32_t* right_input, float32_t* left_output, float32_t* right_output, size_t nSamples)
{
    for(size_t i = 0; i < nSamples; ++i)
    {
        // calculate the fluttered delay time
        float32_t fluttered_delay_time = this->getDelayTime() + this->getFlutteredDelayTime();

        // Calculate write positions
        int left_write_pos = this->left_read_pos_ - static_cast<int>(fluttered_delay_time);
        while(left_write_pos < 0)
        {
            left_write_pos += this->MaxSampleDelayTime;
        }

        int right_write_pos = this->right_read_pos_ - static_cast<int>(fluttered_delay_time);
        while(right_write_pos < 0)
        {
            right_write_pos += this->MaxSampleDelayTime;
        }

        // Write input to delay buffers
        this->left_buffer_[left_write_pos] = *left_input;
        this->right_buffer_[right_write_pos] = *right_input;

        // Read from delay buffers and apply feedback
        *left_output = this->left_buffer_[this->left_read_pos_];
        *right_output = this->right_buffer_[this->right_read_pos_];
        this->left_buffer_[left_write_pos] += *left_output * this->getFeedbackLevel();
        this->right_buffer_[right_write_pos] += *right_output * this->getFeedbackLevel();

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

        // Move to next input sample
        ++left_input;
        ++right_input;

        // Move to next output sample
        ++left_output;
        ++right_output;
    }
}

void TapeDelay::setDelayTime(float32_t delay_time)
{
    this->delay_time_ = constrain(delay_time, 0.0f, 1.0f);
}

inline float32_t TapeDelay::getDelayTime() const
{
    return this->delay_time_;
}

void TapeDelay::setFlutterLevel(float32_t flutter_level)
{
    this->flutter_level_ = constrain(flutter_level, 0.0f, 1.0f); 
}

inline float32_t TapeDelay::getFlutterLevel() const
{
    return this->flutter_level_;
}

void TapeDelay::setFeedbakLevel(float32_t feedback)
{
    this->feedback_ = constrain(feedback, 0.0, 1.0);
}

inline float32_t TapeDelay::getFeedbackLevel() const
{
    return this->feedback_;
}

inline float32_t TapeDelay::getFlutteredDelayTime()
{
    // Genarate a random number in the range [-1.0 , 1.0]
    float32_t r = 
        static_cast<float32_t>(this->random_generator_()) /
        static_cast<float32_t>(this->random_generator_.max());

    // Scale and bias the random number to the desired flutter range
    return r * this->getFlutterLevel();
}
