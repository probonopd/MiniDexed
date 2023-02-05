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
    FXRack rack(SAMPLING_FREQUENCY);

    int fxSwitch = this->GetParam();
    rack.setEnable(true);
    setupRack(&rack, fxSwitch);
    rack.reset();
}

TEST_P(FXScenarioTest, ScenarioProcessing)
{
    int fxSwitch = this->GetParam();
    string name = getScenarioName(fxSwitch);

    FXRack rack(SAMPLING_FREQUENCY);
    rack.setEnable(true);
    setupRack(&rack, fxSwitch);
    rack.reset();

    PREPARE_AUDIO_TEST(size, inSamples, outSamples, full_test_name);
    rack.process(inSamples[0], inSamples[1], outSamples[0], outSamples[1], size);

    stringstream ss;
    ss << full_test_name << "-fx-rack" << name << ".wav";
    saveWaveFile(getResultFile(ss.str(), true), outSamples[0], outSamples[1], size, static_cast<unsigned>(SAMPLING_FREQUENCY), 16);

    CLEANUP_AUDIO_TEST(inSamples, outSamples);
}

INSTANTIATE_TEST_SUITE_P(FXRack, FXScenarioTest, testing::Range(0, 1 << (FXSwitch::FX__ShimmerReverb + 1)));
