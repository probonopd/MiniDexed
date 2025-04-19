#pragma once

#include "arm_math_types.h"

typedef int32_t q23_t;

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Converts the elements of the floating-point vector to Q23 vector.
 * @param[in]  pSrc       points to the floating-point input vector
 * @param[out] pDst       points to the Q23 output vector
 * @param[in]  blockSize  length of the input vector
 */
void arm_float_to_q23(const float32_t * pSrc, q23_t * pDst, uint32_t blockSize);

#ifdef __cplusplus
}
#endif
