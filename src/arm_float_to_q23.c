#include "arm_float_to_q23.h"

#if defined(ARM_MATH_NEON_EXPERIMENTAL)
void arm_float_to_q23(const float32_t * pSrc, q23_t * pDst, uint32_t blockSize)
{
    const float32_t *pIn = pSrc;                   /* Src pointer */
    uint32_t blkCnt;                               /* loop counter */
    
    float32x4_t inV;

    int32x4_t cvt;

    blkCnt = blockSize >> 2U;

    /* Compute 4 outputs at a time.
    ** a second loop below computes the remaining 1 to 3 samples. */
    while (blkCnt > 0U)
    {
        /* C = A * 8388608 */
        /* Convert from float to q23 and then store the results in the destination buffer */
        inV = vld1q_f32(pIn);

        cvt = vcvtq_n_s32_f32(inV, 23);

        /* saturate */
        cvt = vminq_s32(cvt, vdupq_n_s32(0x007fffff));
        cvt = vmaxq_s32(cvt, vdupq_n_s32(0xff800000));

        vst1q_s32(pDst, cvt);
        pDst += 4;
        pIn += 4;

        /* Decrement the loop counter */
        blkCnt--;
    }

    /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
    ** No loop unrolling is used. */
    blkCnt = blockSize & 3;

    while (blkCnt > 0U)
    {
        /* C = A * 8388608 */
        /* Convert from float to q23 and then store the results in the destination buffer */
        *pDst++ = (q23_t) __SSAT((q31_t) (*pIn++ * 8388608.0f), 24);

        /* Decrement the loop counter */
        blkCnt--;
    }
}
#else
void arm_float_to_q23(const float32_t * pSrc, q23_t * pDst, uint32_t blockSize)
{
    uint32_t blkCnt;                /* Loop counter */
    const float32_t *pIn = pSrc;    /* Source pointer */

    /* Loop unrolling: Compute 4 outputs at a time */
    blkCnt = blockSize >> 2U;

    while (blkCnt > 0U)
    {
        /* C = A * 8388608 */
        /* convert from float to Q23 and store result in destination buffer */

        *pDst++ = (q23_t) __SSAT((q31_t) (*pIn++ * 8388608.0f), 24);
        *pDst++ = (q23_t) __SSAT((q31_t) (*pIn++ * 8388608.0f), 24);
        *pDst++ = (q23_t) __SSAT((q31_t) (*pIn++ * 8388608.0f), 24);
        *pDst++ = (q23_t) __SSAT((q31_t) (*pIn++ * 8388608.0f), 24);

        /* Decrement loop counter */
        blkCnt--;
    }

    /* Loop unrolling: Compute remaining outputs */
    blkCnt = blockSize % 0x4U;

    while (blkCnt > 0U)
    {
        /* C = A * 8388608 */
        /* Convert from float to q23 and then store the results in the destination buffer */
        *pDst++ = (q23_t) __SSAT((q31_t) (*pIn++ * 8388608.0f), 24);

        /* Decrement loop counter */
        blkCnt--;
    }

}
#endif /* #if defined(ARM_MATH_NEON_EXPERIMENTAL) */
