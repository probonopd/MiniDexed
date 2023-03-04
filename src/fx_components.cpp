#include "fx_components.h"

#include <cmath>

///////////////////////////////
// Constants implemlentation //
///////////////////////////////
const float32_t Constants::M_PI_POW_2  = PI * PI;
const float32_t Constants::M_PI_POW_3  = Constants::M_PI_POW_2 * PI;
const float32_t Constants::M_PI_POW_5  = Constants::M_PI_POW_2 * Constants::M_PI_POW_3;
const float32_t Constants::M_PI_POW_7  = Constants::M_PI_POW_2 * Constants::M_PI_POW_5;
const float32_t Constants::M_PI_POW_9  = Constants::M_PI_POW_2 * Constants::M_PI_POW_7;
const float32_t Constants::M_PI_POW_11 = Constants::M_PI_POW_2 * Constants::M_PI_POW_9;

const float32_t Constants::M2PI = 2.0f * PI;
const float32_t Constants::MPI_2 = PI / 2.0f;
const float32_t Constants::MPI_3 = PI / 3.0f;
const float32_t Constants::MPI_4 = PI / 4.0f;
const float32_t Constants::M1_PI = 1.0f / PI;


/////////////////////////////
// FastLFO implemlentation //
/////////////////////////////
FastLFO::FastLFO(float32_t sampling_rate, float32_t min_frequency, float32_t max_frequency, float32_t initial_phase, bool centered) :
    FXBase(sampling_rate),
    InitialPhase(initial_phase),
    min_frequency_(min_frequency),
    max_frequency_(max_frequency),
    centered_(centered),
    frequency_(0.0f),
    nb_sub_increment_(1),
    sub_increment_(0),
    y0_(0.0f),
    y1_(0.0f),
    iir_coefficient_(0.0f),
    initial_amplitude_(0.0f),
    current_(0.0f)
{
    assert(this->min_frequency_ <= this->max_frequency_);
    assert(this->max_frequency_ < sampling_rate / 2.0f);

    this->setFrequency(this->min_frequency_);
}

FastLFO::~FastLFO()
{
}

void FastLFO::setNormalizedFrequency(float32_t normalized_frequency)
{
    normalized_frequency = constrain(normalized_frequency, 0.0f, 1.0f);
    if(this->normalized_frequency_ != normalized_frequency)
    {
        float32_t frequency = mapfloat(normalized_frequency, 0.0f, 1.0f, this->min_frequency_, this->max_frequency_);
        this->normalized_frequency_ = normalized_frequency;
        this->frequency_ = frequency;
        this->unitary_frequency_ = this->frequency_ / this->getSamplingRate();

        this->nb_sub_increment_ = (frequency >= 3.0f ? 10 : 100);
        this->unitary_frequency_ *= static_cast<float32_t>(this->nb_sub_increment_);

        this->updateCoefficient();
    }
}

float32_t FastLFO::getNormalizedFrequency() const
{
    return this->normalized_frequency_;
}

void FastLFO::setFrequency(float32_t frequency)
{
    frequency = constrain(frequency, this->min_frequency_, this->max_frequency_);
    if(this->frequency_ != frequency)
    {
        float32_t normalized_frequency = mapfloat(frequency, this->min_frequency_, this->max_frequency_, 0.0f, 1.0f);
        this->normalized_frequency_ = normalized_frequency;
        this->frequency_ = frequency;
        this->unitary_frequency_ = this->frequency_ / this->getSamplingRate();

        this->nb_sub_increment_ = (frequency >= 3.0f ? 10 : (frequency < 0.1f ? 1000 : 100));
        this->unitary_frequency_ *= static_cast<float32_t>(this->nb_sub_increment_);

        this->updateCoefficient();
    }
}

float32_t FastLFO::getFrequency() const
{
    return this->frequency_;
}

void FastLFO::updateCoefficient()
{
    this->iir_coefficient_ = 2.0f * cos(Constants::M2PI * this->unitary_frequency_);
    this->initial_amplitude_ = this->iir_coefficient_ * 0.25f;

    this->reset();
}

void FastLFO::reset()
{
    static const float32_t epsilon = 1e-3;

    this->sub_increment_ = 0.0f;

    // computing cos(0) = sin(-PI/2)
    this->y1_ = this->initial_amplitude_;
    this->y0_ = 0.5f;

    if(this->unitary_frequency_ == 0.0f)
    {
        return;
    }

    float32_t p_i = Constants::M2PI * this->unitary_frequency_ / static_cast<float32_t>(this->nb_sub_increment_);
    float32_t p = Constants::MPI_2;
    float32_t oldP = 1000.0f;
    float32_t t_p = this->InitialPhase;
    const float32_t target = sin(this->InitialPhase);
    if(t_p < p)
    {
        p -= Constants::M2PI;
    }
    float32_t tuning = -3.0f;
    while(p < t_p || abs(tuning - target) > epsilon)
    {
        oldP = p;
        tuning = this->process();
        p += p_i;
        if(oldP == p)
        {
            return;
        }
    }
}

float32_t FastLFO::process()
{
    float32_t temp = this->y0_;
    float32_t current = temp + 0.5f;
    if(this->centered_)
    {
        current = current * 2.0f - 1.0f;
    }

    this->sub_increment_++;
    if(this->sub_increment_ >= this->nb_sub_increment_)
    {
        this->sub_increment_ = 0;
        this->y0_ = this->iir_coefficient_ * this->y0_ - this->y1_;
        this->y1_ = temp;
        this->current_ = current;
        return current;
    }

    return mapfloat(this->sub_increment_, 0, this->nb_sub_increment_, this->current_, current);
}

float32_t FastLFO::current() const
{
    return this->current_;
}


/////////////////////////////
// FastLFO2 implemlentation //
/////////////////////////////
FastLFO2::FastLFO2(float32_t sampling_rate, float32_t min_frequency, float32_t max_frequency, float32_t initial_phase, bool centered) :
    FXBase(sampling_rate),
    InitialPhase(initial_phase),
    min_frequency_(min_frequency),
    max_frequency_(max_frequency),
    centered_(centered),
    frequency_(0.0f),
    phase_(initial_phase),
    phase_increment_(0.0f),
    current_(0.0f)
{
    assert(this->min_frequency_ <= this->max_frequency_);
    assert(this->max_frequency_ < sampling_rate / 2.0f);

    this->setFrequency(this->min_frequency_);
}

FastLFO2::~FastLFO2()
{
}

void FastLFO2::setNormalizedFrequency(float32_t normalized_frequency)
{
    normalized_frequency = constrain(normalized_frequency, 0.0f, 1.0f);
    if(this->normalized_frequency_ != normalized_frequency)
    {
        float32_t frequency = mapfloat(normalized_frequency, 0.0f, 1.0f, this->min_frequency_, this->max_frequency_);
        this->normalized_frequency_ = normalized_frequency;
        this->frequency_ = frequency;

        this->phase_increment_ = Constants::M2PI * frequency / this->getSamplingRate();
    }
}

float32_t FastLFO2::getNormalizedFrequency() const
{
    return this->normalized_frequency_;
}

void FastLFO2::setFrequency(float32_t frequency)
{
    frequency = constrain(frequency, this->min_frequency_, this->max_frequency_);
    if(this->frequency_ != frequency)
    {
        float32_t normalized_frequency = mapfloat(frequency, this->min_frequency_, this->max_frequency_, 0.0f, 1.0f);
        this->normalized_frequency_ = normalized_frequency;
        this->frequency_ = frequency;

        this->phase_increment_ = Constants::M2PI * frequency / this->getSamplingRate();
    }
}

float32_t FastLFO2::getFrequency() const
{
    return this->frequency_;
}

void FastLFO2::reset()
{
    this->phase_ = this->InitialPhase;
}

float32_t FastLFO2::process()
{
    static const float32_t K = 5.0f * Constants::M_PI_POW_2; 

    float32_t x = this->phase_;
    float32_t f = 4.0f;
    if(x > PI)
    {
        x -= PI;
        f = -4.0f;
    }

    float32_t tmp = 4.0f * x * (PI - x);
    this->current_ = f * tmp / (K - tmp);

    if(!this->centered_)
    {
        this->current_ = this->current_ * 0.5f + 0.5f;
    }

    this->phase_ += this->phase_increment_;
    if(this->phase_ > Constants::M2PI)
    {
        this->phase_ -= Constants::M2PI;
    }

    return this->current_;
}

float32_t FastLFO2::current() const
{
    return this->current_;
}


////////////////////////////////////////////////
// InterpolatedSineOscillator implemlentation //
////////////////////////////////////////////////
bool InterpolatedSineOscillator::ClassInitializer()
{
    float32_t phase_increment = Constants::M2PI / static_cast<float32_t>(InterpolatedSineOscillator::DataPointSize);
    float32_t phase = 0.0;
    for(size_t i = 0; i <= InterpolatedSineOscillator::DataPointSize; ++i)
    {
        InterpolatedSineOscillator::CenteredDataPoints[i] = std::sin(phase);
        InterpolatedSineOscillator::UpliftDataPoints[i] = InterpolatedSineOscillator::CenteredDataPoints[i] * 0.5f + 0.5f;
        phase += phase_increment;
    }

    return true;
}

float32_t InterpolatedSineOscillator::CenteredDataPoints[InterpolatedSineOscillator::DataPointSize + 1];
float32_t InterpolatedSineOscillator::UpliftDataPoints[InterpolatedSineOscillator::DataPointSize + 1];

const float32_t InterpolatedSineOscillator::DeltaTime = Constants::M2PI / static_cast<float32_t>(InterpolatedSineOscillator::DataPointSize);

InterpolatedSineOscillator::InterpolatedSineOscillator(float32_t sampling_rate, float32_t min_frequency, float32_t max_frequency, float32_t initial_phase, bool centered) :
    FXBase(sampling_rate),
    InitialPhase(initial_phase),
    min_frequency_(min_frequency),
    max_frequency_(max_frequency),
    centered_(centered),
    frequency_(0.0f),
    normalized_frequency_(-1.0f),
    phase_index_(initial_phase / InterpolatedSineOscillator::DeltaTime),
    phase_index_increment_(0.0f),
    current_sample_(0.0f)
{
    static bool initialized = false;
    if(!initialized)
    {
        initialized = InterpolatedSineOscillator::ClassInitializer();
    }

    assert(this->min_frequency_ <= this->max_frequency_);
    assert(this->max_frequency_ < sampling_rate / 2.0f);

    this->setFrequency(this->min_frequency_);
}

InterpolatedSineOscillator::~InterpolatedSineOscillator()
{
}

void InterpolatedSineOscillator::setNormalizedFrequency(float32_t normalized_frequency)
{
    normalized_frequency = constrain(normalized_frequency, 0.0f, 1.0f);
    if(this->normalized_frequency_ != normalized_frequency)
    {
        float32_t frequency = mapfloat(normalized_frequency, 0.0f, 1.0f, this->min_frequency_, this->max_frequency_);
        this->normalized_frequency_ = normalized_frequency;
        this->frequency_ = frequency;
        this->phase_index_increment_ = static_cast<float32_t>(InterpolatedSineOscillator::DataPointSize) * this->frequency_ / this->getSamplingRate();
    }
}

float32_t InterpolatedSineOscillator::getNormalizedFrequency() const
{
    return this->normalized_frequency_;
}

void InterpolatedSineOscillator::setFrequency(float32_t frequency)
{
    frequency = constrain(frequency, this->min_frequency_, this->max_frequency_);
    if(this->frequency_ != frequency)
    {
        float32_t normalized_frequency = mapfloat(frequency, this->min_frequency_, this->max_frequency_, 0.0f, 1.0f);
        this->normalized_frequency_ = normalized_frequency;
        this->frequency_ = frequency;
        this->phase_index_increment_ = static_cast<float32_t>(InterpolatedSineOscillator::DataPointSize) * this->frequency_ / this->getSamplingRate();
    }
}

float32_t InterpolatedSineOscillator::getFrequency() const
{
    return this->frequency_;
}

void InterpolatedSineOscillator::reset()
{
    this->phase_index_ = this->InitialPhase / InterpolatedSineOscillator::DeltaTime;
    this->current_sample_ = 0.0f;
}

float32_t InterpolatedSineOscillator::process()
{
    float32_t* dataPoints = this->centered_ ? InterpolatedSineOscillator::CenteredDataPoints : InterpolatedSineOscillator::UpliftDataPoints;

    float32_t out = 0.0f;

    float32_t findex = this->phase_index_;
    size_t index1 = static_cast<size_t>(findex);
    size_t index2 = index1 + 1;

    float32_t f1 = dataPoints[index1];
    float32_t f2 = dataPoints[index2];
    float32_t r = findex - index1;

    out = f1 + (f2 - f1) * r * InterpolatedSineOscillator::DeltaTime;

    this->phase_index_ += this->phase_index_increment_;
    if(this->phase_index_ > InterpolatedSineOscillator::DataPointSize)
    {
        this->phase_index_ -= InterpolatedSineOscillator::DataPointSize;
    }

    return this->current_sample_ = out;
}

float32_t InterpolatedSineOscillator::current() const
{
    return this->current_sample_;
}


////////////////////////////////
// ComplexLFO implemlentation //
////////////////////////////////
ComplexLFO::ComplexLFO(float32_t sampling_rate, float32_t min_frequency, float32_t max_frequency, float32_t initial_phase, bool centered) :
    FXBase(sampling_rate),
    InitialPhase(initial_phase),
    min_frequency_(min_frequency),
    max_frequency_(max_frequency),
    centered_(centered),
    normalized_frequency_(-1.0f),
    frequency_(0.0f),
    phase_(initial_phase),
    phase_increment_(0.0f),
    current_sample_(0.0f),
    new_phase_(true),
    rnd_generator_(rnd_device_()),
    rnd_distribution_(-1.0f, 1.0f)
{
    assert(this->min_frequency_ <= this->max_frequency_);
    assert(this->max_frequency_ < sampling_rate / 2.0f);

    this->setWaveform(Waveform::Sine);
    this->setFrequency(this->min_frequency_);
}

ComplexLFO::~ComplexLFO()
{
}

void ComplexLFO::setWaveform(Waveform waveform)
{
    this->waveform_ = waveform;
}

ComplexLFO::Waveform ComplexLFO::getWaveform() const
{
    return this->waveform_;
}

void ComplexLFO::setNormalizedFrequency(float32_t normalized_frequency)
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

float32_t ComplexLFO::getNormalizedFrequency() const
{
    return this->normalized_frequency_;
}

void ComplexLFO::setFrequency(float32_t frequency)
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

float32_t ComplexLFO::getFrequency() const
{
    return this->frequency_;
}

void ComplexLFO::reset()
{
    this->phase_ = this->InitialPhase;
    this->current_sample_ = 0.0f;
}

float32_t ComplexLFO::process()
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

    if(!this->centered_)
    {
        out = out * 0.5f + 0.5f;
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

float32_t ComplexLFO::current() const
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
    static const float32_t max_frequency = 0.45f * this->getSamplingRate();

    speed = constrain(speed, 0.0f, max_frequency);
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
    this->magnitude_ = constrain(magnitude, 0.0f, 1.0f);
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

float32_t PerlinNoiseGenerator::current() const
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
    const static float kTubeCurve = 4.0f;
    const static float kTubeBias  = 0.5f;

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
    float32_t x = input * saturator_factor;
    float32_t abs_x = std::abs(x);
    float32_t sat_x = std::log(1.0f + abs_x) / std::log(1.0f + saturator_factor);
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