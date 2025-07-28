#include "arm_scale_zip_f32_to_q23.h"

/**
* @brief Scale two floating-point vector with a scalar and zip after.
* @param[in]  pSrc1      points to the input vector 1
* @param[in]  pSrc2      points to the input vector 2
* @param[in]  scale      scale scalar
* @param[out] pDst       points to the output vector
* @param[in]  blockSize  number of samples in the vector
*/

#if defined(ARM_MATH_NEON_EXPERIMENTAL)
void arm_scale_zip_f32_to_q23(
  const float32_t * pSrc1,
  const float32_t * pSrc2,
        float32_t scale,
        q23_t * pDst,
        uint32_t blockSize)
{
    uint32_t blkCnt;                               /* Loop counter */

    int32x4x2_t res;

    /* Compute 4 outputs at a time */
    blkCnt = blockSize >> 2U;

    while (blkCnt > 0U)
    {
        res.val[0] = vcvtq_n_s32_f32(vmulq_n_f32(vld1q_f32(pSrc1), scale), 23);
        res.val[0] = vminq_s32(res.val[0], vdupq_n_s32(0x007fffff));
        res.val[0] = vmaxq_s32(res.val[0], vdupq_n_s32(0xff800000));

        res.val[1] = vcvtq_n_s32_f32(vmulq_n_f32(vld1q_f32(pSrc2), scale), 23);
        res.val[1] = vminq_s32(res.val[1], vdupq_n_s32(0x007fffff));
        res.val[1] = vmaxq_s32(res.val[1], vdupq_n_s32(0xff800000));

        vst2q_s32(pDst, res);

        /* Increment pointers */
        pSrc1 += 4;
        pSrc2 += 4;
        pDst += 8;
        
        /* Decrement the loop counter */
        blkCnt--;
    }

    /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
    ** No loop unrolling is used. */
    blkCnt = blockSize & 3;

    while (blkCnt > 0U)
    {
        *pDst++ = (q23_t) __SSAT((q31_t) (*pSrc1++ * scale * 8388608.0f), 24);
        *pDst++ = (q23_t) __SSAT((q31_t) (*pSrc2++ * scale * 8388608.0f), 24);

        /* Decrement the loop counter */
        blkCnt--;
    }
}
#else
void arm_scale_zip_f32_to_q23(
  const float32_t * pSrc1,
  const float32_t * pSrc2,
        float32_t scale,
        q23_t * pDst,
        uint32_t blockSize)
{
  uint32_t blkCnt;                               /* Loop counter */

  blkCnt = blockSize;

  while (blkCnt > 0U)
  {
      *pDst++ = (q23_t) __SSAT((q31_t) (*pSrc1++ * scale * 8388608.0f), 24);
      *pDst++ = (q23_t) __SSAT((q31_t) (*pSrc2++ * scale * 8388608.0f), 24);

      /* Decrement the loop counter */
      blkCnt--;
  }
}
#endif
