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
// fx_tape_delay.h
//
// Stereo Delay proposed in the context of the MiniDexed project
// Author: Vincent Gauché
//
#pragma once

#include "fx_components.h"
#include "fx_svf.h"

class Delay : public FXElement
{
    DISALLOW_COPY_AND_ASSIGN(Delay);

    class LowHighPassFilter : public FXElement
    {
        DISALLOW_COPY_AND_ASSIGN(LowHighPassFilter);

    public:
        LowHighPassFilter(float32_t sampling_rate);
        virtual ~LowHighPassFilter();

        virtual void reset() override;
        virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;

        void setCutoffChangeRatio(float32_t ratio);
        
    private:
        StateVariableFilter lpf_;
        StateVariableFilter hpf_;
        float32_t ratio_;

        IMPLEMENT_DUMP(
            const size_t space = 10;
            const size_t precision = 5;

            std::stringstream ss;

            out << "START " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;

            SS_RESET(ss, precision, std::left);
            SS__TEXT(ss, ' ', space, std::left, '|', "ratio_");
            out << "\t" << ss.str() << std::endl;

            SS_RESET(ss, precision, std::left);
            SS_SPACE(ss, '-', space, std::left, '+');
            out << "\t" << ss.str() << std::endl;

            SS_RESET(ss, precision, std::left);
            SS__TEXT(ss, ' ', space - 1, std::right, " |", this->ratio_);
            out << "\t" << ss.str() << std::endl;

            if(deepInspection)
            {
                out << "\t" << std::endl;
                this->lpf_.dump(out, deepInspection, tag + ".lpf_");
                this->hpf_.dump(out, deepInspection, tag + ".hpf_");
            }

            out << "END " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;
        )

        IMPLEMENT_INSPECT(
            size_t nb_errors = 0u;

            if(deepInspection)
            {
                nb_errors += this->lpf_.inspect(inspector, deepInspection, tag + ".lpf_");
                nb_errors += this->hpf_.inspect(inspector, deepInspection, tag + ".hpf_");
            }
            nb_errors += inspector(tag + ".ratio_", this->ratio_, -1.0f, 1.0f, deepInspection);

            return nb_errors;            
        )
    };

public:
    Delay(const float32_t sampling_rate, float32_t default_delay_time = 0.25f, float32_t default_flutter_level = 1.0f, float32_t default_wet_level = 0.5f);
    virtual ~Delay();
    
    virtual void reset() override;
    virtual void processSample(float32_t inL, float32_t inR, float32_t& outL, float32_t& outR) override;

    void setLeftDelayTime(float32_t delay_time);
    float32_t getLeftDelayTime() const;

    void setRightDelayTime(float32_t delay_time);
    float32_t getRightDelayTime() const;

    void setFeedback(float32_t feedback);
    float32_t getFeedback() const;

    void setFlutterRate(float32_t rate);
    float32_t getFlutterRate() const;

    void setFlutterAmount(float32_t amount);
    float32_t getFlutterAmount() const;

private:
    const size_t MaxSampleDelayTime;
    unsigned read_pos_L_;
    unsigned read_pos_R_;
    float32_t* buffer_L_;
    float32_t* buffer_R_;
    float32_t delay_time_L_;        // Left delay time in seconds (0.0 - 2.0)
    float32_t delay_time_R_;        // Right delay time in seconds (0.0 - 2.0)
    float32_t feedback_;            // Feedback (0.0 - 1.0)
    float32_t jitter_amount_;

    LowHighPassFilter filter_;
    PerlinNoiseGenerator jitter_generator_;

    IMPLEMENT_DUMP(
        const size_t space = 18;
        const size_t precision = 5;

        std::stringstream ss;

        out << "START " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;

        SS_RESET(ss, precision, std::left);
        SS__TEXT(ss, ' ', space, std::left, '|', "read_pos_L_");
        SS__TEXT(ss, ' ', space, std::left, '|', "read_pos_R_");
        SS__TEXT(ss, ' ', space, std::left, '|', "delay_time_L_");
        SS__TEXT(ss, ' ', space, std::left, '|', "delay_time_R_");
        SS__TEXT(ss, ' ', space, std::left, '|', "feedback_");
        SS__TEXT(ss, ' ', space, std::left, '|', "jitter_amount_");
        SS__TEXT(ss, ' ', space, std::left, '|', "filter_");
        out << "\t" << ss.str() << std::endl;

        SS_RESET(ss, precision, std::left);
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        SS_SPACE(ss, '-', space, std::left, '+');
        out << "\t" << ss.str() << std::endl;

        SS_RESET(ss, precision, std::left);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->read_pos_L_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->read_pos_R_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->delay_time_L_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->delay_time_R_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->feedback_);
        SS__TEXT(ss, ' ', space - 1, std::right, " |", this->jitter_amount_);
        out << "\t" << ss.str() << std::endl;

        if(deepInspection)
        {
            out << "Flanger internal delay lines:" << std::endl;

            SS_RESET(ss, precision, std::left);
            SS__TEXT(ss, ' ', space, std::left, '|', "index");
            SS__TEXT(ss, ' ', space, std::left, '|', "buffer_L_");
            SS__TEXT(ss, ' ', space, std::left, '|', "buffer_R_");
            out << "\t" << ss.str() << std::endl;

            SS_RESET(ss, precision, std::left);
            SS_SPACE(ss, '-', space, std::left, '+');
            SS_SPACE(ss, '-', space, std::left, '+');
            SS_SPACE(ss, '-', space, std::left, '+');
            out << "\t" << ss.str() << std::endl;

            for(size_t i = 0; i < this->MaxSampleDelayTime; ++i)
            {
                SS_RESET(ss, precision, std::left);
                SS__TEXT(ss, ' ', space - 1, std::right, " |", i);
                SS__TEXT(ss, ' ', space - 1, std::right, " |", this->buffer_L_[i]);
                SS__TEXT(ss, ' ', space - 1, std::right, " |", this->buffer_R_[i]);
                out << "\t" << ss.str() << std::endl;
            }

            this->filter_.dump(out, deepInspection, tag + ".filter_");
            this->jitter_generator_.dump(out, deepInspection, tag + ".jitter_generator_");
        }

        out << "END " << tag << "(" << typeid(*this).name() << ") dump" << std::endl << std::endl;
    )

    IMPLEMENT_INSPECT(
        size_t nb_errors = 0;
        nb_errors += inspector(tag + ".read_pos_L_", static_cast<float32_t>(this->read_pos_L_), 0.0f, static_cast<float32_t>(this->MaxSampleDelayTime), deepInspection);
        nb_errors += inspector(tag + ".read_pos_R_", static_cast<float32_t>(this->read_pos_R_), 0.0f, static_cast<float32_t>(this->MaxSampleDelayTime), deepInspection);
        nb_errors += inspector(tag + ".delay_time_L_", this->delay_time_L_, 0.0f, static_cast<float32_t>(this->MaxSampleDelayTime), deepInspection);
        nb_errors += inspector(tag + ".delay_time_R_", this->delay_time_R_, 0.0f, static_cast<float32_t>(this->MaxSampleDelayTime), deepInspection);
        nb_errors += inspector(tag + ".feedback_", this->feedback_, 0.0f, 1.0f, deepInspection);
        nb_errors += inspector(tag + ".jitter_amount_", this->jitter_amount_, 0.0f, 1.0f, deepInspection);

        if(deepInspection)
        {
            for(size_t i = 0; i < this->MaxSampleDelayTime; ++i)
            {
                nb_errors += inspector(tag + ".buffer_L_[ " + std::to_string(i) + " ]", this->buffer_L_[i], -1.0f, 1.0f, deepInspection);
                nb_errors += inspector(tag + ".buffer_R_[ " + std::to_string(i) + " ]", this->buffer_R_[i], -1.0f, 1.0f, deepInspection);
            }

            nb_errors += this->filter_.inspect(inspector, deepInspection, tag + ".filter_");
            nb_errors += this->jitter_generator_.inspect(inspector, deepInspection, tag + ".jitter_generator_");
        }

        return nb_errors;
    )
};
