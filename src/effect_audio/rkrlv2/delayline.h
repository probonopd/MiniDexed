/*
  Rakarrack Guitar FX

  delayline.h - An interpolated delay line.  Input new sample and desired delay time for output.
  Copyright (C) 2010 Ryan Billing
  Author: Ryan Billing

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

#ifndef DLINE_H
#define DLINE_H

#include "global.h"


class delayline
{
public:
    delayline(float maxdelay, int maxtaps_, double samplerate);	//construct the object with intended maximum delay time
    ~delayline();
    void cleanup();
    void set_averaging(float tc_);	//use this if you want the time change averaging longer or shorter
    void set_mix(float mix_);
    float envelope();

    //Delay line simple use case is this:
    // mydelayed_sample = mydelayline->delay(input, delay_time, 0, 1, 0)
    float delay(float smps, float time, int tap_, int touch, int reverse);	//interpolating delay
    float delay_simple(float smps, float time, int tap_, int touch, int reverse);	//simple ring buffer
    //smps  - The current input sample
    //time  - amount of delay you want
    //mix   - for chorus or flanger how much of original to mix
    //tap_  - if multi-tap delay, this is the tap you want to access. Usually set touch=0
    //when accessing multiple taps after input.
    //touch  -set to zero if you want smps written to the delay line.  Set nonzero if you only want to read out of delay line
    //reverse -set to nonzero if you want to play the samples in the delay line backward.
    //Typically you want to apply an envelope to eliminate the click at wraparound from old to recent.
    //in this case, multiply the sample by the envelope:
    // myreversedelayedsample = mydelayline->delay(input, delay_time, 0, 1, 1) * mydelayline->envelope;

    float get_phaser(float smps, float lfo, int tap_, int stg);	//Allows you to use phaser directly without delay line
    //smps  - input sample
    //lfo   - ranges from 0 to 1
    //tap   - allows multiple separate phasers with the same object
    //stg   - number of phase stages to process

private:
    int zero_index;
    int tap, maxtaps;
    float maxtime;
    long maxdelaysmps;
    int rvptr, distance;

    float *avgtime, *time;	//keeping it from changing too quickly
    float tconst, alpha, beta, mix, imix;	//don't allow change in delay time exceed 1 sample at a time

    int *newtime;
    int *oldtime;
    int *crossfade;
    float *xfade, fadetime;
    float *cur_smps;

    struct phasevars {
        float yn1[4];
        float xn1[4];
        float gain[4];
        int stages;
    } *pstruct;

    float phaser(float fxn);
    float lagrange(float p0, float p1, float p2, float p3, float x_);
    float spline(float p0, float p1, float p2, float p3, float x_);

    struct tapvars {
        float lvars[4];
        float ivars[4];
        float fracts[4];
    } *tapstruct;

    float *ringbuffer;

    float fSAMPLE_RATE;

};

#endif
