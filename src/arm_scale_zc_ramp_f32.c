#include "arm_scale_zc_ramp_f32.h"

const float32_t RAMP_DT = 1.0 / 254;
const float32_t RAMP_EPS = 1.0 / 127;

/**
* @brief Scale a floating-point vector with a scalar and scalar2 after a zero cross.
* @param[in]  pSrc      points to the input vector 1
* @param[in]  scale1     scale1 scalar
* @param[in]  scale2     scale2 scalar
* @param[out] pDst       points to the output vector
* @param[in]  blockSize  number of samples in the vector
*/

#if defined(ARM_MATH_NEON_EXPERIMENTAL)

static inline int
v_any_u32(uint32x4_t d)
{
  return vpaddd_u64(vreinterpretq_u64_u32(d)) != 0;
}

void arm_scale_zc_ramp_f32(
  const float32_t * pSrc,
        float32_t * pScale,
        float32_t dScale,
        float32_t * pDst,
        uint32_t blockSize)
{
  uint32_t blkCnt;                               /* Loop counter */
  float32_t scale = *pScale;

  f32x4_t res;

  blkCnt = blockSize >> 2U;

  while (blkCnt > 0U)
  {
    res = vmulq_n_f32(vld1q_f32(pSrc), scale);
    vst1q_f32(pDst, res);

    pSrc += 4;
    pDst += 4;
  
    /* Decrement the loop counter */
    blkCnt--;

    if (scale != dScale && blkCnt)
    {
      f32x4_t resn1 = vld1q_f32(pSrc-3);

      // this probably runs faster, but does not find all zc
      // so ramping is a bit slower
      //if (vminnmvq_f32(res) <= 0 && vmaxnmvq_f32(res) >= 0)

      if (v_any_u32(vorrq_u32(
          vandq_u32(vcgezq_f32(res), vclezq_f32(resn1)),
          vandq_u32(vclezq_f32(res), vcgezq_f32(resn1))
      )))
      {
        scale += dScale > scale ? RAMP_DT : -RAMP_DT;
        if (fabs(dScale - scale) < RAMP_EPS) scale = dScale;
      }
    }
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
  ** No loop unrolling is used. */
  blkCnt = blockSize & 3;

  while (blkCnt > 0U)
  {
    *pDst++ = *pSrc++ * scale;

    /* Decrement the loop counter */
    blkCnt--;

    if (blkCnt && scale != dScale && (*(pSrc-1) <= 0 && *pSrc >= 0 || *(pSrc-1) >= 0 && *pSrc <= 0))
    {
      scale += dScale > scale ? RAMP_DT : -RAMP_DT;
      if (fabs(dScale - scale) < RAMP_EPS) scale = dScale;
    }    
  }

  *pScale = scale;
}

#else

void arm_scale_zc_ramp_f32(
  const float32_t * pSrc,
        float32_t * pScale,
        float32_t dScale,
        float32_t * pDst,
        uint32_t blockSize)
{
  uint32_t blkCnt;                               /* Loop counter */
  float32_t scale = *pScale;

  blkCnt = blockSize;

  while (blkCnt > 0U)
  {
    *pDst++ = *pSrc++ * scale;
  
    /* Decrement the loop counter */
    blkCnt--;

    if (blkCnt && scale != dScale && (*(pSrc-1) <= 0 && *pSrc >= 0 || *(pSrc-1) >= 0 && *pSrc <= 0))
    {
      scale += dScale > scale ? RAMP_DT : -RAMP_DT;
      if (fabs(dScale - scale) < RAMP_EPS) scale = dScale;
    }
  }

  *pScale = scale;
}

#endif
