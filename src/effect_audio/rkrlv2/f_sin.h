/*
Cubic Sine Approximation based upon a modified Taylor Series Expansion
Author: Ryan Billing (C) 2010

This is unlicensed.  Do whatever you want with but use at your own disgression.
The author makes no guarantee of its suitability for any purpose.
*/
#ifndef FSIN_H
#define FSIN_H

#include <arm_math.h>
#include "global.h"

//globals
static const float p2 = PI/2.0f;
static const float fact3 = 0.148148148148148f; //can multiply by 1/fact3

static inline float
f_sin(float x)
{

    float y;  //function output
    float tmp;
    bool sign;
    if ((x>D_PI) || (x<-D_PI)) x = fmod(x,D_PI);
    if (x < 0.0f) x+=D_PI;
    sign = 0;
    if(x>PI) {
        x = D_PI - x;
        sign = 1;
    }

    if (x <= p2) y = x - x*x*x*fact3;
    else {
        tmp = x - PI;
        y = -tmp + tmp*tmp*tmp*fact3;
    }

    if (sign) y = -y;

    return y;
}

static inline float
f_cos(float x_)
{
    return f_sin(p2 + x_);
}

#endif
