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

#include "fx.h"

class PhaserStage;

class PhaserParameter : public FXBase
{
    friend class PhaserStage;
    DISALLOW_COPY_AND_ASSIGN(PhaserParameter);

public:
    PhaserParameter(float32_t sampling_rate, float32_t frequency = 0.5f, float32_t q = 1.0f);
    virtual ~PhaserParameter();

    void setFrequency(float32_t frequency);
    inline float32_t getFrequency() const;

    void setQ(float32_t q);
    inline float32_t getQ() const;

private:
    void computeCoefficients();

    float32_t frequency_;           // LFO frequency in Hz
    float32_t q_;                   // Q factor for the filters

    float32_t a0, a1, a2, b1, b2;   // Coefficients for the stage's filter
};

class PhaserStage : public FXBase
{
    DISALLOW_COPY_AND_ASSIGN(PhaserStage);

public:
    PhaserStage(float32_t sampling_rate, PhaserParameter* params);

    void process(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR);

private:
    PhaserParameter* params_;   // All paremters of the phaser including the inner coefficients
    float32_t z1[2], z2[2];     // State variables for the stage's filter
};

#define NUM_PHASER_STAGES 6

class Phaser : public FX
{
    DISALLOW_COPY_AND_ASSIGN(Phaser);

public:
    Phaser(float32_t sampling_rate, float32_t frequency = 0.5f, float32_t q = 1.0f);
    virtual ~Phaser();

    virtual void process(float32_t* left_input, float32_t* right_input, float32_t* left_output, float32_t* right_output, size_t nSamples) override;

    void setFrequency(float32_t frequency);
    inline float32_t getFrequency() const;

    void setQ(float32_t q);
    inline float32_t getQ() const;

private:
    PhaserParameter params_;
    float32_t phase_;           // Current phase of the LFO
    float32_t phase_increment_; // Amount to increment the phase at each sample
    PhaserStage* stages_[NUM_PHASER_STAGES];
};