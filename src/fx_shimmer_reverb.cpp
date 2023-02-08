#include "fx_shimmer_reverb.h"

#define TAIL , -1

ShimmerReverb::ShimmerReverb(float32_t sampling_frequency) : 
    FXElement(sampling_frequency, 1.2f),
    pitch_shifter_(sampling_frequency, PITCH_SHIFTER_TRANSPOSE_BOUNDARY),
    lp_filter_(sampling_frequency, SVF::FilterMode::SVF_LP),
    hp_filter_(sampling_frequency, SVF::FilterMode::SVF_HP),
    reverberator_(sampling_frequency),
    texture_(0.0f),
    lp_cutoff_(0.0f),
    hp_cutoff_(0.0f),
    lpq_(0.0f),
    amount_(0.0f),
    feedback_(0.0f),
    cutoff_(0.0f)
{
    this->setInputGain(0.2f);
    this->setDiffusion(0.7f);
    this->setCutoff(1.0f);

    this->reset();
}

ShimmerReverb::~ShimmerReverb()
{
}

void ShimmerReverb::reset()
{
    this->pitch_shifter_.reset();
    this->lp_filter_.reset();
    this->hp_filter_.reset();
    this->reverberator_.reset();
}

void ShimmerReverb::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    this->pitch_shifter_.processSample(inL, inR, outL, outR);
    this->lp_filter_.processSample(outL, outR, outL, outR);
    this->hp_filter_.processSample(outL, outR, outL, outR);
    this->reverberator_.processSample(outL, outR, outL, outR);

    outL *= this->OutputLevelCorrector;
    outR *= this->OutputLevelCorrector;
}

void ShimmerReverb::setInputGain(float32_t input_gain)
{
    this->reverberator_.setInputGain(input_gain);
}

float32_t ShimmerReverb::getInputGain() const
{
    return this->reverberator_.getInputGain();
}

void ShimmerReverb::setDiffusion(float32_t diffusion)
{
    this->reverberator_.setDiffusion(diffusion);
}

float32_t ShimmerReverb::getDiffusion() const
{
    return this->reverberator_.getDiffusion();
}

void ShimmerReverb::setTime(float32_t time)
{
    this->reverberator_.setTime(time);
}

float32_t ShimmerReverb::getTime() const
{
    return this->reverberator_.getTime();
}

void ShimmerReverb::setReverbAmount(float32_t amount)
{
    amount = constrain(amount, 0.0f, 1.0f);
    if(this->amount_ != amount)
    {
        this->amount_ = amount;
        this->updateReverberatorCoefficients();
    }
}

void ShimmerReverb::setTexture(float32_t texture)
{
    texture = constrain(texture, 0.0f, 1.0f);
    if(this->texture_ != texture)
    {
        this->texture_ = texture;
        this->updateFilterCoefficients();
    }
}

float32_t ShimmerReverb::getTexture() const
{
    return this->texture_;
}

void ShimmerReverb::setFeedback(float32_t feedback)
{
    feedback = constrain(feedback, 0.0f, 1.0f);
    if(this->feedback_ != feedback)
    {
        this->feedback_ = feedback;
        this->updateFilterCoefficients();
        this->updateReverberatorCoefficients();
    }
}

float32_t ShimmerReverb::getFeedback() const
{
    return this->feedback_;
}

void ShimmerReverb::setCutoff(float32_t cutoff)
{
    cutoff = constrain(cutoff, 0.0f, 1.0f);
    if(this->cutoff_ != cutoff)
    {
        this->cutoff_ = cutoff;
        this->updateFilterCoefficients();
    }
}

void ShimmerReverb::updateFilterCoefficients()
{
    this->lp_cutoff_ = constrain(0.50f * semitoneToRatio((this->cutoff_ < 0.5f ? this->cutoff_ - 0.5f : 0.0f ) * 216.0f), 0.0f, 0.499f);
    this->hp_cutoff_ = constrain(0.25f * semitoneToRatio((this->cutoff_ < 0.5f ? -0.5f : this->cutoff_ - 1.0f) * 216.0f), 0.0f, 0.499f);
    this->lpq_ = 1.0f + 3.0f * (1.0f - this->feedback_) * (0.5f - this->lp_cutoff_);

    this->lp_filter_.setFQ<SVF::FrequencyApproximation::FrequencyFast>(this->lp_cutoff_, this->lpq_);
    this->hp_filter_.setFQ<SVF::FrequencyApproximation::FrequencyFast>(this->hp_cutoff_, 1.0f);

    this->reverberator_.setLP(0.6f + 0.37f * this->feedback_);
}

void ShimmerReverb::updateReverberatorCoefficients()
{
    float32_t reverb_amount = this->amount_ * 0.95f;
    reverb_amount += this->feedback_ * (2.0f - this->feedback_);
    reverb_amount = constrain(reverb_amount, 0.0f, 1.0f);

    this->setTime(0.35f + 0.63f * reverb_amount);
}

void ShimmerReverb::setPitch(float32_t pitch)
{
    this->pitch_shifter_.setTranspose(pitch);
}

float32_t ShimmerReverb::getPitch() const
{
    return this->pitch_shifter_.getTranspose();
}

void ShimmerReverb::setSize(float32_t size)
{
    this->pitch_shifter_.setSize(size);
}

float32_t ShimmerReverb::getSize() const
{
    return this->pitch_shifter_.getSize();
}
