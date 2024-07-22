#include <cmath>
#include "Distortion_BigMuff.h"
#include "HyperbolicTables.h"

void BM_Filter1(float *u, float *y, int N, double T, float *U_1, float *Y_1 )
{
	const float R1 = 56e3;
	const float R4 = 330e3;
	//const float R5 = 10e3;
	//const float R6 = 47e3;
	//const float R7 = 62e3;
	//const float R8 = 560e3;
	//const float R9 = 47;
	//const float R10 = 8.2e3;
	//const float R13 = 470e3;
	//const float R14 = 5.6e3;
	//const float R15 = 1.2e3;
	//const float R16 = 100e3;
	
	const float C1 = 150e-9;
	//const float C3 = 4.7e-9;
	//const float C4 = 10e-9;
	//const float C5 = 10e-6;
	//const float C6 = 10e-6;
	//const float C7 = 4.7e-6;
	//const float C9 = 150e-12;
	//const float C10 = 100e-9;
	//const float C11 = 1e-6;
	//const float C12 = 120e-9;
	
	//const float P1 = 10e3;
	//const float P2 = 10e3;
	//const float P3 = 100e3;
	
	float c = 2/T;
	
	float y_1 = Y_1[0];
	float u_1 = U_1[0];
	
	/*
	         b1s + b0
	 G(s) = ----------
	         a1s + a0

	         B0 + B1z⁻¹
	 G(z) =  ----------
             A0 + A1z⁻¹
     
     y[k] = (-A1*y[k-1] + B0*u[k] + B1*u[k-1] )/A0
	*/
	
	const float b0 = 0;
	const float b1 = -C1*R4;
	const float a0 = 1;
	const float a1 = C1*R1;
	
	float B0 = b0 + b1*c;
	float B1 = b0 - b1*c;
	float A0 = a0 + a1*c;
	float A1 = a0 - a1*c;	
	
	y[0] = (-A1*y_1 + B0*u[0] + B1*u_1)/A0;
		
	for (int i=1; i<=N-1; i++)
	{
		y[i] = (-A1*y[i-1] + B0*u[i] + B1*u[i-1] )/A0;
	}
	
	U_1[0] = u[N-1];
	Y_1[0] = y[N-1];
	
}

void BM_Filter2(float *u, float *y, int N, double T, float *U_1, float *Y_1, float *U_2, float *Y_2, float*U_3, float *Y_3 )
{
	//const double R1 = 56e3;
	//const double R4 = 330e3;
	const double R5 = 10e3;
	const double R6 = 47e3;
	const double R7 = 62e3;
	const double R8 = 560e3;
	//const double R9 = 47;
	//const double R10 = 8.2e3;
	//const double R13 = 470e3;
	//const double R14 = 5.6e3;
	//const double R15 = 1.2e3;
	//const double R16 = 100e3;
	
	//const double C1 = 150e-9;
	const double C3 = 4.7e-9;
	const double C4 = 10e-9;
	const double C5 = 10e-6;
	//const double C6 = 10e-6;
	//const double C7 = 4.7e-6;
	//const double C9 = 150e-12;
	//const double C10 = 100e-9;
	//const double C11 = 1e-6;
	//const double C12 = 120e-9;
	
	//const double P1 = 10e3;
	//const double P2 = 10e3;
	//const double P3 = 100e3;
	
	double c = 2/T;
	
	double y_1 = Y_1[0];
	double u_1 = U_1[0];
	double y_2 = Y_2[0];
	double u_2 = U_2[0];
	double y_3 = Y_3[0];
	double u_3 = U_3[0];
	
	
	
	
	/*
	         b3s³ + b2s² + b1s + b0
	 G(s) = ------------------------
	         a3s³ + a2s² + a1s + a0

	         B0 + B1z⁻¹ + B2z⁻² + B3z⁻3    Y(z)
	 G(z) =  -------------------------- =  ----
             A0 + A1z⁻¹ + A2z⁻² + A3z⁻3    U(z)
     
     y[k] = (-A1*y[k-1] -A2*y[k-2] - A3*y[k-3] + B0*u[k] + B1*u[k-1] + B2*u[k-2] + B3*u[k-3] )/A0
	*/
	
	const double b0 = 1;
	const double b1 = C5*(R7+R8);
	const double b2 = 0;
	const double b3 = 0;
	const double a0 = 1;
	const double a1 = C4*(R5 + R6) + C5*R7;
	const double a2 = C4*C5*(R5 + R6)*R7 + C3*R5*(C4*R6 - C5*R8);
	const double a3 = C3*C4*C5*R5*R6*R7;
	
	double c2 = pow(c,2);
	double c3 = pow(c,3);
	
	double B0 = -b0 - b1*c - b2*c2 - b3*c3;
	double B1 = -3*b0 - b1*c + b2*c2 + 3*b3*c3;
	double B2 = -3*b0 + b1*c + b2*c2 - 3*b3*c3;
	double B3 = -b0 + b1*c - b2*c2 + b3*c3;
	double A0 = -a0 - a1*c - a2*c2 - a3*c3;
	double A1 = -3*a0 - a1*c + a2*c2 + 3*a3*c3;
	double A2 = -3*a0 + a1*c + a2*c2 - 3*a3*c3;
	double A3 = -a0 + a1*c - a2*c2 + a3*c3;
	
	y[0] = (-A1*y_1 -A2*y_2 -A3*y_3 + B0*u[0] + B1*u_1 + B2*u_2 + B3*u_3)/A0;
	y[1] = (-A1*y[0] -A2*y_1 -A3*y_2 + B0*u[1] + B1*u[0] + B2*u_1 + B3*u_2)/A0;
	y[2] = (-A1*y[1] -A2*y[0] -A3*y_1 + B0*u[2] + B1*u[1] + B2*u[0] + B3*u_1)/A0;
		
	for (int i=3; i<=N-1; i++)
	{
		y[i] = (-A1*y[i-1] -A2*y[i-2] -A3*y[i-3] + B0*u[i] + B1*u[i-1] + B2*u[i-2] + B3*u[i-3])/A0;
	}
	
	U_1[0] = u[N-1];
	Y_1[0] = y[N-1];
	U_2[0] = u[N-2];
	Y_2[0] = y[N-2];
	U_3[0] = u[N-3];
	Y_3[0] = y[N-3];
	
}

void BM_Filter3(float *u, float *y, int N, double T, float *U_1, float *Y_1, float *U_2, float *Y_2, double x, double x_1 )
{
	//const double R1 = 56e3;
	//const double R4 = 330e3;
	//const double R5 = 10e3;
	//const double R6 = 47e3;
	//const double R7 = 62e3;
	//const double R8 = 560e3;
	const double R9 = 47;
	const double R10 = 8.2e3;
	//const double R13 = 470e3;
	//const double R14 = 5.6e3;
	//const double R15 = 1.2e3;
	//const double R16 = 100e3;
	
	//const double C1 = 150e-9;
	//const double C3 = 4.7e-9;
	//const double C4 = 10e-9;
	//const double C5 = 10e-6;
	const double C6 = 10e-6;
	const double C7 = 4.7e-6;
	//const double C9 = 150e-12;
	//const double C10 = 100e-9;
	//const double C11 = 1e-6;
	//const double C12 = 120e-9;
	
	const double P1 = 10e3;
	//const double P2 = 10e3;
	//const double P3 = 100e3;
	
	double c = 2/T;
	
	double y_1 = Y_1[0];
	double u_1 = U_1[0];
	double y_2 = Y_2[0];
	double u_2 = U_2[0];
	
	/*
	         b2s² + b1s + b0
	 G(s) = ----------------
	         a2s² + a1s + a0

	         B0 + B1z⁻¹ + B2z⁻²    Y(z)
	 G(z) =  ------------------- = ----
             A0 + A1z⁻¹ + A2z⁻²    U(z)
     
     y[k] = (-A1*y[k-1] -A2*y[k-2] + B0*u[k] + B1*u[k-1] + B2*u[k-2] )/A0
	*/
	
	double X = ( (N-1-0)*x_1 + (0)*x )/(N-1);
	
	const double b0 = 0;
	const double b1 = 0;
	double b2 = -C6*C7*R10*(R9 + P1*X);
	const double a0 = -1;
	double a1 = -C6*(P1 + R9) - C7*(R10 + R9 + P1*X);
	double a2 = C6*C7*(-R10*R9 + P1*P1*(-1 + X)*X - P1*(R10 + R9 - R9*X));
	
	double c2 = pow(c,2);
	
	double B0 = b0 + b1*c + b2*c2;
	double B1 = 2*b0 - 2*b2*c2;
	double B2 = b0 - b1*c + b2*c2;
	double A0 = a0 + a1*c + a2*c2;
	double A1 = 2*a0 - 2*a2*c2;
	double A2 = a0 - a1*c + a2*c2;
	
	y[0] = (-A1*y_1 -A2*y_2 + B0*u[0] + B1*u_1 + B2*u_2)/A0;
	
	X = ( (N-1-1)*x_1 + (1)*x )/(N-1);
	
	b2 = -C6*C7*R10*(R9 + P1*X);
	a1 = -C6*(P1 + R9) - C7*(R10 + R9 + P1*X);
	a2 = C6*C7*(-R10*R9 + P1*P1*(-1 + X)*X - P1*(R10 + R9 - R9*X));
	
	B0 = b0 + b1*c + b2*c2;
	B1 = 2*b0 - 2*b2*c2;
	B2 = b0 - b1*c + b2*c2;
	A0 = a0 + a1*c + a2*c2;
	A1 = 2*a0 - 2*a2*c2;
	A2 = a0 - a1*c + a2*c2;
	
	y[1] = (-A1*y[0] -A2*y_1 + B0*u[1] + B1*u[0] + B2*u_1)/A0;
		
	for (int i=2; i<=N-1; i++)
	{
		X = ( (N-1-i)*x_1 + (i)*x )/(N-1);
	
		b2 = -C6*C7*R10*(R9 + P1*X);
		a1 = -C6*(P1 + R9) - C7*(R10 + R9 + P1*X);
		a2 = C6*C7*(-R10*R9 + P1*P1*(-1 + X)*X - P1*(R10 + R9 - R9*X));
	
		B0 = b0 + b1*c + b2*c2;
		B1 = 2*b0 - 2*b2*c2;
		B2 = b0 - b1*c + b2*c2;
		A0 = a0 + a1*c + a2*c2;
		A1 = 2*a0 - 2*a2*c2;
		A2 = a0 - a1*c + a2*c2;
		
		y[i] = (-A1*y[i-1] -A2*y[i-2] + B0*u[i] + B1*u[i-1] + B2*u[i-2])/A0;
	}
	
	U_1[0] = u[N-1];
	Y_1[0] = y[N-1];
	U_2[0] = u[N-2];
	Y_2[0] = y[N-2];
	
}

void BM_Clip(float *u, float *y, int N, double T, float *U_1, float *Y_1 )
{
	//const double R1 = 56e3;
	//const double R4 = 330e3;
	//const double R5 = 10e3;
	//const double R6 = 47e3;
	//const double R7 = 62e3;
	//const double R8 = 560e3;
	//const double R9 = 47;
	const double R10 = 8.2e3;
	const double R13 = 470e3;
	//const double R14 = 5.6e3;
	//const double R15 = 1.2e3;
	//const double R16 = 100e3;
	
	//const double C1 = 150e-9;
	//const double C3 = 4.7e-9;
	//const double C4 = 10e-9;
	//const double C5 = 10e-6;
	//const double C6 = 10e-6;
	//const double C7 = 4.7e-6;
	const double C9 = 150e-12;
	//const double C10 = 100e-9;
	//const double C11 = 1e-6;
	//const double C12 = 120e-9;
	
	//const double P1 = 10e3;
	//const double P2 = 10e3;
	//const double P3 = 100e3;
	
	const double Is = 2.52*pow(10,-9);
	const double Vt = 3*45.3/1000;
	
	//double c = 2/T;
	
	double y_1 = Y_1[0];
	double u_1 = U_1[0];
	
	double A;
	double B;
	double D;
	
	/*

	*/
	
	A = y_1;
	B = T*Vt*(R13*(4*Is*R10*SINH(y_1/Vt)+u_1+u[0])+2*R10*y_1);
	D = R10*((2*C9*R13+T)*Vt+2*Is*R13*T*COSH(y_1/Vt));

	y[0] = A - B/D;
		
	for (int k=1; k<=N-1; k++)
	{
		A = y[k-1];
		B = T*Vt*(R13*(4*Is*R10*SINH(y[k-1]/Vt)+u[k-1]+u[k])+2*R10*y[k-1]);
		D = R10*((2*C9*R13+T)*Vt+2*Is*R13*T*COSH(y[k-1]/Vt));

		y[k] = A - B/D;
	}
	
	U_1[0] = u[N-1];
	Y_1[0] = y[N-1];
	
}

void BM_Filter4(float *u, float *y, int N, double T, float *U_1, float *Y_1, float *U_2, float *Y_2, float *U_3, float *Y_3, double tone, double vol )
{
	//const double R1 = 56e3;
	//const double R4 = 330e3;
	//const double R5 = 10e3;
	//const double R6 = 47e3;
	//const double R7 = 62e3;
	//const double R8 = 560e3;
	//const double R9 = 47;
	//const double R10 = 8.2e3;
	//const double R13 = 470e3;
	const double R14 = 5.6e3;
	const double R15 = 1.2e3;
	const double R16 = 100e3;
	
	//const double C1 = 150e-9;
	//const double C3 = 4.7e-9;
	//const double C4 = 10e-9;
	//const double C5 = 10e-6;
	//const double C6 = 10e-6;
	//const double C7 = 4.7e-6;
	//const double C9 = 150e-12;
	const double C10 = 100e-9;
	const double C11 = 1e-6;
	const double C12 = 120e-9;
	
	//const double P1 = 10e3;
	const double P2 = 10e3;
	const double P3 = 100e3;
	
	double c = 2/T;
	
	double y_1 = Y_1[0];
	double u_1 = U_1[0];
	double y_2 = Y_2[0];
	double u_2 = U_2[0];
	double y_3 = Y_3[0];
	double u_3 = U_3[0];
	
	/*
	         b3s³ + b2s² + b1s + b0
	 G(s) = ------------------------
	         a3s³ + a2s² + a1s + a0

	         B0 + B1z⁻¹ + B2z⁻² + B3z⁻3    Y(z)
	 G(z) =  -------------------------- =  ----
             A0 + A1z⁻¹ + A2z⁻² + A3z⁻3    U(z)
     
     y[k] = (-A1*y[k-1] -A2*y[k-2] - A3*y[k-3] + B0*u[k] + B1*u[k-1] + B2*u[k-2] + B3*u[k-3] )/A0
	*/
	
	const double b0 = 0;
	const double b1 = -P3*R16*(C10*R15+C11*(P2+R15-P2*tone))*vol;
	const double b2 = -C10*P3*R15*R16*(C11*(P2+R14)+C12*P2*tone)*vol;
	const double b3 = -C10*C11*C12*P2*P3*R14*R15*R16*tone*vol;
	
	const double a0 = -(P3+R15)*R16-P3*(P3+R15)*vol+P3*P3*vol*vol+P2*(-1+tone)*(R16+P3*vol);
	const double a1 = -(C10*(P2+P3)*R15+C12*P3*(P2+R15)+C11*(P3*R14+P2*(P3+R14)+(P3+R14)*R15))*R16-P2*(-C10*R15+C12*(P2+R15)+C11*(P2-R14+R15))*R16*tone+(C11+C12)*P2*P2*R16*tone*tone-P3*(C10*R15*(P2+P3-P2*tone)+C12*(P3*(P2+R15)+P2*(P2+R15)*tone-P2*P2*tone*tone)+C11*(P3*R14+(P3+R14)*R15-P2*P2*(-1+tone)*tone+P2*(P3+R14-R14*tone+R15*tone)))*vol+P3*P3*(C10*R15+C12*(P2+R15)+C11*(P2+R14+R15))*vol*vol;
	const double a2 = C10*C12*P2*R15*(-P3*R16+P2*R16*(-1+tone)*tone+P2*P3*(-1+tone)*tone*vol+P3*P3*(-1+vol)*vol)+C11*(C10*R15*(P2*P2*(-1+tone)*tone*(R16+P3*vol)+P3*R14*(-R16+P3*(-1+vol)*vol)+P2*(-P3*R16+R14*R16*(-1+tone)+P3*R14*(-1+tone)*vol+P3*P3*(-1+vol)*vol))+C12*R14*(P2*P2*(-1+tone)*tone*(R16+P3*vol)+P3*R15*(-R16+P3*(-1+vol)*vol)+P2*(-R15*R16*tone+P3*P3*(-1+vol)*vol-P3*(R16+R15*tone*vol))));
	const double a3 = C10*C11*C12*P2*R14*R15*(-P3*R16+P2*R16*(-1+tone)*tone+P2*P3*(-1+tone)*tone*vol+P3*P3*(-1+vol)*vol);
	
	double c2 = pow(c,2);
	double c3 = pow(c,3);
	
	double B0 = -b0 - b1*c - b2*c2 - b3*c3;
	double B1 = -3*b0 - b1*c + b2*c2 + 3*b3*c3;
	double B2 = -3*b0 + b1*c + b2*c2 - 3*b3*c3;
	double B3 = -b0 + b1*c - b2*c2 + b3*c3;
	double A0 = -a0 - a1*c - a2*c2 - a3*c3;
	double A1 = -3*a0 - a1*c + a2*c2 + 3*a3*c3;
	double A2 = -3*a0 + a1*c + a2*c2 - 3*a3*c3;
	double A3 = -a0 + a1*c - a2*c2 + a3*c3;
	
	y[0] = (-A1*y_1 -A2*y_2 -A3*y_3 + B0*u[0] + B1*u_1 + B2*u_2 + B3*u_3)/A0;
	y[1] = (-A1*y[0] -A2*y_1 -A3*y_2 + B0*u[1] + B1*u[0] + B2*u_1 + B3*u_2)/A0;
	y[2] = (-A1*y[1] -A2*y[0] -A3*y_1 + B0*u[2] + B1*u[1] + B2*u[0] + B3*u_1)/A0;
		
	for (int i=3; i<=N-1; i++)
	{
		y[i] = (-A1*y[i-1] -A2*y[i-2] -A3*y[i-3] + B0*u[i] + B1*u[i-1] + B2*u[i-2] + B3*u[i-3])/A0;
	}
	
	U_1[0] = u[N-1];
	Y_1[0] = y[N-1];
	U_2[0] = u[N-2];
	Y_2[0] = y[N-2];
	U_3[0] = u[N-3];
	Y_3[0] = y[N-3];
	
}