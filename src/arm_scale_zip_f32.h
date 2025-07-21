#pragma once

#include "arm_math_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
* @brief Scale two floating-point vector with a scalar and zip after.
* @param[in]  pSrc1      points to the input vector 1
* @param[in]  pSrc2      points to the input vector 2
* @param[in]  scale      scale scalar
* @param[out] pDst       points to the output vector
* @param[in]  blockSize  number of samples in the vector
*/
void arm_scale_zip_f32(const float32_t * pSrc1, const float32_t * pSrc2, float32_t scale, float32_t * pDst, uint32_t blockSize);

#ifdef __cplusplus
}
#endif
