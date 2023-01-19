#include "fx_components.h"

#include <cmath>

///////////////////////////////
// Constants implemlentation //
///////////////////////////////
const float32_t Constants::M2PI = 2.0f * PI;
const float32_t Constants::MPI_2 = PI / 2.0f;
const float32_t Constants::MPI_3 = PI / 3.0f;
const float32_t Constants::MPI_4 = PI / 4.0f;
const float32_t Constants::M1_PI = 1.0f / PI;

/////////////////////////
// LFO implemlentation //
/////////////////////////
LFO::LFO(float32_t sampling_rate, Waveform waveform, float32_t min_frequency, float32_t max_frequency, float32_t initial_phase) :
    FXBase(sampling_rate),
    InitialPhase(initial_phase),
    min_frequency_(min_frequency),
    max_frequency_(max_frequency),
    waveform_(waveform),
    normalized_frequency_(-1.0f),
    frequency_(0.0f),
    phase_(initial_phase),
    phase_increment_(0.0f),
    current_sample_(0.0f),
    new_phase_(true),
    rnd_generator_(rnd_device_()),
    rnd_distribution_(-1.0f, 1.0f)
{
    this->setWaveform(waveform);
    this->setFrequency(this->min_frequency_);
}

LFO::~LFO()
{
}

void LFO::setWaveform(Waveform waveform)
{
    this->waveform_ = waveform;
}

LFO::Waveform LFO::getWaveform() const
{
    return this->waveform_;
}

void LFO::setNormalizedFrequency(float32_t normalized_frequency)
{
    normalized_frequency = constrain(normalized_frequency, 0.0f, 1.0f);
    if(this->normalized_frequency_ != normalized_frequency)
    {
        float32_t frequency = mapfloat(normalized_frequency, 0.0f, 1.0f, this->min_frequency_, this->max_frequency_);
        this->normalized_frequency_ = normalized_frequency;
        this->frequency_ = frequency;
        this->phase_increment_ = Constants::M2PI * this->frequency_ / this->getSamplingRate();
    }
}

float32_t LFO::getNormalizedFrequency() const
{
    return this->normalized_frequency_;
}

void LFO::setFrequency(float32_t frequency)
{
    frequency = constrain(frequency, this->min_frequency_, this->max_frequency_);
    if(this->frequency_ != frequency)
    {
        float32_t normalized_frequency = mapfloat(frequency, this->min_frequency_, this->max_frequency_, 0.0f, 1.0f);
        this->normalized_frequency_ = normalized_frequency;
        this->frequency_ = frequency;
        this->phase_increment_ = Constants::M2PI * this->frequency_ / this->getSamplingRate();
    }
}

float32_t LFO::getFrequency() const
{
    return this->frequency_;
}

void LFO::reset()
{
    this->phase_ = this->InitialPhase;
    this->current_sample_ = 0.0f;
}

float32_t LFO::process()
{
    float32_t out = 0.0f;
    switch(this->waveform_)
    {
    case Waveform::Sine:
        out = arm_sin_f32(this->phase_);
        break;
    case Waveform::Saw:
        out = Constants::M1_PI * this->phase_ - 1.0f;
        break;
    case Waveform::Square:
        out = this->phase_ < PI ? 1.0 : -1.0;
        break;
    case Waveform::SH:
        if(this->new_phase_)
        {
            out = this->rnd_distribution_(this->rnd_generator_);
        }
        else
        {
            out = this->current_sample_;
        }
        break;
    case Waveform::Noise:
        out = this->rnd_distribution_(this->rnd_generator_);
        break;
    }

    this->current_sample_ = out;

    this->phase_ += this->phase_increment_;
    if(this->phase_ >= Constants::M2PI)
    {
        this->phase_ -= Constants::M2PI;
        this->new_phase_ = true;
    }
    else
    {
        this->new_phase_ = false;
    }

    return out;
}

float32_t LFO::current() const
{
    return this->current_sample_;
}


////////////////////////////////////
// JitterGenerator implementation //
////////////////////////////////////
JitterGenerator::JitterGenerator(float32_t sampling_rate) :
    FXBase(sampling_rate),
    rnd_generator_(rnd_device_()),
    rnd_distribution_(-1.0f, 1.0f),
    speed_(-1.0f),
    magnitude_(-1.0f),
    phase_(0.0f),
    phase_increment_(0.0f)
{
    this->setSpeed(1.0f);
    this->setMagnitude(0.1f);
}

JitterGenerator::~JitterGenerator()
{
}

void JitterGenerator::setSpeed(float32_t speed)
{
    if(this->speed_ != speed)
    {
        this->speed_ = speed;
        this->phase_increment_ = Constants::M2PI * this->speed_ / this->getSamplingRate();
    }
}

float32_t JitterGenerator::getSpeed() const
{
    return this->speed_;
}

void JitterGenerator::setMagnitude(float32_t magnitude)
{
    this->magnitude_ = magnitude;
}

float32_t JitterGenerator::getMagnitude() const
{
    return this->magnitude_;
}

void JitterGenerator::reset()
{
    this->phase_ = 0.0f;
}

float32_t JitterGenerator::process()
{
    float32_t out = arm_sin_f32(this->phase_);

    this->phase_ += this->phase_increment_ * (1.0f + this->magnitude_ * this->rnd_distribution_(this->rnd_generator_));
    if(this->phase_ > Constants::M2PI)
    {
        this->phase_ -= Constants::M2PI;
    }

    return out;
}


//////////////////////////////////////////
// PerlinNoiseGenerator implemlentation //
//////////////////////////////////////////
#define MAX_FREQUENCY_PERLIN_NOISE_GENERATOR 0.5f

const float32_t PerlinNoiseGenerator::Gradients[] = 
{
    -1.0f, +1.0f,
    -1.0f, -1.0f,
    +1.0f, -1.0f,
    +1.0f, +1.0f
};

PerlinNoiseGenerator::PerlinNoiseGenerator(float32_t sampling_rate, float32_t rate) :
    FXBase(sampling_rate),
    rate_(0.0f),
    phase_(0.0f),
    phase_increment_(0.0f),
    current_(0.0f)
{
    this->setRate(rate);

    this->reset();
}

PerlinNoiseGenerator::~PerlinNoiseGenerator()
{
}

void PerlinNoiseGenerator::setRate(float32_t rate)
{
    rate = constrain(rate, 0.0f, 1.0f);
    if(rate != this->rate_)
    {
        this->rate_ = rate;
        this->phase_increment_ = Constants::M2PI * rate / this->getSamplingRate();
    }
}

float32_t PerlinNoiseGenerator::getRate() const
{
    return this->rate_;
}

float32_t PerlinNoiseGenerator::getCurrent() const
{
    return this->current_;
}

void PerlinNoiseGenerator::reset()
{
    this->phase_ = 0.0f;
    this->current_ = 0.0f;
}

float32_t PerlinNoiseGenerator::process()
{
    if(this->rate_ == 0.0f)
    {
        return this->current_ = 0.0f;
    }

    this->current_ = PerlinNoiseGenerator::perlin(this->phase_);
    this->phase_ += this->phase_increment_;
    if(this->phase_ >= Constants::M2PI)
    {
        this->phase_ -= Constants::M2PI;
    }

    return this->current_;
}

int PerlinNoiseGenerator::hash(int x)
{
    x = ((x << 13) ^ x);
    return (x * (x * x * 15731 + 789221) + 1376312589) & 0x7fffffff;
}

float32_t PerlinNoiseGenerator::interpolate(float32_t a, float32_t b, float32_t x)
{
    float32_t ft = x * PI;
    float32_t f = (1.0f - arm_cos_f32(ft)) * 0.5;
    return a * (1.0f - f) + b * f;
}

float32_t PerlinNoiseGenerator::perlin(float32_t x)
{
    // Find the unit square that contains x
    int squareX = (int)x;

    // Find the relative x of x within that square
    double relX = x - squareX;

    // Calculate the hashes for the square's four corners
    int h1 = PerlinNoiseGenerator::hash(squareX);
    int h2 = PerlinNoiseGenerator::hash(squareX + 1);

    // Calculate the gradients for each corner
    double grad1 = PerlinNoiseGenerator::Gradients[h1 & 3];
    double grad2 = PerlinNoiseGenerator::Gradients[h2 & 3];

    // Calculate the dot products between the gradient vectors and the distance vectors
    double dot1 = grad1 * relX;
    double dot2 = grad2 * (relX - 1);

    // Interpolate the dot products and return the final noise value
    return PerlinNoiseGenerator::interpolate(dot1, dot2, relX);
}

//////////////////////////////////
// softSaturate implemlentation //
//////////////////////////////////
float32_t softSaturator1(float32_t in, float32_t threshold)
{
    float32_t x = std::abs(in);
    float32_t y = 0.0f;
    if(x < threshold)
    {
        y = x;
    }
    else if(x > threshold)
    {
        y = threshold + (x - threshold) / (1.0f + std::pow((x - threshold) / (1.0f - threshold), 2.0f));
    }
    else if(x > 1.0f)
    {
        y = (threshold + 1.0f) / 2.0f;
    }

    float32_t g = 2.0f / (1.0f + threshold);
    y *= g;

    return (in < 0.0f) ? -y : y;
}

float32_t softSaturator2(float32_t input, float32_t saturation)
{
    constexpr static float kTubeCurve = 4.0f;
    constexpr static float kTubeBias  = 0.5f;

    float absInput = std::abs(input);
    float output = 0.0f;
    if(absInput > kTubeBias)
    {
        output = (kTubeCurve + saturation) * (absInput - kTubeBias) / (1.0f - kTubeBias);
    }
    else
    {
        output = (kTubeCurve + saturation) * absInput / (1.0f + kTubeCurve * absInput);
    }

    // Clip the output if overdrive is set to 1
    // output = std::min(1.0f, output);
    if(output > 1.0f)
    {
        output = 1.0f;
    }
    else
    {
        output -= output * output * output / 3.0f;
    }

    if(input < 0.0f)
    {
        output = -output;
    }

    return output;
}

float32_t softSaturator3(float32_t input, float32_t overdrive)
{
    const float32_t w = (1.0f + overdrive) * Constants::MPI_4;
    return constrain(std::tan(w * input), -1.0f, 1.0f);
}

float32_t softSaturator4(float32_t input, float32_t saturator_factor)
{
    float32_t x = input * (saturator_factor);
    float32_t abs_x = std::fabs(x);
    float32_t sat_x = std::log(1.0 + abs_x) / std::log(1.0f + saturator_factor);
    return x > 0 ? sat_x : -sat_x;
}

float32_t waveFolder(float32_t input, float32_t bias)
{
    bias = 0.5 + (2.0f - bias) / 4.0f;
    float32_t output = std::abs(input) / bias;

    if(output > 1.0f)
    {
        output = 2.0f - output;
    }

    if(input < 0.0f)
    {
        output = -output;
    }

    return output;
}