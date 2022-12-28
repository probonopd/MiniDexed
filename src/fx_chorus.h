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
// fx_chorus.h
//
// Stereo Chorus audio effects proposed in the context of the MiniDexed project
//
#pragma once

#include "fx.h"

class Chorus : public FXElement
{
    DISALLOW_COPY_AND_ASSIGN(Chorus);

public:
    Chorus(float32_t sampling_rate, unsigned voices = 4, float32_t depth = 5.0f, float32_t rate = 0.5f, float32_t feedback = 0.5f);
    virtual ~Chorus();

    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;

    void setDepth(float32_t depth);
    float32_t getDepth() const;

    void setRate(float32_t rate);
    float32_t getRate() const;

    void setFeedback(float32_t feedback);
    float32_t getFeedback() const;

private:
    const unsigned NumVoices;   // Number of voices in the chorus
    const float32_t sample_position_ratio_;
    float32_t** delay_buffersL_;
    float32_t** delay_buffersR_;
    unsigned* delay_buffer_indices_;

    float32_t lfo_phase_;
    float32_t lfo_phase_increment_;

    float32_t depth_;           // Depth of the chorus in milliseconds (0.0 - 10.0)
    float32_t rate_;            // Rate of the chorus in Hz (0.1 - 1.0)
    float32_t feedback_;        // Feedback level of the chorus (0.0 - 1.0)
};
