#include "test_fx_helper.h"

#include "../fx_rack.h"

using namespace std;

#define MAX_SVF_SAMPLES 10000000
#define MAX_NB_ERRORS 100

void setupRack(FXRack* rack, int scenario)
{
    rack->setWetLevel(1.0f);

    rack->getTube()->setEnable(Active(scenario, FXSwitch::FX__Tube));
    rack->getTube()->setWetLevel(0.25f);
    rack->getTube()->setOverdrive(0.25f);

    rack->getChorus()->setEnable(Active(scenario, FXSwitch::FX__Chorus));
    rack->getChorus()->setWetLevel(0.5f);
    rack->getChorus()->setRate(0.4f);
    rack->getChorus()->setDepth(0.5f);
    
    rack->getFlanger()->setEnable(Active(scenario, FXSwitch::FX__Flanger));
    rack->getFlanger()->setWetLevel(0.5f);
    rack->getFlanger()->setRate(0.03f);
    rack->getFlanger()->setDepth(0.75f);
    rack->getFlanger()->setFeedback(0.5f);

    rack->getOrbitone()->setEnable(Active(scenario, FXSwitch::FX__Orbitone));
    rack->getOrbitone()->setWetLevel(0.8f);
    rack->getOrbitone()->setRate(0.4f);
    rack->getOrbitone()->setDepth(0.7f);

    rack->getPhaser()->setEnable(Active(scenario, FXSwitch::FX__Phaser));
    rack->getPhaser()->setWetLevel(1.0f);
    rack->getPhaser()->setRate(0.1f);
    rack->getPhaser()->setDepth(1.0f);
    rack->getPhaser()->setFeedback(0.5f);
    rack->getPhaser()->setNbStages(12);

    rack->getDelay()->setEnable(Active(scenario, FXSwitch::FX__Delay));
    rack->getDelay()->setWetLevel(0.6f);
    rack->getDelay()->setLeftDelayTime(0.15f);
    rack->getDelay()->setLeftDelayTime(0.2f);
    rack->getDelay()->setFeedback(0.35f);
    rack->getDelay()->setFlutterRate(0.0f);
    rack->getDelay()->setFlutterAmount(0.0f);

    rack->getShimmerReverb()->setEnable(Active(scenario, FXSwitch::FX__ShimmerReverb));
    rack->getShimmerReverb()->setWetLevel(0.5f);
    rack->getShimmerReverb()->setInputGain(0.35f);
    rack->getShimmerReverb()->setTime(0.89f);
    rack->getShimmerReverb()->setDiffusion(0.75f);
    rack->getShimmerReverb()->setLP(0.8f);
}

TEST_P(FXScenarioTest, FXRackResetAllScenarios)
{
    FXRack *rack = new FXRack(SAMPLING_FREQUENCY);

    int fxSwitch = this->GetParam();
    rack->setEnable(true);
    setupRack(rack, fxSwitch);
    rack->reset();

    delete rack;
}

TEST_P(FXScenarioTest, ScenarioProcessing)
{
    const testing::TestInfo* test_info = testing::UnitTest::GetInstance()->current_test_info();
    std::string full_test_name = test_info->test_case_name();
    full_test_name += ".";
    full_test_name += test_info->name();

    const unsigned nbRepeats = 1;
    size_t size;
    float32_t** samples = readWaveFile(AUDIO_SOURCE_FILE, size);
    float32_t* sampleOutL = new float32_t[size * nbRepeats];
    float32_t* sampleOutR = new float32_t[size * nbRepeats];
    memset(sampleOutL, 0, size * nbRepeats * sizeof(float32_t));
    memset(sampleOutR, 0, size * nbRepeats * sizeof(float32_t));

    FXRack *rack = new FXRack(SAMPLING_FREQUENCY);

    int fxSwitch = this->GetParam();
    rack->setEnable(true);
    setupRack(rack, fxSwitch);
    rack->reset();

    string name = getScenarioName(fxSwitch);

    for(unsigned i = 0; i < nbRepeats; ++i)
    {
        rack->process(samples[0], samples[1], sampleOutL + i * size, sampleOutR + i * size, size);
    }

    stringstream ss;
    ss << full_test_name << "-fx-rack" << name << ".wav";
    saveWaveFile(getResultFile(ss.str(), true), sampleOutL, sampleOutR, nbRepeats * size, static_cast<unsigned>(SAMPLING_FREQUENCY), 16);

    delete[] samples[0];
    delete[] samples[1];
    delete[] samples;

    delete[] sampleOutL;
    delete[] sampleOutR;

    delete rack;
}

INSTANTIATE_TEST_SUITE_P(FXRack, FXScenarioTest, testing::Range(0, 1 << (FXSwitch::FX__ShimmerReverb + 1)));
