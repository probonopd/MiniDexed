/*
  ZynAddSubFX - a software synthesizer

  Phaser.C - Phaser effect
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
#include "Phaser.h"
#include <stdio.h>
//#include "FPreset.h"
#define PHASER_LFO_SHAPE 2

Phaser::Phaser (float * efxoutl_, float * efxoutr_, double sample_rate)
{
    efxoutl = efxoutl_;
    efxoutr = efxoutr_;

    oldl = (float *) malloc(sizeof(float)* MAX_PHASER_STAGES * 2);
    oldr = (float *) malloc(sizeof(float)* MAX_PHASER_STAGES * 2);

    lfo = new EffectLFO(sample_rate);

    Ppreset = 0;
    PERIOD = 256; //best guess until the effect starts running;
    setpreset (Ppreset);
    cleanup ();
};

Phaser::~Phaser ()
{
    free(oldl);
    free(oldr);
	delete lfo;
};


/*
 * Effect output
 */
void
Phaser::out (float * smpsl, float * smpsr, uint32_t period)
{
    unsigned int i;
    int j;
    float lfol, lfor, lgain, rgain, tmp;

    lfo->effectlfoout (&lfol, &lfor);
    lgain = lfol;
    rgain = lfor;
    lgain =
        (expf (lgain * PHASER_LFO_SHAPE) - 1.0f) / (expf (PHASER_LFO_SHAPE) - 1.0f);
    rgain =
        (expf (rgain * PHASER_LFO_SHAPE) - 1.0f) / (expf (PHASER_LFO_SHAPE) - 1.0f);


    lgain = 1.0f - phase * (1.0f - depth) - (1.0f - phase) * lgain * depth;
    rgain = 1.0f - phase * (1.0f - depth) - (1.0f - phase) * rgain * depth;

    if (lgain > 1.0)
        lgain = 1.0f;
    else if (lgain < 0.0)
        lgain = 0.0f;
    if (rgain > 1.0)
        rgain = 1.0f;
    else if (rgain < 0.0)
        rgain = 0.0f;

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


    for (i = 0; i < period; i++) {
        float x = (float) i / ((float)period);
        float x1 = 1.0f - x;
        float gl = lgain * x + oldlgain * x1;
        float gr = rgain * x + oldrgain * x1;
        float inl = smpsl[i] * panning + fbl;
        float inr = smpsr[i] * (1.0f - panning) + fbr;

        //Left channel
        for (j = 0; j < Pstages * 2; j++) {
            //Phasing routine
            tmp = oldl[j] + DENORMAL_GUARD;
            oldl[j] = gl * tmp + inl;
            inl = tmp - gl * oldl[j];
        };
        //Right channel
        for (j = 0; j < Pstages * 2; j++) {
            //Phasing routine
            tmp = oldr[j] + DENORMAL_GUARD;
            oldr[j] = (gr * tmp) + inr;
            inr = tmp - (gr * oldr[j]);
        };
        //Left/Right crossing
        float l = inl;
        float r = inr;
        inl = l * (1.0f - lrcross) + r * lrcross;
        inr = r * (1.0f - lrcross) + l * lrcross;

        fbl = inl * fb;
        fbr = inr * fb;

        if (Poutsub != 0) {
            inl *= -1.0f;
            inr *= -1.0f;
        }

        efxoutl[i] = smpsl[i] * v1 + inl * v2;
        efxoutr[i] = smpsr[i] * v1 + inr * v2;

    };

    oldlgain = lgain;
    oldrgain = rgain;
};

/*
 * Cleanup the effect
 */
void
Phaser::cleanup ()
{
    fbl = 0.0;
    fbr = 0.0;
    oldlgain = 0.0;
    oldrgain = 0.0;
    for (int i = 0; i < Pstages * 2; i++) {
        oldl[i] = 0.0;
        oldr[i] = 0.0;
    };
};

/*
 * Parameter control
 */
void
Phaser::setdepth (int Pdepth)
{
    this->Pdepth = Pdepth;
    depth = ((float)Pdepth / 127.0f);
};


void
Phaser::setfb (int Pfb)
{
    this->Pfb = Pfb;
    fb = ((float)Pfb - 64.0f) / 64.1f;
};

void
Phaser::setvolume (int Pvolume)
{
    this->Pvolume = Pvolume;
    outvolume = (float)Pvolume / 127.0f;
};

void
Phaser::setpanning (int Ppanning)
{
    this->Ppanning = Ppanning;
    panning = ((float)Ppanning + .5f)/ 127.0f;
};

void
Phaser::setlrcross (int Plrcross)
{
    this->Plrcross = Plrcross;
    lrcross = (float)Plrcross / 127.0f;
};

void
Phaser::setstages (int Pstages)
{
    if (Pstages > MAX_PHASER_STAGES)
        Pstages = MAX_PHASER_STAGES;
    this->Pstages = Pstages;
    cleanup ();
};

void
Phaser::setphase (int Pphase)
{
    this->Pphase = Pphase;
    phase = ((float)Pphase / 127.0f);
};


void
Phaser::setpreset (int npreset)
{
    const int PRESET_SIZE = 12;
    const int NUM_PRESETS = 6;
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //Phaser1
        {64, 64, 11, 0, 0, 64, 110, 64, 1, 0, 0, 20},
        //Phaser2
        {64, 64, 10, 0, 0, 88, 40, 64, 3, 0, 0, 20},
        //Phaser3
        {64, 64, 8, 0, 0, 66, 68, 107, 2, 0, 0, 20},
        //Phaser4
        {39, 64, 1, 0, 0, 66, 67, 10, 5, 0, 1, 20},
        //Phaser5
        {64, 64, 1, 0, 1, 110, 67, 78, 10, 0, 0, 20},
        //Phaser6
        {64, 64, 31, 100, 0, 58, 37, 78, 3, 0, 0, 20}
    };

    if(npreset>NUM_PRESETS-1) {

    } else {
        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, presets[npreset][n]);
    }
    Ppreset = npreset;
};


void
Phaser::changepar (int npar, int value)
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
        setfb (value);
        break;
    case 8:
        setstages (value);
        break;
    case 9:
        setlrcross (value);
        break;
    case 10:
        if (value > 1)
            value = 1;
        Poutsub = value;
        break;
    case 11:
        setphase (value);
        break;
    }
};

int
Phaser::getpar (int npar)
{
    switch (npar) {
    case 0:
        return (Pvolume);
        break;
    case 1:
        return (Ppanning);
        break;
    case 2:
        return (lfo->Pfreq);	// tempo
        break;
    case 3:
        return (lfo->Prandomness);
        break;
    case 4:
        return (lfo->PLFOtype);
        break;
    case 5:
        return (lfo->Pstereo);	// STDL
        break;
    case 6:
        return (Pdepth);
        break;
    case 7:
        return (Pfb);			// pfb feedback
        break;
    case 8:
        return (Pstages);
        break;
    case 9:
        return (Plrcross);
        break;
    case 10:
        return (Poutsub);
        break;
    case 11:
        return (Pphase);
        break;
    default:
        return (0);
    }
};
