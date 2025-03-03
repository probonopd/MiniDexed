#ifndef _OVER_SAMPLE_H
#define _OVER_SAMPLE_H

#include <math.h>
#include <stdint.h>

void Over1_Float(float *in, float *u, float *u_1, uint32_t n);
void Over2_Float(float *in, float *u, float *u_1, uint32_t n);
void Over4_Float(float *in, float *u, float *u_1, uint32_t n);
void Over8_Float(float *in, float *u, float *u_1, uint32_t n);
void Over1(float *in, double *u, double *u_1, uint32_t n);
void Over2(float *in, double *u, double *u_1, uint32_t n);
void Over4(float *in, double *u, double *u_1, uint32_t n);
void Over8(float *in, double *u, double *u_1, uint32_t n);
void Over1_Double(double *in, double *u, double *u_1, uint32_t n);
void Over2_Double(double *in, double *u, double *u_1, uint32_t n);
void Over4_Double(double *in, double *u, double *u_1, uint32_t n);
void Over8_Double(double *in, double *u, double *u_1, uint32_t n);
void Down1_Float(float *out, float *y, uint32_t n);
void Down2_Float(float *out, float *y, uint32_t n);
void Down4_Float(float *out, float *y, uint32_t n);
void Down8_Float(float *out, float *y, uint32_t n);
void Down1(float *out, double *y, uint32_t n);
void Down2(float *out, double *y, uint32_t n);
void Down4(float *out, double *y, uint32_t n);
void Down8(float *out, double *y, uint32_t n);
void Down1_Double(double *out, double *y, uint32_t n);
void Down2_Double(double *out, double *y, uint32_t n);
void Down4_Double(double *out, double *y, uint32_t n);
void Down8_Double(double *out, double *y, uint32_t n);

#endif // _OVER_SAMPLE_H