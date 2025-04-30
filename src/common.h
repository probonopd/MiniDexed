#ifndef _common_h
#define _common_h

#include <stdint.h>

inline long maplong(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

inline float32_t mapfloat(float32_t val, float32_t in_min, float32_t in_max, float32_t out_min, float32_t out_max)
{
  return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

inline float32_t mapfloat(int val, int in_min, int in_max, float32_t out_min, float32_t out_max)
{
  return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

template<typename T>
// Code that used to use constrain shall use clamp instead
// This was renamed to avoid conflicts with the Synth_Dexed submodule
inline T clamp(T amt, T low, T high) {
    return (amt < low) ? low : ((amt > high) ? high : amt);
}

#endif
