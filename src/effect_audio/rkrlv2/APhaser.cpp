/*
  APhaser.C  - Approximate digital model of an analog JFET phaser.
  Analog modeling implemented by Ryan Billing aka Transmogrifox.
  November, 2009

  Credit to:
  ///////////////////
  ZynAddSubFX - a software synthesizer

  Phaser.C - Phaser effect
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul

  Modified for rakarrack by Josep Andreu

  DSP analog modeling theory & practice largely influenced by various CCRMA publications, particularly works by Julius O. Smith.
  ////////////////////


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
#include "APhaser.h"
#include <stdio.h>
//#include "FPreset.h"
#define PHASER_LFO_SHAPE 2
#define ONE_  0.99999f        // To prevent LFO ever reaching 1.0 for filter stability purposes
#define ZERO_ 0.00001f        // Same idea as above.

Analog_Phaser::Analog_Phaser (float * efxoutl_, float * efxoutr_, double sample_rate)
{
    float fSAMPLE_RATE = sample_rate;

    efxoutl = efxoutl_;
    efxoutr = efxoutr_;

    lxn1 = (float *) malloc(sizeof(float)* MAX_PHASER_STAGES);

    lyn1 = (float *) malloc(sizeof(float)* MAX_PHASER_STAGES);

    rxn1 = (float *) malloc(sizeof(float)* MAX_PHASER_STAGES);

    ryn1 = (float *) malloc(sizeof(float)* MAX_PHASER_STAGES);

    offset = (float *) malloc(sizeof(float)* MAX_PHASER_STAGES);	//model mismatch between JFET devices
    offset[0] = -0.2509303f;
    offset[1] = 0.9408924f;
    offset[2] = 0.998f;
    offset[3] = -0.3486182f;
    offset[4] = -0.2762545f;
    offset[5] = -0.5215785f;
    offset[6] = 0.2509303f;
    offset[7] = -0.9408924f;
    offset[8] = -0.998f;
    offset[9] = 0.3486182f;
    offset[10] = 0.2762545f;
    offset[11] = 0.5215785f;

    barber = 0;  //Deactivate barber pole phasing by default

    mis = 1.0f;
    Rmin = 625.0f;	// 2N5457 typical on resistance at Vgs = 0
    Rmax = 22000.0f;	// Resistor parallel to FET
    Rmx = Rmin/Rmax;
    Rconst = 1.0f + Rmx;  // Handle parallel resistor relationship
    C = 0.00000005f;	     // 50 nF
    CFs = 2.0f*fSAMPLE_RATE*C;

    lfo = new EffectLFO(sample_rate);

    Ppreset = 0;
    PERIOD = 255; //make best guess for init;
    setpreset (Ppreset);//this will get done before out is run
    cleanup ();
};

Analog_Phaser::~Analog_Phaser ()
{
    free(lxn1);
    free(lyn1);
    free(rxn1);
    free(ryn1);
    free(offset);
	delete lfo;
};


/*
 * Effect output
 */
void
Analog_Phaser::out (float * smpsl, float * smpsr, uint32_t period)
{
    unsigned int i;
    int j;
    float lfol, lfor, lgain, rgain, bl, br, gl, gr, rmod, lmod, d, hpfr, hpfl;
    invperiod = 1.0f / (float)PERIOD;//had to move this to run
    lgain = 0.0;
    rgain = 0.0;

    //initialize hpf
    hpfl = 0.0;
    hpfr = 0.0;

    lfo->effectlfoout (&lfol, &lfor);
    lmod = lfol*width + depth;
    rmod = lfor*width + depth;

    if (lmod > ONE_)
        lmod = ONE_;
    else if (lmod < ZERO_)
        lmod = ZERO_;
    if (rmod > ONE_)
        rmod = ONE_;
    else if (rmod < ZERO_)
        rmod = ZERO_;

    if (Phyper != 0) {
        lmod *= lmod;  //Triangle wave squared is approximately sin on bottom, tri on top
        rmod *= rmod;  //Result is exponential sweep more akin to filter in synth with exponential generator circuitry.
    };

    lmod = sqrtf(1.0f - lmod);  //gl,gr is Vp - Vgs. Typical FET drain-source resistance follows constant/[1-sqrt(Vp - Vgs)]
    rmod = sqrtf(1.0f - rmod);

    rdiff = (rmod - oldrgain) * invperiod;
    ldiff = (lmod - oldlgain) * invperiod;

    gl = oldlgain;
    gr = oldrgain;

    oldlgain = lmod;
    oldrgain = rmod;

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

        gl += ldiff;	// Linear interpolation between LFO samples
        gr += rdiff;

        float lxn = smpsl[i];
        float rxn = smpsr[i];


        if (barber) {
            gl = fmodf((gl + 0.25f) , ONE_);
            gr = fmodf((gr + 0.25f) , ONE_);
        };


        //Left channel
        for (j = 0; j < Pstages; j++) {
            //Phasing routine
            mis = 1.0f + offsetpct*offset[j];
            d = (1.0f + 2.0f*(0.25f + gl)*hpfl*hpfl*distortion) * mis;  //This is symmetrical. FET is not, so this deviates slightly, however sym dist. is better sounding than a real FET.
            Rconst =  1.0f + mis*Rmx;
            bl = (Rconst - gl )/ (d*Rmin);  // This is 1/R. R is being modulated to control filter fc.
            lgain = (CFs - bl)/(CFs + bl);

            lyn1[j] = lgain * (lxn + lyn1[j]) - lxn1[j];
            lyn1[j] += DENORMAL_GUARD;
            hpfl = lyn1[j] + (1.0f-lgain)*lxn1[j];  //high pass filter -- Distortion depends on the high-pass part of the AP stage.

            lxn1[j] = lxn;
            lxn = lyn1[j];
            if (j==1) lxn += fbl;  //Insert feedback after first phase stage
        };

        //Right channel
        for (j = 0; j < Pstages; j++) {
            //Phasing routine
            mis = 1.0f + offsetpct*offset[j];
            d = (1.0f + 2.0f*(0.25f + gr)*hpfr*hpfr*distortion) * mis;   // distortion
            Rconst =  1.0f + mis*Rmx;
            br = (Rconst - gr )/ (d*Rmin);
            rgain = (CFs - br)/(CFs + br);

            ryn1[j] = rgain * (rxn + ryn1[j]) - rxn1[j];
            ryn1[j] += DENORMAL_GUARD;
            hpfr = ryn1[j] + (1.0f-rgain)*rxn1[j];  //high pass filter

            rxn1[j] = rxn;
            rxn = ryn1[j];
            if (j==1) rxn += fbr;  //Insert feedback after first phase stage
        };

        fbl = lxn * fb;
        fbr = rxn * fb;

        if (Poutsub != 0) {
            lxn *= -1.0f;
            rxn *= -1.0f;
        }

        efxoutl[i] = smpsl[i] * v1 + lxn * v2;
        efxoutr[i] = smpsr[i] * v1 + rxn * v2;
    };
};

/*
 * Cleanup the effect
 */
void
Analog_Phaser::cleanup ()
{
    fbl = 0.0;
    fbr = 0.0;
    oldlgain = 0.0;
    oldrgain = 0.0;
    for (int i = 0; i < Pstages; i++) {
        lxn1[i] = 0.0;

        lyn1[i] = 0.0;

        rxn1[i] = 0.0;

        ryn1[i] = 0.0;

    };
};

/*
 * Parameter control
 */
void
Analog_Phaser::setwidth (int Pwidth)
{
    this->Pwidth = Pwidth;
    width = ((float)Pwidth / 127.0f);
};


void
Analog_Phaser::setfb (int Pfb)
{
    this->Pfb = Pfb;
    fb = (float) (Pfb - 64) / 64.2f;
};

void
Analog_Phaser::setvolume (int Pvolume)
{
    this->Pvolume = Pvolume;
    // outvolume is needed in calling program
    outvolume = (float)Pvolume / 127.0f;
};

void
Analog_Phaser::setdistortion (int Pdistortion)
{
    this->Pdistortion = Pdistortion;
    distortion = (float)Pdistortion / 127.0f;
};

void
Analog_Phaser::setoffset (int Poffset)
{
    this->Poffset = Poffset;
    offsetpct = (float)Poffset / 127.0f;
};

void
Analog_Phaser::setstages (int Pstages)
{

    if (Pstages >= MAX_PHASER_STAGES)
        Pstages = MAX_PHASER_STAGES ;
    this->Pstages = Pstages;

    cleanup ();
};

void
Analog_Phaser::setdepth (int Pdepth)
{
    this->Pdepth = Pdepth;
    depth = (float)(Pdepth - 64) / 127.0f;  //Pdepth input should be 0-127.  depth shall range 0-0.5 since we don't need to shift the full spectrum.
};


void
Analog_Phaser::setpreset (int npreset)
{
    const int PRESET_SIZE = 13;
    const int NUM_PRESETS = 6;
    int presets[NUM_PRESETS][PRESET_SIZE] = {
        //Phaser1
        {64, 20, 14, 0, 1, 64, 110, 40, 4, 10, 0, 64, 1},
        //Phaser2
        {64, 20, 14, 5, 1, 64, 110, 40, 6, 10, 0, 70, 1},
        //Phaser3
        {64, 20, 9, 0, 0, 64, 40, 40, 8, 10, 0, 60, 0},
        //Phaser4
        {64, 20, 14, 10, 0, 64, 110, 80, 7, 10, 1, 45, 1},
        //Phaser5
        {25, 20, 240, 10, 0, 64, 25, 16, 8, 100, 0, 25, 0},
        //Phaser6
        {64, 20, 1, 10, 1, 64, 110, 40, 12, 10, 0, 70, 1}
    };

    if(npreset>NUM_PRESETS-1) {

    } else {

        for (int n = 0; n < PRESET_SIZE; n++)
            changepar (n, presets[npreset][n]);
    }

    Ppreset = npreset;
};


void
Analog_Phaser::changepar (int npar, int value)
{
    switch (npar) {
    case 0:
        setvolume (value);
        break;
    case 1:
        setdistortion (value);
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
        barber = 0;
        if (value == 2) barber = 1;
        break;
    case 5:
        lfo->Pstereo = value;
        lfo->updateparams (PERIOD);
        break;
    case 6:
        setwidth (value);
        break;
    case 7:
        setfb (value);
        break;
    case 8:
        setstages (value);
        break;
    case 9:
        setoffset (value);
        break;
    case 10:
        if (value > 1)
            value = 1;
        Poutsub = value;
        break;
    case 11:
        setdepth (value);
        break;
    case 12:
        if (value > 1)
            value = 1;
        Phyper = value;
        break;
    };
};

int
Analog_Phaser::getpar (int npar)
{
    switch (npar) {
    case 0:
        return (Pvolume);
        break;
    case 1:
        return (Pdistortion);
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
        return (Pwidth);
        break;
    case 7:
        return (Pfb);
        break;
    case 8:
        return (Pstages);
        break;
    case 9:
        return (Poffset);
        break;
    case 10:
        return (Poutsub);
        break;
    case 11:
        return (Pdepth);
        break;
    case 12:
        return (Phyper);
        break;

    default:
        return (0);
    };

};
