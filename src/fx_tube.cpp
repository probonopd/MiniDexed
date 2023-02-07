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
    float32_t x = inL * this->saturator_factor_;
    float32_t abs_x = abs(x);
    float32_t sat_x = log(1.0f + abs_x) * this->gain_factor_;
    
    outL = inL > 0 ? sat_x : -sat_x;

    x = inR * this->saturator_factor_;
    abs_x = abs(x);
    sat_x = log(1.0f + abs_x) * this->gain_factor_;

    outR = inR > 0 ? sat_x : -sat_x;
}

void Tube::setOverdrive(float32_t overdrive)
{
    static const float32_t N = 200.0f;

    overdrive = constrain(overdrive, 0.0f, 1.0f);
    if(this->overdrive_ != overdrive)
    {
        this->overdrive_ = overdrive;
        this->saturator_factor_ = 1.0f + N * overdrive;
        this->gain_factor_ = this->OutputLevelCorrector / log(1.0f + this->saturator_factor_);
    }
}

float32_t Tube::getOverdrive() const
{
    return this->overdrive_;
}
