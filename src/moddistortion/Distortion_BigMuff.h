#ifndef _DISTORTION_BIGMUFF_H
#define _DISTORTION_BIGMUFF_H

#include <cmath>

void BM_Filter1(float *u, float *y, int N, double T, float *U_1, float *Y_1 );
void BM_Filter2(float *u, float *y, int N, double T, float *U_1, float *Y_1, float *U_2, float *Y_2, float*U_3, float *Y_3 );
void BM_Filter3(float *u, float *y, int N, double T, float *U_1, float *Y_1, float *U_2, float *Y_2, double x, double x_1 );
void BM_Clip(float *u, float *y, int N, double T, float *U_1, float *Y_1 );
void BM_Filter4(float *u, float *y, int N, double T, float *U_1, float *Y_1, float *U_2, float *Y_2, float *U_3, float *Y_3, double tone, double vol );

#endif // _DISTORTION_BIGMUFF_H