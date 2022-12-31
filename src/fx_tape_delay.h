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

//
// fx_tape_delay.h
//
// Stereo Tape Delay proposed in the context of the MiniDexed project
//
#pragma once

#include "fx.h"
#include "fx_svf.h"

#include <random>

class TapeDelay : public FXElement
{
    DISALLOW_COPY_AND_ASSIGN(TapeDelay);

    class LowHighPassFilter : public FXElement
    {
        DISALLOW_COPY_AND_ASSIGN(LowHighPassFilter);

    public:
        LowHighPassFilter(float32_t sampling_rate);
        virtual ~LowHighPassFilter();

        virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;

        void setCutoffChangeRatio(float32_t ratio);
        
    private:
        // void processSampleLPF(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR);
        // void processSampleHPF(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR);


        // float32_t a0_lpf_;
        // float32_t a1_lpf_;
        // float32_t a2_lpf_;
        // float32_t b1_lpf_;
        // float32_t b2_lpf_;
        // float32_t x1_lpf_[2];
        // float32_t x2_lpf_[2];
        // float32_t y1_lpf_[2];
        // float32_t y2_lpf_[2];

        // float32_t a0_hpf_;
        // float32_t a1_hpf_;
        // float32_t a2_hpf_;
        // float32_t b1_hpf_;
        // float32_t b2_hpf_;
        // float32_t x1_hpf_[2];
        // float32_t x2_hpf_[2];
        // float32_t y1_hpf_[2];
        // float32_t y2_hpf_[2];

        StateVariableFilter lpf_;
        StateVariableFilter hpf_;
    };


public:
    TapeDelay(const float32_t sampling_rate, float32_t default_delay_time = 0.25f, float32_t default_flutter_level = 1.0f, float32_t default_wet_level = 0.5f);
    virtual ~TapeDelay();

    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;

    void setLeftDelayTime(float32_t delay_time);
    float32_t getLeftDelayTime() const;

    void setRightDelayTime(float32_t delay_time);
    float32_t getRightDelayTime() const;

    void setFlutterLevel(float32_t flutter_level);
    float32_t getFlutterLevel() const;

    void setFeedbakLevel(float32_t feedback);
    float32_t getFeedbackLevel() const;

private:
    inline float32_t getFlutteredDelayTime();

private:
    const size_t MaxSampleDelayTime;
    size_t read_pos_L_;
    size_t read_pos_R_;
    float32_t* buffer_L_;
    float32_t* buffer_R_;
    float32_t delay_time_L_;        // Left delay time in seconds (0.0 - 2.0)
    float32_t delay_time_R_;        // Right delay time in seconds (0.0 - 2.0)
    float32_t flutter_level_;       // Flutter level (0.0 - 0.1)
    float32_t feedback_;            // Feedback (0.0 - 1.0)

    LowHighPassFilter filter_;

    std::random_device                          rnd_device_;
    std::mt19937                                rnd_generator_;
    std::uniform_real_distribution<float32_t>   rnd_distribution_;
};
