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
// fx_phaser.h
//
// Stereo Phaser audio effects proposed in the context of the MiniDexed project
//
#pragma once

#include "fx_components.h"

class AllpassDelay
{
    DISALLOW_COPY_AND_ASSIGN(AllpassDelay);

public:
    AllpassDelay();
    virtual ~AllpassDelay();

    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR);

    void setDelay(float32_t delay);

private:
    float32_t a1_;
    float32_t z_[2];
};


#define MAX_NB_PHASES 24

class Phaser : public FXElement
{
    DISALLOW_COPY_AND_ASSIGN(Phaser);

public:
    Phaser(float32_t sampling_rate, float32_t rate = 0.5f, float32_t depth = 1.0f, float32_t feedback = 0.7f);
    virtual ~Phaser();

    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;

    void setFrequencyRange(float32_t min_frequency, float32_t max_frequency);

    void setRate(float32_t rate);
    float32_t getRate() const;

    void setDepth(float32_t depth);
    float32_t getDepth() const;

    void setFeedback(float32_t depth);
    float32_t getFeedback() const;

    void setNbStages(unsigned nb_stages);
    unsigned getNbStages() const;

private:
    LFO lfo_;
    float32_t depth_;
    float32_t feedback_;
    float32_t dmin_;
    float32_t dmax_;
    unsigned nb_stages_;
    AllpassDelay stages_[MAX_NB_PHASES];
    float32_t z_[2];
};