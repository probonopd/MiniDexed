
#ifndef _common_h
#define _common_h

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

#ifndef _constrain_defined_
#define _constrain_defined_
template<class T>
const T& constrain(const T& x, const T& a, const T& b) {
    if(x < a) {
        return a;
    }
    else if(b < x) {
        return b;
    }
    else
        return x;
}
#endif

#endif
