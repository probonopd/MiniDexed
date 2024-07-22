/*
    ==============================================================================
    This file is part of Tal-NoiseMaker by Patrick Kunz.

    Copyright(c) 2005-2010 Patrick Kunz, TAL
    Togu Audio Line, Inc.
    http://kunz.corrupt.ch

    This file may be licensed under the terms of of the
    GNU General Public License Version 2 (the ``GPL'').

    Software distributed under the License is distributed
    on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
    express or implied. See the GPL for the specific language
    governing rights and limitations.

    You should have received a copy of the GPL along with this
    program. If not, go to http://www.gnu.org/licenses/gpl.html
    or write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
    ==============================================================================
 */

#if !defined(__Chorus_h)
#define __Chorus_h

#include "OnePoleLP.h"
#include "math.h"

class Chorus {
public:
    float *delayLineStart;
    float *delayLineEnd;
    float *writePtr;

    int delayLineLength;
    float rate;
    float delayLineOutput;

    float sampleRate;
    float delayTime;

    // Runtime variables
    float offset, diff, frac, *ptr, *ptr2;

    int readPos;

    OnePoleLP *lp;
    float z1;
    float mult, sign;

    // lfo
    float lfoPhase, lfoStepSize, lfoSign;

    Chorus(float sampleRate, float phase, float rate, float delayTime) {
        this->rate = rate;
        this->sampleRate = sampleRate;
        this->delayTime = delayTime;
        z1 = 0.0f;
        sign = 0;
        lfoPhase = phase * 2.0f - 1.0f;
        lfoStepSize = (4.0f * rate / sampleRate);
        lfoSign = 1.0f;

        // Compute required buffer size for desired delay and allocate it
        // Add extra point to aid in interpolation later
        delayLineLength = ((int)floorf(delayTime * sampleRate * 0.001f) * 2);
        delayLineStart = new float[delayLineLength];

        // Set up pointers for delay line
        delayLineEnd = delayLineStart + delayLineLength;
        writePtr = delayLineStart;

        // Zero out the buffer (silence)
        do {
            *writePtr = 0.0f;
        }
        while (++writePtr < delayLineEnd);

        // Set read pointer to end of delayline. Setting it to the end
        // ensures the interpolation below works correctly to produce
        // the first non-zero sample.
        writePtr = delayLineStart + delayLineLength -1;
        delayLineOutput = 0.0f;
        lp = new OnePoleLP();
    }

    ~Chorus() {
        delete[] delayLineStart;
        delete lp;
    }

    void setLfoRate(float rate) {
        this->rate = rate;
        lfoStepSize = (4.0f * rate / sampleRate);
    }

    float process(float *sample) {
        // Get delay time
        offset = (nextLFO() * 0.3f + 0.4f) * delayTime * sampleRate * 0.001f;

        // Compute the largest read pointer based on the offset.  If ptr
        // is before the first delayline location, wrap around end point
        ptr = writePtr - (int)floorf(offset);
        if (ptr < delayLineStart)
            ptr += delayLineLength;

        ptr2 = ptr - 1;
        if (ptr2 < delayLineStart)
            ptr2 += delayLineLength;

        frac = offset - (int)floorf(offset);
        delayLineOutput = *ptr2 + *ptr * (1 - frac) - (1 - frac) * z1;
        z1 = delayLineOutput;

        // Low pass
        lp->tick(&delayLineOutput, 0.95f);

        // Write the input sample and any feedback to delayline
        *writePtr = *sample;

        // Increment buffer index and wrap if necesary
        if (++writePtr >= delayLineEnd) {
            writePtr = delayLineStart;
        }
        return delayLineOutput;
    }

    inline float nextLFO() {
        if (lfoPhase >= 1.0f)
        {
            lfoSign = -1.0f;
        }
        else if (lfoPhase <= -1.0f)
        {
            lfoSign = +1.0f;
        }
        lfoPhase += lfoStepSize * lfoSign;
        return lfoPhase;
    }
};

#endif