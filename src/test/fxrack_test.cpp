#include "../fx_rack.h"

#include <iostream>
#include <ctime>
#include <random>
#include "wave.h"

using namespace std;

#define MAX_SVF_SAMPLES 10000000
#define MAX_NB_ERRORS 100

enum CosineOscillatorMode
{
    COSINE_OSCILLATOR_APPROXIMATE,
    COSINE_OSCILLATOR_EXACT
};

class CosineOscillator
{
public:
    CosineOscillator() {}
    ~CosineOscillator() {}

    template <CosineOscillatorMode mode>
    inline void Init(float frequency)
    {
        if (mode == COSINE_OSCILLATOR_APPROXIMATE)
        {
            InitApproximate(frequency);
        }
        else
        {
            iir_coefficient_ = 2.0f * cosf(2.0f * M_PI * frequency);
            initial_amplitude_ = iir_coefficient_ * 0.25f;
        }
        Start();
    }

    inline void InitApproximate(float frequency)
    {
        float sign = 16.0f;
        frequency -= 0.25f;
        if (frequency < 0.0f)
        {
            frequency = -frequency;
        }
        else
        {
            if (frequency > 0.5f)
            {
                frequency -= 0.5f;
            }
            else
            {
                sign = -16.0f;
            }
        }
        iir_coefficient_ = sign * frequency * (1.0f - 2.0f * frequency);
        initial_amplitude_ = iir_coefficient_ * 0.25f;
    }

    inline void Start()
    {
        y1_ = initial_amplitude_;
        y0_ = 0.5f;
    }

    inline float value() const
    {
        return y1_ + 0.5f;
    }

    inline float Next()
    {
        float temp = y0_;
        y0_ = iir_coefficient_ * y0_ - y1_;
        y1_ = temp;
        return temp + 0.5f;
    }

private:
    float y1_;
    float y0_;
    float iir_coefficient_;
    float initial_amplitude_;

    DISALLOW_COPY_AND_ASSIGN(CosineOscillator);
};

void testCosineOsc(unsigned& step)
{
    cout << "Step #" << (++step) << ": Testing CosineOscillator" << endl;

    CosineOscillator osc;
    osc.template Init<CosineOscillatorMode::COSINE_OSCILLATOR_APPROXIMATE>(32.0f * 0.5f / 32000.0f);

    for(unsigned i = 0; i < 2000; ++i)
    {
        cout << "LFO #" << i << ": " << osc.Next() << endl;
    }
}

void testFlutter(unsigned& step)
{
    cout << "Step #" << (++step) << ": Testing JitterGenerator" << endl;

    JitterGenerator jg(44100.0f);
    jg.setSpeed(1.0f);
    jg.setMagnitude(0.1f);

    for (unsigned i = 0; i < 1000; ++i)
    {
        cout << jg.process() << endl;
    }
}

void testSVF(unsigned& step, std::mt19937& gen, std::uniform_real_distribution<float32_t> dist)
{
    float32_t inL, inR;
    float32_t outL, outR;
    StateVariableFilter svf(44100.0f, StateVariableFilter::Type::LPF, 12000.0f);

    cout << "Step #" << (++step) << ": Testing SVF in LPF mode" << endl;
    {
        svf.setFilterType(StateVariableFilter::Type::LPF);
        svf.setCutoff(12000.0f);
        svf.setResonance(0.0f);
        unsigned nbSamples = 0;
        unsigned nbErrors = 0;
        while (nbErrors < MAX_NB_ERRORS && nbSamples < MAX_SVF_SAMPLES)
        {
            nbSamples++;
            inL = dist(gen);
            inR = dist(gen);
            svf.processSample(inL, inR, outL, outR);

            if (std::abs(outL) > 1.0f)
                nbErrors++;
            if (std::abs(outR) > 1.0f)
                nbErrors++;
        }
        cout << "nbSamples: " << nbSamples << " -- nbErrors: " << nbErrors << endl;
    }

    cout << "Step #" << (++step) << ": Testing SVF in HPF mode" << endl;
    {
        svf.setFilterType(StateVariableFilter::Type::LPF);
        svf.setCutoff(60.0f);
        svf.setResonance(0.0f);
        unsigned nbSamples = 0;
        unsigned nbErrors = 0;
        while (nbErrors < MAX_NB_ERRORS && nbSamples < MAX_SVF_SAMPLES)
        {
            nbSamples++;
            inL = dist(gen);
            inR = dist(gen);
            svf.processSample(inL, inR, outL, outR);

            if (std::abs(outL) > 1.0f)
                nbErrors++;
            if (std::abs(outR) > 1.0f)
                nbErrors++;
        }
        cout << "nbSamples: " << nbSamples << " -- nbErrors: " << nbErrors << endl;
    }
}

int main()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float32_t> dist(-1.0f, 1.0f);

    unsigned step = 0;

    // testCosineOsc(step);
    // testFlutter(step, gen, dist);
    // testSVF(step);

    cout << "Step #" << (++step) << ": Intanciation FXRack" << endl;
    FXRack *rack = new FXRack(44100.0f);

    cout << "Step #" << (++step) << ": Test preparation" << endl;
    rack->setEnable(true);
    rack->setWetLevel(1.0f);

    rack->getTube()->setEnable(false);
    rack->getTube()->setWetLevel(1.0f);
    rack->getTube()->setOverdrive(1.0f);

    rack->getChorus()->setEnable(true);
    rack->getChorus()->setWetLevel(0.5f);
    rack->getChorus()->setRate(0.5f);
    rack->getChorus()->setDepth(0.5f);
    rack->getChorus()->setFeedback(0.15f);
    
    rack->getPhaser()->setEnable(false);

    rack->getOrbitone()->setEnable(false);
    rack->getOrbitone()->setWetLevel(0.8f);
    rack->getOrbitone()->setFeedback(1.0f);

    rack->getFlanger()->setEnable(false);

    rack->getTapeDelay()->setEnable(false);
    rack->getTapeDelay()->setWetLevel(0.6f);
    rack->getTapeDelay()->setLeftDelayTime(0.075f);
    rack->getTapeDelay()->setLeftDelayTime(0.05f);
    rack->getTapeDelay()->setFlutterLevel(0.0f);
    rack->getTapeDelay()->setFeedbak(0.5f);

    rack->getShimmerReverb()->setEnable(false);
    rack->getShimmerReverb()->setWetLevel(0.7f);
    rack->getShimmerReverb()->setInputGain(0.45f);
    rack->getShimmerReverb()->setTime(0.89f);
    rack->getShimmerReverb()->setDiffusion(0.75f);
    rack->getShimmerReverb()->setLP(0.8f);

    const unsigned nSamples = 1;
    float32_t inSamples[2][nSamples];
    float32_t outSamples[2][nSamples];

    for (unsigned i = 0; i < nSamples; ++i)
    {
        inSamples[0][i] = dist(gen);
        inSamples[1][i] = dist(gen);
    }

    memset(outSamples[0], 0, nSamples * sizeof(float32_t));
    memset(outSamples[1], 0, nSamples * sizeof(float32_t));

    cout << "Step #" << (++step) << ": Run test" << endl;
    rack->process(inSamples[0], inSamples[1], outSamples[0], outSamples[1], nSamples);

    cout << "Step #" << (++step) << ": Render results" << endl;
    for (unsigned i = 0; i < nSamples; ++i)
    {
        std::cout << "#" << i << " " << inSamples[0][i] << "   --> " << outSamples[0][i] << " = " << ((outSamples[0][i] - inSamples[0][i]) * 100.0f / inSamples[0][i]) << "%" << std::endl;
    }

    unsigned nbRepeats = 4;

    unsigned size;
    float32_t** samples = readWaveFile("test.wav", size);
    float32_t* sampleOutL = new float32_t[size * nbRepeats];
    float32_t* sampleOutR = new float32_t[size * nbRepeats];
    memset(sampleOutL, 0, size * nbRepeats * sizeof(float32_t));
    memset(sampleOutR, 0, size * nbRepeats * sizeof(float32_t));

    for (unsigned i = 0; i < nbRepeats; ++i)
    {
        rack->process(samples[0], samples[1], sampleOutL + i * size, sampleOutR + i * size, size);
    }

    saveWaveFile("result.wav", sampleOutL, sampleOutR, nbRepeats * size, 44100, 16);

    delete[] sampleOutL;
    delete[] sampleOutR;
    delete[] samples[0];
    delete[] samples[1];
    delete[] samples;

    cout << "Step #" << (++step) << ": Test cleanup" << endl;
    delete rack;

    return 0;
}
