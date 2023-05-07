#include <arm_math.h>

float32_t arm_sin_f32(float32_t phase)
{
    return sin(phase);
}

float32_t arm_cos_f32(float32_t phase)
{
    return cos(phase);
}

void arm_scale_f32(const float32_t *pSrc, float32_t scale, float32_t *pDst, uint32_t blockSize)
{
    for(unsigned i = 0; i < blockSize; ++i)
    {
        pDst[i] = scale * pSrc[i];
    }
}

void arm_copy_f32(const float32_t *pSrc, float32_t *pDst, uint32_t blockSize)
{
    memcpy(pDst, pSrc, blockSize * sizeof(float32_t));
}

void arm_add_f32(const float32_t *pSrcA, const float32_t *pSrcB, float32_t *pDst, uint32_t blockSize)
{
    for(size_t i = 0; i < blockSize; ++i) pDst[i] = pSrcA[i] + pSrcB[i];
}

void arm_fill_f32(float32_t value, float32_t *pDst, uint32_t blockSize)
{
    for(size_t i = 0; i < blockSize; ++i) pDst[i] = value;
}

float32_t arm_weighted_sum_f32(const float32_t *in, const float32_t *weights, uint32_t blockSize)
{
    float32_t s = 0.0f;
    float32_t w = 0.0f;

    for(size_t i = 0; i < blockSize; ++i)
    {
        s += in[i] * weights[i];
        w += weights[i];
    }

    return s / w;
}

void arm_clip_f32(const float32_t *pSrc, float32_t *pDst, float32_t low, float32_t high, uint32_t numSamples)
{
    for(size_t i = 0; i < numSamples; ++i) pDst[i] = (pSrc[i] < low) ? low : (pSrc[i] > high) ? high : pSrc[i];
}