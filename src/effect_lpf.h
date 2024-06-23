#ifndef _EFFECT_LPF_H
#define _EFFECT_LPF_H

#include "effect_base.h"

class AudioEffectLPF : public AudioEffect
{
public:
    static constexpr float32_t MIN_CUTOFF = 0.00001f;
	static constexpr float32_t MAX_CUTOFF = 8000.0f;
    static constexpr float32_t MIN_RES = 0.0f;
	static constexpr float32_t MAX_RES = 1.0f;

    AudioEffectLPF(float32_t samplerate) :  AudioEffect(samplerate)
    {
        // Setup init values
        this->setCutoff(2000.0f);
        this->setResonance(MIN_RES);
    }
    virtual ~AudioEffectLPF()
    {
    }

    virtual unsigned getId()
    {
        return EFFECT_LPF;
    }

    /**
	 * Set the static cutoff frequency of the filter.
	 * Cutoff frequency must be between MIN_CUTOFF and MAX_CUTOFF.
	 * Envelope signal varies the cutoff frequency from this static value.
	  */ 
	void setCutoff(float32_t cutoff)
    {
        // Check limits
		cutoff = (cutoff < MIN_CUTOFF) ? MIN_CUTOFF : cutoff;
		cutoff = (cutoff > MAX_CUTOFF) ? MAX_CUTOFF : cutoff;
		
		this->cutoff = cutoff;
		recalculate();
	}

    /**
	 * Set the resonance of the filter.
	 * Valid values are between 0.0 and 1.0 where
	 * 0.0 is no resonance and 1.0 is full resonance or oscillation.
	 */ 
	void setResonance(float32_t resonance)
    {
        // Check limits
        resonance = (resonance < MIN_RES) ? MIN_RES : resonance;
		resonance = (resonance > MAX_RES) ? MAX_RES : resonance;

		this->resonance = resonance;
		recalculate();
	}

protected:
    virtual void doProcess(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len)
    {
		for (int i = 0; i < len; i++) {
			// Get a sample to process
			float32_t s = inblockL[i];
			
            // Return processed sample from filter
			outblockL[i] = processSample(s);
		}
    }

private:
    float32_t cutoff;
    float32_t resonance;
	float32_t x, r, p, k, y1, y2, y3, y4, oldx, oldy1, oldy2, oldy3;

    /**
	 * Recalculate filter parameters on changes to cutoff or resonance
	 */
    void recalculate()
    {
        float32_t f = (cutoff + cutoff) / this->samplerate;
		p = f * (1.8 - (0.8 * f));
		k = p + p - 1.0;

		float32_t t = (1.0 - p) * 1.386249;
		float32_t t2 = 12.0 + t * t;
		r = resonance * (t2 + 6.0 * t) / (t2 - 6.0 * t);
    }

    /**
	 * Process a single sample through the filter
	 */
	float32_t processSample(float32_t input)
    {
		// Process input
		//x = ((float32_t) input/ F32_MAX) - r * y4;
        x = input - r * y4;
		
		// Four cascaded one pole filters (bilinear transform)
		y1 = x * p + oldx * p - k * y1;
		y2 = y1 * p + oldy1 * p - k * y2;
		y3 = y2 * p + oldy2 * p - k * y3;
		y4 = y3 * p + oldy3 * p - k * y4;
		
		// Clipper band limited sigmoid
		y4 -= (y4 * y4 * y4) / 6.0;
		
		oldx = x;
        oldy1 = y1;
        oldy2 = y2;
        oldy3 = y3;
		//return (float32_t) (y4 * F32_MAX);
        return y4;
	}
};

#endif // _EFFECT_LPF_H