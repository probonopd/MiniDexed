#include "arm_scale_zip_f32.h"

/**
  Scale two vectors and zip after.  For floating-point data, the algorithm used is:

  <pre>
      pDst[n] = pSrc1[n] * scale, pDst[n+1] = pSrc2[n] * scale   0 <= n < blockSize.
  </pre>

 */

/**
* @brief Scale two floating-point vector with a scalar and zip after.
* @param[in]  pSrc1      points to the input vector 1
* @param[in]  pSrc2      points to the input vector 2
* @param[in]  scale      scale scalar
* @param[out] pDst       points to the output vector
* @param[in]  blockSize  number of samples in the vector
*/

#if defined(ARM_MATH_NEON_EXPERIMENTAL)
void arm_scale_zip_f32(
  const float32_t * pSrc1,
  const float32_t * pSrc2,
        float32_t scale,
        float32_t * pDst,
        uint32_t blockSize)
{
    uint32_t blkCnt;                               /* Loop counter */

    f32x2x2_t res;

    /* Compute 2 outputs at a time */
    blkCnt = blockSize >> 1U;

    while (blkCnt > 0U)
    {
        res.val[0] = vmul_n_f32(vld1_f32(pSrc1), scale);
        res.val[1] = vmul_n_f32(vld1_f32(pSrc2), scale);
        vst2_f32(pDst, res);

        /* Increment pointers */
        pSrc1 += 2;
        pSrc2 += 2;
        pDst += 4;
        
        /* Decrement the loop counter */
        blkCnt--;
    }

    /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
    ** No loop unrolling is used. */
    blkCnt = blockSize & 1;

    while (blkCnt > 0U)
    {
        *pDst++ = *pSrc1++ * scale;
        *pDst++ = *pSrc2++ * scale;

        /* Decrement the loop counter */
        blkCnt--;
    }
}
#else
void arm_scale_zip_f32(
  const float32_t * pSrc1,
  const float32_t * pSrc2,
        float32_t scale,
        float32_t * pDst,
        uint32_t blockSize)
{
  uint32_t blkCnt;                               /* Loop counter */

  blkCnt = blockSize;

  while (blkCnt > 0U)
  {
      *pDst++ = *pSrc1++ * scale;
      *pDst++ = *pSrc2++ * scale;
      
      /* Decrement the loop counter */
      blkCnt--;
  }
}
#endif
