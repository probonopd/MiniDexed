#include "test_fx_helper.h"

string getScenarioName(int scenario)
{
    stringstream ss;

    bool fxTube = Active(scenario, FXSwitch::FX__Tube);
    bool fxChorus = Active(scenario, FXSwitch::FX__Chorus);
    bool fxPhaser = Active(scenario, FXSwitch::FX__Phaser);
    bool fxOrbitone = Active(scenario, FXSwitch::FX__Orbitone);
    bool fxFlanger = Active(scenario, FXSwitch::FX__Flanger);
    bool fxDelay = Active(scenario, FXSwitch::FX__Delay);
    bool fxShimmer = Active(scenario, FXSwitch::FX__ShimmerReverb);
    bool fxReverb = Active(scenario, FXSwitch::FX__PlateReverb);
    bool first = true;

    ss << "[ ";

    if(fxTube) 
    {
        if(!first) ss << ", ";
        ss << "Tube";
        first = false;
    }

    if(fxChorus) 
    {
        if(!first) ss << ", ";
        ss << "Chorus";
        first = false;
    }

    if(fxPhaser) 
    {
        if(!first) ss << ", ";
        ss << "Phaser";
        first = false;
    }

    if(fxOrbitone) 
    {
        if(!first) ss << ", ";
        ss << "Orbitone";
        first = false;
    }

    if(fxFlanger) 
    {
        if(!first) ss << ", ";
        ss << "Flanger";
        first = false;
    }

    if(fxDelay) 
    {
        if(!first) ss << ", ";
        ss << "Delay";
        first = false;
    }

    if(fxDelay) 
    {
        if(!first) ss << ", ";
        ss << "Delay";
        first = false;
    }

    if(fxReverb) 
    {
        if(!first) ss << ", ";
        ss << "Reverb";
        first = false;
    }

    if(fxShimmer) 
    {
        if(!first) ss << ", ";
        ss << "Shimmer";
        first = false;
    }

    ss << " ]";

    return ss.str();
}


void setupOuputStreamFocCSV(std::ostream& out)
{
    struct comma_separator : numpunct<char>
    {
        virtual char do_decimal_point() const override { return ','; }
    };

    out.imbue(locale(out.getloc(), new comma_separator));
    out << fixed << showpoint;
}

FxComponentFixture::FxComponentFixture() :
    testing::Test(),
    gen_(rd_()),
    dist_(-1.0f, 1.0f)
{
}

void FxComponentFixture::SetUp()
{
}

void FxComponentFixture::TearDown()
{
}

string FxComponentFixture::getResultFile(const std::string& filename)
{
    return std::string(STR(OUTPUT_FOLDER)) + "/" + filename;
}

float32_t FxComponentFixture::getRandomValue()
{
    return this->dist_(this->gen_);
}
