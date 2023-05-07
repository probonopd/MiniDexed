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
// fx_shimmer_reverb.h
//
// Stereo ShimmerReverb Reverb proposed in the context of the MiniDexed project 
// It is adapted from the ShimmerReverb Reverb that could be found on Cloud EuroRack module from Mutable Instrruments
// Ported by: Vincent Gauché
//
#pragma once

#include "fx_components.h"
#include "fx_svf.h"
#include "fx_shimmer_helper.h"
#include "fx_pitch_shifter.h"
#include "fx_reverberator.h"

class ShimmerReverb : public FXElement
{
    DISALLOW_COPY_AND_ASSIGN(ShimmerReverb);

public:
    ShimmerReverb(float32_t sampling_rate);
    virtual ~ShimmerReverb();

    virtual void reset() override;
    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;

    void setInputGain(float32_t input_gain);
    float32_t getInputGain() const;

    void setDiffusion(float32_t diffusion);
    float32_t getDiffusion() const;

    void setTime(float32_t time);
    float32_t getTime() const;

    void setReverbAmount(float32_t amount);

    void setTexture(float32_t texture);
    float32_t getTexture() const;

    void setFeedback(float32_t feedback);
    float32_t getFeedback() const;

    void setPitch(float32_t pitch);
    float32_t getPitch() const;

    void setSize(float32_t size);
    float32_t getSize() const;

    void setCutoff(float32_t cutoff);
    float32_t getCutoff() const;

private:
    void updateFilterCoefficients();
    void updateReverberatorCoefficients();

    PitchShifter pitch_shifter_;
    SVF lp_filter_;
    SVF hp_filter_;
    Reverberator reverberator_;

    float32_t texture_;
    float32_t lp_cutoff_;
    float32_t hp_cutoff_;
    float32_t lpq_;
    float32_t amount_;
    float32_t feedback_;
    float32_t cutoff_;
    
    IMPLEMENT_DUMP(
        out << "START " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;

        out << "END " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;
    )

    IMPLEMENT_INSPECT(
        size_t nb_errors = 0u;

        nb_errors += inspector(tag + ".texture_", this->texture_, 0.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".lp_cutoff_", this->lp_cutoff_, 0.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".hp_cutoff_", this->hp_cutoff_, 0.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".lpq_", this->lpq_, 0.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".amount_", this->amount_, 0.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".feedback_", this->feedback_, 0.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".cutoff_", this->cutoff_, 0.0f, 1.0f, deepInspection);

        if(deepInspection)
        {
            nb_errors += this->pitch_shifter_.inspect(inspector, deepInspection, tag + ".pitch_shifter_");
            nb_errors += this->lp_filter_.inspect(inspector, deepInspection, tag + ".lp_filter_");
            nb_errors += this->hp_filter_.inspect(inspector, deepInspection, tag + ".hp_filter_");
            nb_errors += this->reverberator_.inspect(inspector, deepInspection, tag + ".reverberator_");
        }

        return nb_errors;
    )
};
