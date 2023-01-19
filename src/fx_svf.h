// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

//
// fx_svf.h
//
// State Variable Filter used in Tape Delay
//

#include "fx.h"
#include "mixing_console_constants.h"

class StateVariableFilter : public FXElement
{
    DISALLOW_COPY_AND_ASSIGN(StateVariableFilter);

public:
    typedef enum
    {
        LPF,    // Low pass filter
        HPF,    // High pass filter
        BPF     // Band pass filter
    } Type;

    StateVariableFilter(float32_t sampling_rate, Type type, float32_t cutoff);
    virtual ~StateVariableFilter();

    void setFilterType(Type type);
    void setGainDB(float32_t gainDB);
    void setCutoff(float32_t cutoff);
    void setResonance(float32_t resonance);

    virtual void reset() override;
    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;

private:
    void updateCoefficients();

    Type type_;
    float32_t gain_;
    float32_t cutoff_;
    float32_t resonance_;
    float32_t g_;
    float32_t w_;
    float32_t a_;
    float32_t b_;
    float32_t c1_;
    float32_t c2_;
    float32_t d0_;
    float32_t d1_;
    float32_t z1_[StereoChannels::kNumChannels];
    float32_t z2_[StereoChannels::kNumChannels];
};