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

#include <random>

#define MAX_DELAY_TIME 2

class TapeDelay : public FXElement
{
    DISALLOW_COPY_AND_ASSIGN(TapeDelay);
 
public:
    TapeDelay(const float32_t sampling_rate, float32_t default_delay_time = 0.25f, float32_t default_flutter_level = 0.05f, float32_t default_wet_level = 0.5f);
    virtual ~TapeDelay();

    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;

    void setDelayTime(float32_t delay_time);
    inline float32_t getDelayTime() const;

    void setFlutterLevel(float32_t flutter_level);
    inline float32_t getFlutterLevel() const;

    void setFeedbakLevel(float32_t feedback);
    inline float32_t getFeedbackLevel() const;

private:
    inline float32_t getFlutteredDelayTime();

private:
    const size_t MaxSampleDelayTime;
    size_t left_read_pos_;
    size_t right_read_pos_;
    float32_t* left_buffer_;
    float32_t* right_buffer_;
    float32_t delay_time_;
    float32_t flutter_level_;
    float32_t feedback_;
    std::mt19937 random_generator_;
};
