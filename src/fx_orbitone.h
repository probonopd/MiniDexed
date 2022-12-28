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
// fx_orbitone.h
//
// Stereo Orbitone audio effects proposed in the context of the MiniDexed project
//
#pragma once

#include "fx.h"

class OrbitoneStage;

class OrbitoneParameter : public FXBase
{
    friend class OrbitoneStage;
    DISALLOW_COPY_AND_ASSIGN(OrbitoneParameter);

public:
    OrbitoneParameter(float32_t sampling_rate, float32_t feedback = 0.5f);
    virtual ~OrbitoneParameter();

    void setFeedback(float32_t feedback);
    inline float32_t getFeedback() const;

private:
    float32_t feedback_;        // Amount of feedback to apply to the stage's input
};

class OrbitoneStage : public FXElement
{
    DISALLOW_COPY_AND_ASSIGN(OrbitoneStage);

public:
    OrbitoneStage(float32_t sampling_rate, OrbitoneParameter* params, float32_t frequency);

    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;

private:
    OrbitoneParameter* params_;
    float32_t frequency_;       // Frequency of the stage oscillator in Hz
    float32_t phase_;           // Phase of the stage's oscillator
    float32_t phase_increment_; // Amount to increment the phase at each sample
};

#define NUM_ORBITONR_STAGES 4

class Orbitone : public FXElement
{
    DISALLOW_COPY_AND_ASSIGN(Orbitone);

public:
    Orbitone(float32_t sampling_rate, float32_t feedback = 0.5f);
    virtual ~Orbitone();

    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;

    void setFeedback(float32_t feedback);
    float32_t getFeedback() const;

private:
    OrbitoneParameter params_;
    OrbitoneStage* stages_[NUM_ORBITONR_STAGES];
};