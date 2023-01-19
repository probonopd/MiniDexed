#include "fx_tube.h"

#include <cmath>

Tube::Tube(float32_t samplingRate) :
    FXElement(samplingRate),
    overdrive_(0.0f),
    saturator_factor_(0.0f)
{
    this->setOverdrive(0.0f);
}

Tube::~Tube()
{
}

void Tube::reset()
{
    // does nothing
}

void Tube::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    outL = softSaturator4(inL, this->saturator_factor_);
    outR = softSaturator4(inR, this->saturator_factor_);
}

void Tube::setOverdrive(float32_t overdrive)
{
    static const float32_t N = 200.0f;

    overdrive = constrain(overdrive, 0.0f, 1.0f);
    if(this->overdrive_ != overdrive)
    {
        this->overdrive_ = overdrive;
        this->saturator_factor_ = 1.0f + N * overdrive;
    }
}

float32_t Tube::getOverdrive() const
{
    return this->overdrive_;
}
