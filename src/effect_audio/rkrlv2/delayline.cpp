/*
  Author: Ryan BillingV

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 3 of the GNU General Public License
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License (version 2) for more details.

  You should have received a copy of the GNU General Public License (version 2)
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/
#include "delayline.h"
#include <arm_math.h>
#include <stdlib.h>
#include "f_sin.h"

delayline::delayline(float maxdelay, int maxtaps_, double samplerate)
{
    fSAMPLE_RATE = samplerate;
    maxtaps = maxtaps_;
    maxtime = fSAMPLE_RATE * maxdelay;
    maxdelaysmps = fSAMPLE_RATE * lrintf(ceilf(maxdelay));
    ringbuffer = (float *) malloc(sizeof(float) * maxdelaysmps);
    avgtime = (float *) malloc(sizeof(float) * maxtaps);
    time = (float *) malloc(sizeof(float) * maxtaps);
    xfade = (float *) malloc(sizeof(float) * maxtaps);
    cur_smps = (float *) malloc(sizeof(float) * maxtaps);
    oldtime = (int *) malloc(sizeof(int) * maxtaps);
    newtime = (int *) malloc(sizeof(int) * maxtaps);
    crossfade = (int *) malloc(sizeof(int) * maxtaps);

    pstruct = (phasevars *) malloc(sizeof(struct phasevars) * maxtaps);
    tapstruct = (tapvars *) malloc(sizeof(struct tapvars) * maxtaps);

    zero_index = 0;
    tap = 0;
    rvptr = 0;
    distance = 0;

    mix = 0.5f;
    imix = 0.5f;

    float dt = 1.0f / fSAMPLE_RATE;
    alpha = dt / (0.15f + dt);
    beta = 1.0f - alpha;	//time change smoothing parameters

    cleanup();
};

delayline::~delayline()
{
	free(ringbuffer);
	free(avgtime);
	free(time);
	free(xfade);
	free(cur_smps);
	free(oldtime);
	free(newtime);
	free(crossfade);
	free(pstruct);
	free(tapstruct);
}

void
delayline::cleanup()
{
    zero_index = 0;
    int i, k;
    for (i = 0; i < maxdelaysmps; i++)
        ringbuffer[i] = 0.0;
    for (i = 0; i < maxtaps; i++) {
        avgtime[i] = 0.0;
        time[i] = 0.0;
        for (k = 0; k < 4; k++) {
            pstruct[i].yn1[k] = 0.0f;
            pstruct[i].xn1[k] = 0.0f;
            pstruct[i].gain[k] = 0.0f;
            tapstruct[i].lvars[k] = 0.0f;
            tapstruct[i].ivars[k] = 0.0f;
            tapstruct[i].fracts[k] = 0.0f;

        }
    }

    for (i = 0; i < maxtaps; i++) {
        avgtime[i] = 0.0f;
        newtime[i] = 0;
        oldtime[i] = 0;
        xfade[i] = 0.0f;
        crossfade[i] = 0;
        cur_smps[i] = 0.0f;

    }

    set_averaging(0.25f);

};


float
delayline::delay_simple(float smps, float time_, int tap_, int touch,
                        int reverse)
{
    int dlytime = 0;
    int bufptr = 0;

    if (tap_ >= maxtaps)
        tap = 0;
    else
        tap = tap_;

    time[tap] = fSAMPLE_RATE * time_;	//convert to something that can be used as a delay line index

//Do some checks to keep things in bounds
    if (time[tap] > maxtime)
        time[tap] = maxtime;
    dlytime = lrintf(time[tap]);

    if (crossfade[tap]) {
        xfade[tap] += fadetime;
        if (xfade[tap] >= 1.0f) {
            xfade[tap] = 0.0f;
            crossfade[tap] = 0;
            oldtime[tap] = newtime[tap];
            newtime[tap] = dlytime;
        }
    }

    if (crossfade[tap] == 0) {
        if (dlytime != oldtime[tap]) {
            crossfade[tap] = 1;
            xfade[tap] = 0.0f;
            oldtime[tap] = newtime[tap];
            newtime[tap] = dlytime;
        }

    }





    dlytime = newtime[tap];

//now put in the sample
    if (touch) {		//make touch zero if you only want to pull samples off the delay line
        ringbuffer[zero_index] = smps;
        if (--zero_index < 0)
            zero_index = maxdelaysmps - 1;
    }
//if we want reverse delay
//you need to call this every time to keep the buffers up to date, and it's on a different tap
    if (reverse) {

        bufptr = (dlytime + zero_index);	//this points to the sample we want to get
        if (bufptr >= maxdelaysmps)
            bufptr -= maxdelaysmps;
        if (++rvptr > maxdelaysmps)
            rvptr = 0;

        if (bufptr > zero_index) {
            if (rvptr > bufptr) {
                rvptr = zero_index;
                distance = 0;
            } else
                distance = rvptr - zero_index;
        } else if ((bufptr < zero_index) && (rvptr < zero_index)) {
            if (rvptr > bufptr) {
                rvptr = zero_index;
                distance = 0;
            } else
                distance =
                    rvptr + maxdelaysmps - zero_index;
        } else
            distance = rvptr - zero_index;


        bufptr = rvptr;	//this points to the sample we want to get

    } else {
        bufptr = (dlytime + zero_index);	//this points to the sample we want to get
        if (bufptr >= maxdelaysmps)
            bufptr -= maxdelaysmps;
    }

    int oldnewdiff = newtime[tap] - oldtime[tap];
    int tmpptr = 0;
    if (crossfade[tap] != 0) {
        tmpptr = bufptr + oldnewdiff;
        if (tmpptr >= maxdelaysmps)
            tmpptr -= maxdelaysmps;
        else if (tmpptr <= 0)
            tmpptr += maxdelaysmps;
        return (xfade[tap] * ringbuffer[bufptr] + (1.0f - xfade[tap]) * ringbuffer[tmpptr]);	//fade nicely to new tap
    } else
        return (ringbuffer[bufptr]);

};

/*
*  Interpolated delay line
*/

float
delayline::delay(float smps, float time_, int tap_, int touch,
                 int reverse)
{
    int dlytime = 0;
    int bufptr = 0;

    tap = fabs(tap_);
    if (tap >= maxtaps)
        tap = 0;

    if (reverse)  avgtime[tap] = alpha * 2.0*time_ + beta * avgtime[tap];	//smoothing the rate of time change
    else avgtime[tap] = alpha * time_ + beta * avgtime[tap];	//smoothing the rate of time change
    time[tap] = 1.0f + fSAMPLE_RATE * avgtime[tap];	//convert to something that can be used as a delay line index

//Do some checks to keep things in bounds
    if (time[tap] > maxtime)
        time[tap] = maxtime;
    if (time[tap] < 0.0f)
        time[tap] = 0.0f;

    float fract = (time[tap] - floorf(time[tap]));	//compute fractional delay
    dlytime = lrintf(floorf(time[tap]));

//now put in the sample
    if (touch) {		//make touch zero if you only want to pull samples off the delay line
        cur_smps[tap] = ringbuffer[zero_index] = smps;
        if (--zero_index < 0)
            zero_index = maxdelaysmps - 1;
    }
//if we want reverse delay
//you need to call this every time to keep the buffers up to date, and it's on a different tap
    if (reverse) {

        bufptr = (dlytime + zero_index);	//this points to the sample we want to get
        if (bufptr >= maxdelaysmps)
            bufptr -= maxdelaysmps;
        if (++rvptr > maxdelaysmps)
            rvptr = 0;

        if (bufptr > zero_index) {
            if (rvptr > bufptr) {
                rvptr = zero_index;
                distance = 0;
            } else
                distance = rvptr - zero_index;
        } else if ((bufptr < zero_index) && (rvptr < zero_index)) {
            if (rvptr > bufptr) {
                rvptr = zero_index;
                distance = 0;
            } else
                distance =
                    rvptr + maxdelaysmps - zero_index;
        } else
            distance = rvptr - zero_index;

        bufptr = rvptr;	//this points to the sample we want to get

    } else {
        bufptr = (dlytime + zero_index);	//this points to the sample we want to get
        if (bufptr >= maxdelaysmps)
            bufptr -= maxdelaysmps;
    }

    tapstruct[tap].lvars[3] = tapstruct[tap].lvars[2];
    tapstruct[tap].lvars[2] = tapstruct[tap].lvars[1];
    tapstruct[tap].lvars[1] = tapstruct[tap].lvars[0];
    tapstruct[tap].lvars[0] = ringbuffer[bufptr];

    tapstruct[tap].ivars[3] = tapstruct[tap].ivars[2];
    tapstruct[tap].ivars[2] = tapstruct[tap].ivars[1];
    tapstruct[tap].ivars[1] = tapstruct[tap].ivars[0];
    tapstruct[tap].ivars[0] = cur_smps[tap];

    tapstruct[tap].fracts[3] = tapstruct[tap].fracts[2];
    tapstruct[tap].fracts[2] = tapstruct[tap].fracts[1];
    tapstruct[tap].fracts[1] = tapstruct[tap].fracts[0];
    tapstruct[tap].fracts[0] = fract;

    float tmpfrac =
        0.5f * (tapstruct[tap].fracts[1] + tapstruct[tap].fracts[2]);
    //float itmpfrac = 1.0f - tmpfrac;
    float itmpfrac = 0.5f;  //it was the original approximation 

    float output =
        mix * lagrange(tapstruct[tap].ivars[0],
                       tapstruct[tap].ivars[1],
                       tapstruct[tap].ivars[2],
                       tapstruct[tap].ivars[3],
                       itmpfrac) + imix * lagrange(tapstruct[tap].lvars[0],
                               tapstruct[tap].lvars[1],
                               tapstruct[tap].lvars[2],
                               tapstruct[tap].lvars[3],
                               tmpfrac);

    return (output);

};


inline float
delayline::get_phaser(float smps, float lfo, int tap_, int stg)
{
    float delta = lfo;
    if (delta > 1.0f)
        delta = 1.0f;
    if (delta < 0.0f)
        delta = 0.0f;
    tap = tap_;

    pstruct[tap].gain[0] = (1.0f - delta) / (1.0f + delta);
    pstruct[tap].stages = stg;

    return (phaser(smps));
};


inline float
delayline::phaser(float fxn)	//All-pass interpolation
{

    float xn = fxn;
    for (int st = 0; st < pstruct[tap].stages; st++) {
        pstruct[tap].yn1[st] =
            pstruct[tap].xn1[st] - pstruct[tap].gain[st] * (xn +
                    pstruct
                    [tap].
                    yn1
                    [st]);
        pstruct[tap].xn1[st] = xn;
        xn = pstruct[tap].yn1[st];
    }

    return xn;

};

/*  Unfactored WYSIWYG implementation of order=4 Lagrange interpolation polynomial
inline float
delayline::lagrange(float p0, float p1, float p2, float p3, float x_)
{
float x = x_;

float xm2xm1 = (x - 1.0f)*(x - 2.0f);
x = -p0*x*xm2xm1*0.16666666667f + p1*(x + 1.0f)*xm2xm1*0.5f - p2*x*(x + 1.0f)*(x - 2.0f)*0.5f + p3*x*(x + 1.0f)*(x - 1.0f)*0.16666666667f;

return x;
};
*/

inline float
delayline::lagrange(float p0, float p1, float p2, float p3, float x_)
{
//factored version for less multiplies
    float x = x_;

    const float c0p0 = -0.16666666667f * p0;
    const float c1p1 = 0.5f * p1;
    const float c2p2 = -0.5f * p2;
    const float c3p3 = 0.16666666667f * p3;

    const float a = c3p3 + c2p2 + c1p1 + c0p0;
    const float b = -3.0f * c0p0 - p1 - c2p2;
    const float c = 2.0f * c0p0 - c1p1 + p2 - c3p3;
    const float d = p1;

    x = ((a * x + b) * x + c) * x + d;
    return x;
};

inline float
delayline::spline(float p0, float p1, float p2, float p3, float x_)
{
//does not produce any better results than lagrange(), but has less multiplies
//seems to produce discontinuities on a low level (-48dB), so not a preferred algorithm

    float x = x_;

    float const c0 = p1;
    float const c1 = 0.5f * (p2 - p0);
    float const c2 = p0 - 2.5f * p1 + 2.0f * p2 - 0.5f * p3;
    float const c3 = 0.5f * (p3 - p0) + 1.5f * (p1 - p2);

    return (((c3 * x + c2) * x + c1) * x + c0);
}

void delayline::set_averaging(float tc_)
{
    float tc = tc_;
    float dt = 1.0f / fSAMPLE_RATE;
    fadetime = dt * tc;
    alpha = dt / (tc + dt);
    beta = 1.0f - alpha;	//time change smoothing parameters
};

void delayline::set_mix(float mix_)	//mix amount of dry w/ wet
{
    mix = fabs(mix_);
    imix = 1.0f - mix;
    if(mix_<0.0f) imix*=-1.0f;
}

float delayline::envelope()
{
    float fdist = ((float) distance) / time[tap];
    if (fdist > 0.5f) {
        if (fdist <= 1.0f)
            fdist = 1.0f - fdist;
        else
            fdist = 0.0f;
    }

    if (fdist <= 0.125f) {
        fdist =
            1.0f - f_sin(PI * fdist * 4.0f + 1.5707963267949f);
    } else
        fdist = 1.0f;
    return fdist;

};
