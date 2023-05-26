#include "test_fx_helper.h"

#include <fstream>
#include <sstream>
#include <iomanip>

class FastLFODebugger
{
public:
    FastLFODebugger(float32_t sampling_rate, float32_t min_frequency, float32_t max_frequency, float32_t initial_phase, bool centered) :
        SamplingFrequency(sampling_rate),
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
        initial_amplitude_(0.0f)
    {
        assert(this->min_frequency_ <= this->max_frequency_);
        assert(this->max_frequency_ < sampling_rate / 2.0f);

        this->setFrequency(this->min_frequency_);
    }

    ~FastLFODebugger()
    {
    }

    inline float32_t getSamplingRate() const
    {
        return this->SamplingFrequency;
    }

    void setNormalizedFrequency(float32_t normalized_frequency)
    {
        normalized_frequency = constrain(normalized_frequency, 0.0f, 1.0f);
        if(this->normalized_frequency_ != normalized_frequency)
        {
            float32_t frequency = mapfloat(normalized_frequency, 0.0f, 1.0f, this->min_frequency_, this->max_frequency_);
            this->normalized_frequency_ = normalized_frequency;
            this->frequency_ = frequency;
            this->unitary_frequency_ = this->frequency_ / this->getSamplingRate();

            this->nb_sub_increment_ = (frequency >= 3.0f ? 1 : 300);
            this->unitary_frequency_ *= this->nb_sub_increment_;

            this->updateCoefficient(1.0f);
        }
    }

    float32_t getNormalizedFrequency() const
    {
        return this->normalized_frequency_;
    }

    void setFrequency(float32_t frequency)
    {
        frequency = constrain(frequency, this->min_frequency_, this->max_frequency_);
        if(this->frequency_ != frequency)
        {
            float32_t normalized_frequency = mapfloat(frequency, this->min_frequency_, this->max_frequency_, 0.0f, 1.0f);
            this->normalized_frequency_ = normalized_frequency;
            this->frequency_ = frequency;
            this->unitary_frequency_ = this->frequency_ / this->getSamplingRate();

            this->nb_sub_increment_ = (frequency >= 3.0f ? 1 : 300);
            this->unitary_frequency_ *= this->nb_sub_increment_;

            this->updateCoefficient(1.0f);
        }
    }

    float32_t getFrequency() const
    {
        return this->frequency_;
    }

    void updateCoefficient(float32_t correction_ratio)
    {
        float32_t frequency = this->unitary_frequency_ * correction_ratio;

        float32_t sign = 16.0f;
        frequency -= 0.25f;
        if(frequency < 0.0f)
        {
            frequency = -frequency;
        }
        else
        {
            if(frequency > 0.5f)
            {
                frequency -= 0.5f;
            }
            else
            {
                sign = -16.0f;
            }
        }

        this->iir_coefficient_ = sign * frequency * (1.0f - 2.0f * frequency);
        this->initial_amplitude_ = this->iir_coefficient_ * 0.25f;

        this->reset(correction_ratio);
    }

    void reset(float32_t correction_ratio = 1.0f)
    {
        // static const float32_t epsilon = 1e-7;

        this->sub_increment_ = 0.0f;

        // computing cos(0) = sin(-PI/2)
        this->y1_ = this->initial_amplitude_;
        this->y0_ = 0.5f;

        // if(this->unitary_frequency_ == 0.0f)
        // {
        //     return;
        // }

        // float32_t p_i = 2.0f * PI * this->unitary_frequency_ * correction_ratio / static_cast<float32_t>(this->nb_sub_increment_);
        // float32_t p = PI / 2.0f;
        // float32_t t_p = this->InitialPhase;
        // if(t_p < p)
        // {
        //     p -= 2.0f* PI;
        // }
        // float32_t current = 3.0f;
        // while(abs(current, sin(this->InitialPhase)) > epsilon)
        // {
        //     std::cout << "phase: " << p << std::endl;
        //     this->process();
        //     p += p_i;
        //     if(p > (6.0f * PI))
        //         cout << "ERROR: FLO is not precise enough" <<
        //         return;
        // }
    }

    float32_t process()
    {
        float32_t temp = this->y0_;
        float32_t current = temp + 0.5f;
        if(this->centered_)
        {
            current = current * 2.0f - 1.0f;
        }

        if(this->sub_increment_ == 0)
        {
            this->y0_ = this->iir_coefficient_ * this->y0_ - this->y1_;
            this->y1_ = temp;
            this->current_ = current;
            return current;
        }

        this->sub_increment_++;
        if(this->sub_increment_ >= this->nb_sub_increment_)
        {
            this->sub_increment_ = 0;
        }

        return mapfloat(this->sub_increment_, 0, this->nb_sub_increment_, this->current_, current);
    }

    float32_t current() const
    {
        return this->current_;
    }

private:
    const float32_t SamplingFrequency;
    const float32_t InitialPhase;
    const float32_t min_frequency_;
    const float32_t max_frequency_;
    const bool      centered_;
    float32_t       frequency_;
    float32_t       normalized_frequency_;
    float32_t       unitary_frequency_;
    size_t          nb_sub_increment_;
    size_t          sub_increment_;

    float32_t       y0_;
    float32_t       y1_;
    float32_t       iir_coefficient_;
    float32_t       initial_amplitude_;
    float32_t       current_;
};

void updateCorrectionStep(float32_t& sign, float32_t& correction_ratio_step, int direction)
{
    if(sign == direction)
    {
        // does nothing
    }
    else if(sign == -direction)
    {
        sign = static_cast<float32_t>(direction);
        correction_ratio_step /= 2.0f;
    }
    else
    {
        sign = static_cast<float32_t>(direction);
    }

    if(direction > 0)
    {
        std::cout << "LFO is too slow - correction ratio step becomes: " << (sign * correction_ratio_step);
    }
    else if(direction < 0)
    {
        std::cout << "LFO is too fast - correction ratio step becomes: " << (sign * correction_ratio_step);
    }
}

TEST(BetaTesta, FastLFO)
{
    const testing::TestInfo* test_info = testing::UnitTest::GetInstance()->current_test_info();
    std::string full_test_name = test_info->test_case_name();
    full_test_name += ".";
    full_test_name += test_info->name();
    
    size_t NB = static_cast<size_t>(1.0f * SAMPLING_FREQUENCY);
    const float32_t freq = 1.5f;
    const float32_t init_phase = PI / 2.0f;
    float32_t correction_ratio = 1.0f;
    float32_t correction_ratio_step = 8.0f/ SAMPLING_FREQUENCY;

    FastLFODebugger lfo1(SAMPLING_FREQUENCY, freq, 440.0f, init_phase, true);
    lfo1.setFrequency(freq);

    const float32_t epsilon = 1e-3;

    int nbTrials = 100000;
    float32_t maxDiff;
    float32_t sign = 0.0f;
    float32_t phase;
    float32_t phase_increment;
    size_t maxOK = 0;
    float32_t best_correction = correction_ratio;
    while(nbTrials > 0)
    {
        maxDiff = 0.0f;
        phase = init_phase;
        correction_ratio += sign * correction_ratio_step;
        std::cout << std::setprecision(9) << std::fixed << "    - Testing correction_ratio: " << correction_ratio << std::endl;
        lfo1.updateCoefficient(correction_ratio);
        phase_increment = freq / SAMPLING_FREQUENCY;

        for(size_t i = 0; i < NB; ++i)
        {
            float32_t v1 = lfo1.process();
            float32_t v2 = sin(phase);
            // std::cout << std::setprecision(9) << std::fixed << " + phase: " << phase << " // v1: " << v1 << " / v2: " << v2 << " => diff: " << (v2 - v1);
            
            float32_t diff = abs(v1 - v2);
            if(diff > maxDiff) maxDiff = diff;

            // std::cout << " - OK: " << ((diff < epsilon) ? "Yes" : "No") << std::endl;

            if(diff > epsilon)
            {
                if(maxOK < i)
                {
                    maxOK = i + 1;
                    best_correction = correction_ratio;
                }

                int quater = 0;
                if(phase > (PI / 2.0f)) ++quater;
                if(phase > PI) ++quater;
                if(phase > (3.0f * PI / 2.0f)) ++quater;

                if(v1 < v2)
                {
                    switch (quater)
                    {
                    case 0:
                    case 4:
                        // Sinus phase [0, PI / 2] => [0.00, 1.00] 
                        // Sinus phase [3 * PI / 2, 2 * PI] => [-1.00, 0.00] 
                        // lfo1 is too slow
                        updateCorrectionStep(sign, correction_ratio_step, +1);
                        break;

                    case 1:
                    case 3:
                        // Sinus phase [PI / 2, PI] => [1.00, 0.00] 
                        // Sinus phase [PI, 3 * PI / 2] => [0.00, -1.00] 
                        // lfo1 is too fast
                        updateCorrectionStep(sign, correction_ratio_step, -1);
                        break;
                    
                    default:
                        FAIL() << "Issue on phase: " << phase;
                        break;
                    }
                    break;
                }
                else
                {
                    switch (quater)
                    {
                    case 0:
                    case 4:
                        // Sinus phase [0, PI / 2] => [0.00, 1.00] 
                        // Sinus phase [3 * PI / 2, 2 * PI] => [-1.00, 0.00] 
                        // lfo1 is too fast
                        updateCorrectionStep(sign, correction_ratio_step, -1);
                        break;

                    case 1:
                    case 3:
                        // Sinus phase [PI / 2, PI] => [1.00, 0.00] 
                        // Sinus phase [PI, 3 * PI / 2] => [0.00, -1.00] 
                        // lfo1 is too slow
                        updateCorrectionStep(sign, correction_ratio_step, +1);
                        break;
                    
                    default:
                        FAIL() << "Issue on phase: " << phase;
                        break;
                    }
                    break;
                }
            }

            if(correction_ratio_step < 1e-9) FAIL() << "correction_ratio_step became too small. maxOK = " << maxOK << " with best_correction = " << best_correction;

            phase += phase_increment;
            if(phase > 2.0f * PI) phase -= 2.0f * PI;
        }

        --nbTrials;
    }
    if(nbTrials > -2)
        std::cout << "Correct correction ratio = " << correction_ratio << " with maxDiff = " << maxDiff << std::endl;
    else
        std::cout << "No matching correction ratio" << std::endl;

    std::cout << "maxOK = " << maxOK << " with best_correction = " << best_correction << std::endl;;


    // std::stringstream ssFst;
    // std::stringstream ssSin;

    // for(size_t i = 0; i < NB; ++i)
    // {
    //     ssFst << lfo1.process() << (i == (NB - 1) ? "" : ", ");
    //     ssSin << sin(2.0f * PI * freq * i / SAMPLING_FREQUENCY + init_phase) << (i == (NB - 1) ? "" : ", ");
    // }

    // std::ofstream _fst(getResultFile(full_test_name + ".fst.data", true));
    // _fst << ssFst.str();
    // _fst.close();

    // std::ofstream _sin(getResultFile(full_test_name + ".sin.data", true));
    // _sin << ssSin.str();
    // _sin.close();


    // std::ofstream out(getResultFile(full_test_name + ".data.m", true));
    // out << "# m file to tune FastLFO component" << std::endl << std::endl;
    // out << "# Parameters:" << std::endl 
    //     << "# + frequency: " << freq << "Hz" << std::endl 
    //     << "# + # samples: " << NB << std::endl << std::endl;

    // out << "time = 0 : " << (NB - 1) << ";" << std::endl;
    // out << "fst_lfo = load(\"-ascii\", \"" << full_test_name  << ".fst.data\");" << std::endl;
    // out << "sin_lfo = load(\"-ascii\", \"" << full_test_name  << ".sin.data\");" << std::endl;

    // out << std::endl << std::endl;

    // out << "plot(time, fst_lfo, '-', time, sin_lfo, '-');" << std::endl;
    // out << "title('LFO tuning');" << std::endl;
    // out << "legend('FastLFODebugger', 'Sinus');" << std::endl;

    // out.close();
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
