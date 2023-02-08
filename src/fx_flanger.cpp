#include "fx_flanger.h"

Flanger::Flanger(float32_t sampling_rate, float32_t rate, float32_t depth, float32_t feedback) :
    FXElement(sampling_rate, 1.17f),
    MaxDelayLineSize(static_cast<unsigned>(MAX_FLANGER_DELAY * sampling_rate)),
    write_index_(0)
{
    this->delay_lineL_ = new float32_t[this->MaxDelayLineSize];
    this->delay_lineR_ = new float32_t[this->MaxDelayLineSize];

    this->lfo_[LFOIndex::LFO_L] = new LFO(sampling_rate, 0.1f, 5.0f, 0.0f, false);
    this->lfo_[LFOIndex::LFO_R] = new LFO(sampling_rate, 0.1f, 5.0f, Constants::MPI_2, false);

    this->setRate(rate);
    this->setDepth(depth);
    this->setFeedback(feedback);

    this->reset();
}

Flanger::~Flanger()
{
    delete[] this->delay_lineL_;
    delete[] this->delay_lineR_;

    for(unsigned i = 0; i < LFOIndex::kLFOCount; ++i)
    {
        delete this->lfo_[i];
    }
}

inline float32_t linearIterpolationnterp(float32_t inX, float32_t inY, float32_t inPhase)
{
	return (1.0f - inPhase) * inX + inPhase * inY;
}

void Flanger::reset()
{
    memset(this->delay_lineL_, 0, this->MaxDelayLineSize * sizeof(float32_t));
    memset(this->delay_lineR_, 0, this->MaxDelayLineSize * sizeof(float32_t));
    memset(this->feedback_samples_, 0, StereoChannels::kNumChannels * sizeof(float32_t));
    this->write_index_ = 0;

    for(unsigned i = 0; i < LFOIndex::kLFOCount; ++i)
    {
        this->lfo_[i]->reset();
    }
}

void Flanger::processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR)
{
    // Write sample and any feedback into delay buffers
    this->delay_lineL_[this->write_index_] = inL + this->feedback_samples_[StereoChannels::Left ];
    this->delay_lineR_[this->write_index_] = inR + this->feedback_samples_[StereoChannels::Right];

    ++this->write_index_;
    if(this->write_index_ >= this->MaxDelayLineSize)
    {
        this->write_index_ -= this->MaxDelayLineSize;
    }

    // Configure LFO for effect processing
    float32_t lfo_l = this->lfo_[LFOIndex::LFO_L]->process() * this->depth_;
    float32_t lfo_r = this->lfo_[LFOIndex::LFO_R]->process() * this->depth_;

    // Map LFO range to millisecond range according to Chorus or Flanger effect
	float32_t lfoMappedL = mapfloat(lfo_l, -1.0f, 1.0f, 0.001f, 0.005f);
	float32_t lfoMappedR = mapfloat(lfo_r, -1.0f, 1.0f, 0.001f, 0.005f);

	// Calculate delay lengths in samples
	float32_t delayTimeSamplesL = this->getSamplingRate() * lfoMappedL;
	float32_t delayTimeSamplesR = this->getSamplingRate() * lfoMappedR;

	// Calculate read head positions
	float32_t delayReadHeadL = this->write_index_ - delayTimeSamplesL;
    if(delayReadHeadL < 0.0f)
    {
        delayReadHeadL += this->MaxDelayLineSize;
    }
	float32_t delayReadHeadR = this->write_index_ - delayTimeSamplesR;
    if(delayReadHeadR < 0.0f)
    {
        delayReadHeadR += this->MaxDelayLineSize;
    }

	// Calculate linear interpolation point for left channel
	int32_t currentL = static_cast<int32_t>(delayReadHeadL);
	int32_t nextL = currentL + 1;
	float32_t fractionL = delayReadHeadL - currentL;
	if(nextL >= static_cast<int>(this->MaxDelayLineSize))
    {
		nextL -= this->MaxDelayLineSize;
    }

	// Calculate linear interpolation point for right channel
	int32_t currentR = static_cast<int32_t>(delayReadHeadR);
	int32_t nextR = currentR + 1;
	float32_t fractionR = delayReadHeadR - currentR;
	if(nextR >= static_cast<int>(this->MaxDelayLineSize))
	{
        nextR -= this->MaxDelayLineSize;
    }

    // Interpolate and read from delay buffer
    float32_t delay_sample_l = linearIterpolationnterp(this->delay_lineL_[currentL], this->delay_lineL_[nextL], fractionL);
    float32_t delay_sample_r = linearIterpolationnterp(this->delay_lineR_[currentR], this->delay_lineR_[nextR], fractionR);

    // Store delayed samples as feedback
    this->feedback_samples_[StereoChannels::Left ] = delay_sample_l * this->feedback_;
    this->feedback_samples_[StereoChannels::Right] = delay_sample_r * this->feedback_;

    outL = delay_sample_l * this->OutputLevelCorrector;
    outR = delay_sample_r * this->OutputLevelCorrector;
}

void Flanger::setRate(float32_t rate)
{
    this->lfo_[LFOIndex::LFO_L]->setNormalizedFrequency(rate);
    this->lfo_[LFOIndex::LFO_R]->setNormalizedFrequency(rate);
}

float32_t Flanger::getRate() const
{
    return this->lfo_[LFOIndex::LFO_L]->getNormalizedFrequency();
}

void Flanger::setDepth(float32_t depth)
{
    this->depth_ = constrain(depth, 0.0f, 1.0f);
}

float32_t Flanger::getDepth() const
{
    return this->depth_;
}

void Flanger::setFeedback(float32_t feedback)
{
    this->feedback_ = constrain(feedback, 0.0f, 0.97f);
}

float32_t Flanger::getFeedback() const
{
    return this->feedback_;
}
