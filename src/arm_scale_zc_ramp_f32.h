#pragma once

#include "arm_math_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
* @brief Scale a floating-point vector with a scalar and scalar2 after a zero cross.
* @param[in]  pSrc      points to the input vector 1
* @param[inout]  pScale    points to actual scale scalar
* @param[in]  dScale    destination scale scalar
* @param[out] pDst       points to the output vector
* @param[in]  blockSize  number of samples in the vector
*/
void arm_scale_zc_ramp_f32(const float32_t * pSrc, float32_t * pScale, float32_t dScale, float32_t * pDst, uint32_t blockSize);

#ifdef __cplusplus
}
#endif
