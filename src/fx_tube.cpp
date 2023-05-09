#include "fx_tube.h"

#include <cmath>

Tube::Tube(float32_t samplingRate) :
    FXElement(samplingRate),
    overdrive_(1.0f),
    saturator_factor_(1.0f),
    gain_factor_(1.0f)
{
    this->setOverdrive(0.0f);
}

Tube::~Tube()
{
}

void Tube::reset()
{
    // nothing to be done
}

void Tube::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    if(inL == 0.0f)
    {
        outL = 0.0f;
    }
    else
    {
        outL = std::tanh(this->saturator_factor_ * inL) * this->gain_factor_;
    }

    if(inR == 0.0f)
    {
        outR = 0.0f;
    }
    else
    {
        outR = std::tanh(this->saturator_factor_ * inR) * this->gain_factor_;
    }
}

void Tube::setOverdrive(float32_t overdrive)
{
    static const float32_t N = 3.0f;

    overdrive = constrain(overdrive, 0.0f, 1.0f);
    if(this->overdrive_ != overdrive)
    {
        this->overdrive_ = overdrive;
        this->saturator_factor_ = 1.0 + N * overdrive;
        this->gain_factor_ = this->OutputLevelCorrector / std::tanh(this->saturator_factor_);
    }
}

float32_t Tube::getOverdrive() const
{
    return this->overdrive_;
}
