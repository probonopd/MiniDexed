/*
  ZynAddSubFX - a software synthesizer

  Chorus.C - Chorus and Flange effects
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul

  Modified for rakarrack by Josep Andreu

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License (version 2) for more details.

  You should have received a copy of the GNU General Public License (version 2)
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/

#include <math.h>
#include "Chorus.h"
#include <stdio.h>

RKRChorus::RKRChorus (float * efxoutl_, float * efxoutr_, double sample_rate)
{
    fSAMPLE_RATE = sample_rate;
    efxoutl = efxoutl_;
    efxoutr = efxoutr_;
    dlk = 0;
    drk = 0;
    maxdelay = lrintf (MAX_CHORUS_DELAY / 1000.0 * (int)sample_rate);
    delayl = new float[maxdelay];
    delayr = new float[maxdelay];
    lfo = new EffectLFO(sample_rate);

    float tmp = 0.08f;
    ldelay = new delayline(tmp, 2, sample_rate);
    rdelay = new delayline(tmp, 2, sample_rate);
    ldelay -> set_averaging(0.005f);
    rdelay -> set_averaging(0.005f);
    ldelay->set_mix( 1.0f );
    rdelay->set_mix( 1.0f );

    Ppreset = 0;
    PERIOD = 256; //make our best guess for the initializing
    setpreset (Ppreset);

    oldr = 0.0f;
    oldl = 0.0f;
    awesome_mode = 0;

    lfo->effectlfoout (&lfol, &lfor);
    dl2 = getdelay (lfol);
    dr2 = getdelay (lfor);
    cleanup ();
};

RKRChorus::~RKRChorus ()
{
	delete delayl;
	delete delayr;
	delete ldelay;
	delete rdelay;
	delete lfo;
};

/*
 * get the delay value in samples; xlfo is the current lfo value
 */
float RKRChorus::getdelay (float xlfo)
{
    float
    result;
    if (Pflangemode == 0) {
        result = (delay + xlfo * depth) * fSAMPLE_RATE;
    } else
        result = 0;

    //check if it is too big delay(caused bu errornous setdelay() and setdepth()
    if ((result + 0.5) >= maxdelay) {
        fprintf (stderr, "%s",
                 "WARNING: Chorus.C::getdelay(..) too big delay (see setdelay and setdepth funcs.)\n");
        printf ("%f %d\n", result, maxdelay);
        result = (float) maxdelay - 1.0f;
    };
    return (result);
};

/*
 * Apply the effect
 */
void
RKRChorus::out (float * smpsl, float * smpsr, uint32_t period)
{
    unsigned int i;
    float tmp;
    float fPERIOD = period;
    float outL, outR;
    dl1 = dl2;
    dr1 = dr2;
    lfo->effectlfoout (&lfol, &lfor);

    float v1, v2;
    if (outvolume < 0.5f)
    {
        v1 = 1.0f;
        v2 = outvolume * 2.0f;
    }
    else
    {
        v1 = (1.0f - outvolume) * 2.0f;
        v2 = 1.0f;
    }

    if(awesome_mode) { //use interpolated delay line for better sound
        float tmpsub;

        dl2 = delay + lfol * depth;
        dr2 = delay + lfor * depth;
        if (Poutsub != 0) tmpsub = -1.0f;
        else tmpsub = 1.0f;

        for (i = 0; i < period; i++) {
            //Left
            mdel = (dl1 * (float)(period - i) + dl2 * (float)i) / fPERIOD;
            tmp = smpsl[i] + oldl*fb;
            outL = tmpsub*ldelay->delay(tmp, mdel, 0, 1, 0);
            oldl = outL;

            //Right
            mdel = (dr1 * (float)(period - i) + dr2 * (float)i) / fPERIOD;
            tmp = smpsr[i] + oldr*fb;
            outR = tmpsub*rdelay->delay(tmp, mdel, 0, 1, 0);
            oldr = outR;

            outL *= panning;
            outR *= (1.0f - panning);

            efxoutl[i] = smpsl[i] * v1 + outL * v2;
            efxoutr[i] = smpsr[i] * v1 + outR * v2;
        }

    } else {

        dl2 = getdelay (lfol);
        dr2 = getdelay (lfor);
        for (i = 0; i < period; i++) {
            float inl = smpsl[i];
            float inr = smpsr[i];
            //LRcross
            float l = inl;
            float r = inr;
            inl = l * (1.0f - lrcross) + r * lrcross;
            inr = r * (1.0f - lrcross) + l * lrcross;

            //Left channel

            //compute the delay in samples using linear interpolation between the lfo delays
            mdel = (dl1 * (float)(period - i) + dl2 * (float)i) / fPERIOD;
            if (++dlk >= maxdelay)
                dlk = 0;
            tmp = (float) dlk - mdel + (float)maxdelay * 2.0f;	//where should I get the sample from

            F2I (tmp, dlhi);
            dlhi %= maxdelay;

            dlhi2 = (dlhi - 1 + maxdelay) % maxdelay;
            dllo = 1.0f - fmodf (tmp, 1.0f);
            outL = delayl[dlhi2] * dllo + delayl[dlhi] * (1.0f - dllo);
            delayl[dlk] = inl + outL * fb;

            //Right channel

            //compute the delay in samples using linear interpolation between the lfo delays
            mdel = (dr1 * (float)(period - i) + dr2 * (float)i) / fPERIOD;
            if (++drk >= maxdelay)
                drk = 0;
            tmp = (float)drk - mdel + (float)maxdelay * 2.0f;	//where should I get the sample from

            F2I (tmp, dlhi);
            dlhi %= maxdelay;

            dlhi2 = (dlhi - 1 + maxdelay) % maxdelay;
            dllo = 1.0f - fmodf (tmp, 1.0f);
            outR = delayr[dlhi2] * dllo + delayr[dlhi] * (1.0f - dllo);
            delayr[dlk] = inr + outR * fb;

            if (Poutsub != 0) {
                outL *= -1.0f;
                outR *= -1.0f;
            }
            
            outL *= panning;
            outR *= (1.0f - panning);

            efxoutl[i] = smpsl[i] * v1 + outL * v2;
            efxoutr[i] = smpsr[i] * v1 + outR * v2;
        }
    } //end awesome_mode test
};

/*
 * Cleanup the effect
 */
void
RKRChorus::cleanup ()
{
    for (int i = 0; i < maxdelay; i++) {
        delayl[i] = 0.0;
        delayr[i] = 0.0;
    };

};

/*
 * Parameter control
 */
void
RKRChorus::setdepth (int Pdepth)
{
    this->Pdepth = Pdepth;
    depth = (powf (8.0f, ((float)Pdepth / 127.0f) * 2.0f) - 1.0f) / 1000.0f;	//seconds
};

void
RKRChorus::setdelay (int Pdelay)
{
    this->Pdelay = Pdelay;
    delay = (powf (10.0f, ((float)Pdelay / 127.0f) * 2.0f) - 1.0f) / 1000.0f;	//seconds
};

void
RKRChorus::setfb (int Pfb)
{
    this->Pfb = Pfb;
    fb = ((float)Pfb - 64.0f) / 64.1f;
};

void
RKRChorus::setvolume (int Pvolume)
{
    this->Pvolume = Pvolume;
    outvolume = (float)Pvolume / 127.0f;
};

void
RKRChorus::setpanning (int Ppanning)
{
    this->Ppanning = Ppanning;
    panning = ((float)Ppanning +.5f) / 127.0f;
};

void
RKRChorus::setlrcross (int Plrcross)
{
    this->Plrcross = Plrcross;
    lrcross = (float)Plrcross / 127.0f;
};

void
RKRChorus::setpreset (int npreset)
{
    const int PRESET_SIZE = 12;
    const int NUM_PRESETS = 10;
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //Chorus1
        {64, 64, 33, 0, 0, 90, 40, 85, 64, 119, 0, 0},
        //Chorus2
        {64, 64, 19, 0, 0, 98, 56, 90, 64, 19, 0, 0},
        //Chorus3
        {64, 64, 7, 0, 1, 42, 97, 95, 90, 127, 0, 0},
        //Celeste1
        {64, 64, 1, 0, 0, 42, 115, 18, 90, 127, 0, 0},
        //Celeste2
        {64, 64, 7, 117, 0, 50, 115, 9, 31, 127, 0, 1},
        //Flange1
        {64, 64, 39, 0, 0, 60, 23, 3, 62, 0, 0, 0},
        //Flange2
        {64, 64, 9, 34, 1, 40, 35, 3, 109, 0, 0, 0},
        //Flange3
        {64, 64, 31, 34, 1, 94, 35, 3, 54, 0, 0, 1},
        //Flange4
        {64, 64, 14, 0, 1, 62, 12, 19, 97, 0, 0, 0},
        //Flange5
        {64, 64, 34, 105, 0, 24, 39, 19, 17, 0, 0, 1}
    };


    for (int n = 0; n < PRESET_SIZE; n++) {
        changepar (n, presets[npreset][n]);
    }
    Ppreset = npreset;
};


void
RKRChorus::changepar (int npar, int value)
{
    switch (npar) {
    case 0:
        setvolume (value);
        break;
    case 1:
        setpanning (value);
        break;
    case 2:
        lfo->Pfreq = value;
        lfo->updateparams (PERIOD);
        break;
    case 3:
        lfo->Prandomness = value;
        lfo->updateparams (PERIOD);
        break;
    case 4:
        lfo->PLFOtype = value;
        lfo->updateparams (PERIOD);
        break;
    case 5:
        lfo->Pstereo = value;
        lfo->updateparams (PERIOD);
        break;
    case 6:
        setdepth (value);
        break;
    case 7:
        setdelay (value);
        break;
    case 8:
        setfb (value);
        break;
    case 9:
        setlrcross (value);
        break;
    case 10:
        if (value > 1)
            value = 1;
        Pflangemode = value;
        break;
    case 11:
        if (value > 1)
            value = 1;
        Poutsub = value;
        break;
    case 12:
        awesome_mode = value;
        break;
    };
};

int
RKRChorus::getpar (int npar)
{
    switch (npar) {
    case 0:
        return (Pvolume);
        break;
    case 1:
        return (Ppanning);
        break;
    case 2:
        return (lfo->Pfreq);
        break;
    case 3:
        return (lfo->Prandomness);
        break;
    case 4:
        return (lfo->PLFOtype);
        break;
    case 5:
        return (lfo->Pstereo);
        break;
    case 6:
        return (Pdepth);
        break;
    case 7:
        return (Pdelay);
        break;
    case 8:
        return (Pfb);
        break;
    case 9:
        return (Plrcross);
        break;
    case 10:
        return (Pflangemode);
        break;
    case 11:
        return (Poutsub);
        break;
    case 12:
        return (awesome_mode);
        break;
    default:
        return (0);
    };

};
