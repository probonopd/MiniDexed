/*
  ZynAddSubFX - a software synthesizer

  APhaser.h - Phaser effect
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul

  Modified for rakarrack by Josep Andreu and Ryan Billing

  Further modified for rakarrack by Ryan Billing (Transmogrifox) to model Analog Phaser behavior 2009

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
#ifndef APHASER_H
#define APHASER_H
#include "global.h"
#include "EffectLFO.h"


class Analog_Phaser
{
public:
    Analog_Phaser (float * efxoutl_, float * efxoutr_, double sample_rate);
    ~Analog_Phaser ();
    void out (float * smpsl, float * smpsr, uint32_t period);
    void setpreset (int npreset);
    void changepar (int npar, int value);
    int getpar (int npar);
    void cleanup ();
    int Ppreset;
    float *efxoutl;
    float *efxoutr;
    float outvolume;

    uint32_t PERIOD;
    EffectLFO *lfo;		//Phaser modulator

private:
    //Phaser parameters
    int Pvolume;        //Used in Process.C to set wet/dry mix
    int Pdistortion;    //Model distortion added by FET element
    int Pwidth;		//Phaser width (LFO amplitude)
    int Pfb;		//feedback
    int Poffset;	//Model mismatch between variable resistors
    int Pstages;	//Number of first-order All-Pass stages
    int Poutsub;	//if I wish to subtract the output instead of the adding it
    int Phyper;		//lfo^2 -- converts tri into hyper-sine
    int Pdepth;         //Depth of phaser sweep
    int Pbarber;         //Enable barber pole phasing

    //Control parameters
    void setvolume (int Pvolume);
    void setdistortion (int Pdistortion);
    void setwidth (int Pwidth);
    void setfb (int Pfb);
    void setoffset (int Poffset);
    void setstages (int Pstages);
    void setdepth (int Pdepth);

    //Internal Variables
    bool barber;			//Barber pole phasing flag
    float distortion, fb, width, offsetpct, fbl, fbr, depth;
    float *lxn1, *lyn1,*rxn1, *ryn1, *offset;
    float oldlgain, oldrgain, rdiff, ldiff, invperiod;

    float mis;
    float Rmin;	// 2N5457 typical on resistance at Vgs = 0
    float Rmax;	// Resistor parallel to FET
    float Rmx;		// Rmin/Rmax to avoid division in loop
    float Rconst;      // Handle parallel resistor relationship
    float C;	        // Capacitor
    float CFs;		// A constant derived from capacitor and resistor relationships
    float fPERIOD;

    class FPreset *Fpre;

};

#endif