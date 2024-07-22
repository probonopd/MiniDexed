#ifndef _DISTORTION_DS1_H
#define _DISTORTION_DS1_H

#include <cmath>

class ClipClass
{
public:
    ClipClass();
    void ChangeVolTone(float Volume, float Tone);
    void ChangeVol(float Volume);
    void ChangeSampleRate(float SampleRate);
    
    float R1;
	float R2;
	float R3;
	float R4;
	float Rt;
	float Rv;
	
	float C1;
	float C2;
	float C3;
	float C4;

	float Is;
	float Vt;
	
	float T;
	
	float t;
	float vol;
	
	float c1;
	float c2;
	float c3;
	float c4;
	
	float E[4][4];
	float F[4][4];
	float A[4][4];
	float A_[4][4];
	
	float Ka1[4];
	float Ka2[4];
	float Ka3[4];
	float Ka4[4];
	
	float K1[4];
	float K2[4];
	float K3[4];
	float K4[4];
	
	float DENa;
	float DEN;
	
	float Ku1;
	float Ku2;
	float Kv;
    
};

void Filter1(float *u, float *y, int N, float SampleRate, float *U_1, float *Y_1 );
void Filter2(float *u, float *y, int N, float SampleRate, float *U_1, float *Y_1, float *U_2, float *Y_2, float *U_3, float *Y_3, float *U_4, float *Y_4 );
void FilterGain(float *u, float *y, int N, float Dist, float SampleRate, float *U_1, float *Y_1, float *U_2, float *Y_2 );
void DS1_Clip_Tone(float *u, float *y, float *v1, float *v2, float *v3,  int N, float T, float *U_1, float *Y_1, float *V1_1, float *V2_1, float *V3_1, float t, float vol, ClipClass *obj);
void Filter1_48000(float *u, float *y, int N, float *U_1, float *Y_1 );
void Filter2_48000(float *u, float *y, int N, float *U_1, float *Y_1, float *U_2, float *Y_2, float *U_3, float *Y_3, float *U_4, float *Y_4 );
void FilterGain_48000(float *u, float *y, int N, float Dist, float *U_1, float *Y_1, float *U_2, float *Y_2 );
void DS1_Clip_Tone_48000(float *u, float *y, float *v1, float *v2, float *v3,  int N, float *U_1, float *Y_1, float *V1_1, float *V2_1, float *V3_1, float t, float vol);

#endif // _DISTORTION_DS1_H