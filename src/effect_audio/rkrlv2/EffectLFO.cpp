/*
  ZynAddSubFX - a software synthesizer

  EffectLFO.C - Stereo LFO used by some effects
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul

  Modified for rakarrack by Josep Andreu 6 Ryan Billing


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

#include <stdlib.h>
#include <stdio.h>
#include <arm_math.h>

#include "global.h"
#include "EffectLFO.h"
#include "f_sin.h"

EffectLFO::EffectLFO (double sample_rate)
{
    float fPERIOD = 256;//this is our best guess at what it will be, later we'll correct it when we actually know fPERIOD
    fSAMPLE_RATE = sample_rate;
    xl = 0.0;
    xr = 0.0;
    Pfreq = 40;
    Prandomness = 0;
    PLFOtype = 0;
    Pstereo = 96;

    iperiod = fPERIOD/fSAMPLE_RATE;
    h = iperiod;
    a = 10.0f;
    b = 28.0f;
    c = 8.0f / 5.0f;
    scale = 1.0f/36.0f;
    ratediv = 0.1f;
    holdflag = 0;
    tca = iperiod/(iperiod + 0.02);  //20ms default
    tcb = 1.0f - tca;
    rreg = lreg = oldrreg = oldlreg = 0.0f;
    updateparams ((uint32_t)fPERIOD);

    ampl1 = (1.0f - lfornd) + lfornd * (float)RND;
    ampl2 = (1.0f - lfornd) + lfornd * (float)RND;
    ampr1 = (1.0f - lfornd) + lfornd * (float)RND;
    ampr2 = (1.0f - lfornd) + lfornd * (float)RND;


};

EffectLFO::~EffectLFO ()
{
};


/*
 * Update the changed parameters
 */
void
EffectLFO::updateparams (uint32_t period)
{
    float fPERIOD = period;
    //must update several parameters once we actually know the period
    iperiod = fPERIOD/fSAMPLE_RATE;
    h = iperiod;
    tca = iperiod/(iperiod + 0.02);  //20ms default
    tcb = 1.0f - tca;


    incx = (float)Pfreq * fPERIOD / (fSAMPLE_RATE * 60.0f);

    if (incx > 0.49999999)
        incx = 0.499999999f;		//Limit the Frequency

    lfornd = (float)Prandomness / 127.0f;
    if (lfornd < 0.0)
        lfornd = 0.0;
    else if (lfornd > 1.0)
        lfornd = 1.0;

    if (PLFOtype > 11)   //this has to be updated if more lfo's are added
        PLFOtype = 0;		
    lfotype = PLFOtype;

    xr = fmodf (xl + ((float)Pstereo - 64.0f) / 127.0f + 1.0f, 1.0f);

    if ((h = incx*ratediv) > 0.02) h = 0.02;  //keeps it stable

    a = 10.0f + (((float) RND) - 0.5f)*8.0f;
    b = 28.0f + (((float) RND) - 0.5f)*12.0f;
    c = 1.25f + 3.0f * ((float) RND);

// printf("incx %f x0 %f y0 %f z0 %f out %f c %f b %f a %f\n",incx,x0,y0,z0, (2.0f * radius - 1.0f), c, b, a);
    x0 = 0.1f + 0.1f * ((float) RND);
    y0 = 0.0f;
    z0 = 0.2f;
    x1 = y1 = z1 = radius = 0.0f;

    float tmp = 6.0f/((float) Pfreq);  //S/H time attack  0.2*60=12.0
    tca = iperiod/(iperiod + tmp);  //
    tcb = 1.0f - tca;
    maxrate = 4.0f*iperiod;
};


/*
 * Compute the shape of the LFO
 */
float EffectLFO::getlfoshape (float x)
{
    float tmpv;
    float out=0.0;
    int iterations = 1;  //make fractal go faster
    switch (lfotype) {
    case 1:			//EffectLFO_TRIANGLE
        if ((x > 0.0) && (x < 0.25))
            out = 4.0f * x;
        else if ((x > 0.25) && (x < 0.75))
            out = 2.0f - 4.0f * x;
        else
            out = 4.0f * x - 4.0f;
        break;
    case 2:			//EffectLFO_RAMP Ramp+
        out = 2.0f * x - 1.0f;
        break;
    case 3:			//EffectLFO_RAMP Ramp-
        out = - 2.0f * x + 1.0f;
        break;
    case 4:                     //ZigZag
        x = x * 2.0f - 1.0f;
        tmpv = 0.33f * f_sin(x);
        out = f_sin(f_sin(x*D_PI)*x/tmpv);
        break;
    case 5:                     //Modulated Square ?? ;-)
        tmpv = x * D_PI;
        out=f_sin(tmpv+f_sin(2.0f*tmpv));
        break;
    case 6:                     // Modulated Saw
        tmpv = x * D_PI;
        out=f_sin(tmpv+f_sin(tmpv));
        break;
    case 8:                       //Lorenz Fractal, faster, using X,Y outputs
        iterations = 4;
    case 7:			// Lorenz Fractal
        for(int j=0; j<iterations; j++) {
            x1 = x0 + h * a * (y0 - x0);
            y1 = y0 + h * (x0 * (b - z0) - y0);
            z1 = z0 + h * (x0 * y0 - c * z0);
            x0 = x1;
            y0 = y1;
            z0 = z1;
        }
        if(lfotype==7) {
            if((radius = (sqrtf(x0*x0 + y0*y0 + z0*z0) * scale) - 0.25f)  > 1.0f) radius = 1.0f;
            if(radius < 0.0) radius = 0.0;
            out = 2.0f * radius - 1.0f;
        }

        break;
    case 9:                  //Sample/Hold Random
        if(fmod(x,0.5f)<=(2.0f*incx)) {          //this function is called by left, then right...so must toggle each time called
            rreg = lreg;
            lreg = RND1;

        }

        if(xlreg<lreg) xlreg += maxrate;
        else xlreg -= maxrate;
        if(xrreg<rreg) xrreg += maxrate;
        else xrreg -= maxrate;
        oldlreg = xlreg*tca + oldlreg*tcb;
        oldrreg = xrreg*tca + oldrreg*tcb;

        if(holdflag) {
            out = 2.0f*oldlreg -1.0f;
            holdflag = (1 + holdflag)%2;
        } else {
            out = 2.0f*oldrreg - 1.0f;
        }


        break;
    case 10:   //Tri-top
        if(x<=0.5f) out = -f_sin(x*D_PI);
        else if ((x > 0.5f) && (x < 0.75f))
        out = 6 * (x-0.5);
        else
        out = 1.5 - 6.0f *( x - 0.75f);
        out-=0.25f;
        out*=0.88888889f;
    break;
    case 11:  //Tri-Bottom
        if(x<=0.5f) out = -f_sin(x*D_PI);
        else if ((x > 0.5f) && (x < 0.75f))
        out = 6 * (x-0.5);
        else
        out = 1.5 - 6.0f *( x - 0.75f);
        out-=0.25f;
        out*=-0.88888889f;
    break; 
        //more to be added here; also ::updateparams() need to be updated (to allow more lfotypes)
    default:
        out = f_cos (x * D_PI);	//EffectLFO_SINE
    };
    return (out);
};

/*
 * LFO output
 */
void
EffectLFO::effectlfoout (float * outl, float * outr)
{
    float out;

    out = getlfoshape (xl);
    //if ((lfotype == 0) || (lfotype == 1))         //What was that for?
    out *= (ampl1 + xl * (ampl2 - ampl1));
    xl += incx;
    if (xl > 1.0) {
        xl -= 1.0f;
        ampl1 = ampl2;
        ampl2 = (1.0f - lfornd) + lfornd * (float)RND;
    };
    if(lfotype==8) out = scale*x0;  //fractal parameter
    *outl = (out + 1.0f) * 0.5f;


    if(lfotype==8) out = scale*y0;  //fractal parameter
    else out = getlfoshape (xr);

    //if ((lfotype == 0) || (lfotype == 1))
    out *= (ampr1 + xr * (ampr2 - ampr1));
    xr += incx;
    if (xr > 1.0) {
        xr -= 1.0f;
        ampr1 = ampr2;
        ampr2 = (1.0f - lfornd) + lfornd * (float)RND;
    };
    *outr = (out + 1.0f) * 0.5f;
};
