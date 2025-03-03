#include <math.h>
#include "OverSample.h"

void Over1_Float(float *in, float *u, float *u_1, uint32_t n)
{
	for (uint32_t i = 1; i <= n; i++)
		{
			u[i-1] = in[i-1];
		}
}
void Over2_Float(float *in, float *u, float *u_1, uint32_t n)
{
		
		for (uint32_t i = 1; i <= n; i++)
		{
			u[2*i - 1] = in[i-1];
		}
		
		u[0] = 0.5*(u_1[0]+in[0]);
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[2*i - 2] = 0.5*(u[2*i - 3]+u[2*i - 1]);
		}
}

void Over4_Float(float *in, float *u, float *u_1, uint32_t n)
{
		
		for (uint32_t i = 1; i <= n; i++)
		{
			u[4*i - 1] = in[i-1];
		}
		
		u[1] = 0.5*(u_1[0]+in[0]);
		u[0] = 0.5*(u_1[0]+u[1]);
		u[2] = 0.5*(u[1]+in[0]);
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[4*i - 3] = 0.5*(u[4*i - 1]+u[4*i - 5]);
		}
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[4*i - 4] = 0.5*(u[4*i - 5]+u[4*i - 3]);
		}
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[4*i - 2] = 0.5*(u[4*i - 3]+u[4*i - 1]);
		}
}

void Over8_Float(float *in, float *u, float *u_1, uint32_t n)
{
		
		for (uint32_t i = 1; i <= n; i++)
		{
			u[8*i - 1] = in[i-1];
		}
		
		u[3] = 0.5*(u_1[0]+u[7]);
		u[1] = 0.5*(u_1[0]+u[3]);
		u[5] = 0.5*(u[3]+u[7]);
		
		u[2] = 0.5*(u[1]+u[3]);
		u[4] = 0.5*(u[3]+u[5]);
		
		u[0] = 0.5*(u[1]+u_1[0]);
		u[6] = 0.5*(u[5]+u[7]);
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[8*i - 5] = 0.5*(u[8*i - 1]+u[8*i - 9]);
		}
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[8*i - 7] = 0.5*(u[8*i - 5]+u[8*i - 9]);
		}
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[8*i - 3] = 0.5*(u[8*i - 1]+u[8*i - 5]);
		}
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[8*i - 8] = 0.5*(u[8*i - 7]+u[8*i - 9]);
		}
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[8*i - 6] = 0.5*(u[8*i - 5]+u[8*i - 7]);
		}
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[8*i - 4] = 0.5*(u[8*i - 3]+u[8*i - 5]);
		}
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[8*i - 2] = 0.5*(u[8*i - 3]+u[8*i - 1]);
		}
}

void Over1(float *in, double *u, double *u_1, uint32_t n)
{
	for (uint32_t i = 1; i <= n; i++)
		{
			u[i-1] = in[i-1];
		}
}
void Over2(float *in, double *u, double *u_1, uint32_t n)
{
		
		for (uint32_t i = 1; i <= n; i++)
		{
			u[2*i - 1] = in[i-1];
		}
		
		u[0] = 0.5*(u_1[0]+in[0]);
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[2*i - 2] = 0.5*(u[2*i - 3]+u[2*i - 1]);
		}
}

void Over4(float *in, double *u, double *u_1, uint32_t n)
{
		
		for (uint32_t i = 1; i <= n; i++)
		{
			u[4*i - 1] = in[i-1];
		}
		
		u[1] = 0.5*(u_1[0]+in[0]);
		u[0] = 0.5*(u_1[0]+u[1]);
		u[2] = 0.5*(u[1]+in[0]);
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[4*i - 3] = 0.5*(u[4*i - 1]+u[4*i - 5]);
		}
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[4*i - 4] = 0.5*(u[4*i - 5]+u[4*i - 3]);
		}
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[4*i - 2] = 0.5*(u[4*i - 3]+u[4*i - 1]);
		}
}

void Over8(float *in, double *u, double *u_1, uint32_t n)
{
		
		for (uint32_t i = 1; i <= n; i++)
		{
			u[8*i - 1] = in[i-1];
		}
		
		u[3] = 0.5*(u_1[0]+u[7]);
		u[1] = 0.5*(u_1[0]+u[3]);
		u[5] = 0.5*(u[3]+u[7]);
		
		u[2] = 0.5*(u[1]+u[3]);
		u[4] = 0.5*(u[3]+u[5]);
		
		u[0] = 0.5*(u[1]+u_1[0]);
		u[6] = 0.5*(u[5]+u[7]);
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[8*i - 5] = 0.5*(u[8*i - 1]+u[8*i - 9]);
		}
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[8*i - 7] = 0.5*(u[8*i - 5]+u[8*i - 9]);
		}
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[8*i - 3] = 0.5*(u[8*i - 1]+u[8*i - 5]);
		}
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[8*i - 8] = 0.5*(u[8*i - 7]+u[8*i - 9]);
		}
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[8*i - 6] = 0.5*(u[8*i - 5]+u[8*i - 7]);
		}
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[8*i - 4] = 0.5*(u[8*i - 3]+u[8*i - 5]);
		}
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[8*i - 2] = 0.5*(u[8*i - 3]+u[8*i - 1]);
		}
}

void Over1_Double(double *in, double *u, double *u_1, uint32_t n)
{
	for (uint32_t i = 1; i <= n; i++)
		{
			u[i-1] = in[i-1];
		}
}
void Over2_Double(double *in, double *u, double *u_1, uint32_t n)
{
		
		for (uint32_t i = 1; i <= n; i++)
		{
			u[2*i - 1] = in[i-1];
		}
		
		u[0] = 0.5*(u_1[0]+in[0]);
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[2*i - 2] = 0.5*(u[2*i - 3]+u[2*i - 1]);
		}
}

void Over4_Double(double *in, double *u, double *u_1, uint32_t n)
{
		
		for (uint32_t i = 1; i <= n; i++)
		{
			u[4*i - 1] = in[i-1];
		}
		
		u[1] = 0.5*(u_1[0]+in[0]);
		u[0] = 0.5*(u_1[0]+u[1]);
		u[2] = 0.5*(u[1]+in[0]);
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[4*i - 3] = 0.5*(u[4*i - 1]+u[4*i - 5]);
		}
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[4*i - 4] = 0.5*(u[4*i - 5]+u[4*i - 3]);
		}
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[4*i - 2] = 0.5*(u[4*i - 3]+u[4*i - 1]);
		}
}

void Over8_Double(double *in, double *u, double *u_1, uint32_t n)
{
		
		for (uint32_t i = 1; i <= n; i++)
		{
			u[8*i - 1] = in[i-1];
		}
		
		u[3] = 0.5*(u_1[0]+u[7]);
		u[1] = 0.5*(u_1[0]+u[3]);
		u[5] = 0.5*(u[3]+u[7]);
		
		u[2] = 0.5*(u[1]+u[3]);
		u[4] = 0.5*(u[3]+u[5]);
		
		u[0] = 0.5*(u[1]+u_1[0]);
		u[6] = 0.5*(u[5]+u[7]);
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[8*i - 5] = 0.5*(u[8*i - 1]+u[8*i - 9]);
		}
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[8*i - 7] = 0.5*(u[8*i - 5]+u[8*i - 9]);
		}
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[8*i - 3] = 0.5*(u[8*i - 1]+u[8*i - 5]);
		}
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[8*i - 8] = 0.5*(u[8*i - 7]+u[8*i - 9]);
		}
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[8*i - 6] = 0.5*(u[8*i - 5]+u[8*i - 7]);
		}
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[8*i - 4] = 0.5*(u[8*i - 3]+u[8*i - 5]);
		}
		
		for (uint32_t i = 2; i <= n; i++)
		{
			u[8*i - 2] = 0.5*(u[8*i - 3]+u[8*i - 1]);
		}
}

void Down1_Float(float *out, float *y, uint32_t n)
{
	for (uint32_t i = 1; i <= n; i++)
		{
			out[i-1] = y[i-1];
		}
}

void Down2_Float(float *out, float *y, uint32_t n)
{
	for (uint32_t i = 1; i <= n; i++)
		{
			out[i-1] = y[2*i-1];
		}
}

void Down4_Float(float *out, float *y, uint32_t n)
{
	for (uint32_t i = 1; i <= n; i++)
		{
			out[i-1] = y[4*i-1];
		}
}

void Down8_Float(float *out, float *y, uint32_t n)
{
	for (uint32_t i = 1; i <= n; i++)
		{
			out[i-1] = y[8*i-1];
		}
}


void Down1(float *out, double *y, uint32_t n)
{
	for (uint32_t i = 1; i <= n; i++)
		{
			out[i-1] = y[i-1];
		}
}

void Down2(float *out, double *y, uint32_t n)
{
	for (uint32_t i = 1; i <= n; i++)
		{
			out[i-1] = y[2*i-1];
		}
}

void Down4(float *out, double *y, uint32_t n)
{
	for (uint32_t i = 1; i <= n; i++)
		{
			out[i-1] = y[4*i-1];
		}
}

void Down8(float *out, double *y, uint32_t n)
{
	for (uint32_t i = 1; i <= n; i++)
		{
			out[i-1] = y[8*i-1];
		}
}

void Down1_Double(double *out, double *y, uint32_t n)
{
	for (uint32_t i = 1; i <= n; i++)
		{
			out[i-1] = y[i-1];
		}
}

void Down2_Double(double *out, double *y, uint32_t n)
{
	for (uint32_t i = 1; i <= n; i++)
		{
			out[i-1] = y[2*i-1];
		}
}

void Down4_Double(double *out, double *y, uint32_t n)
{
	for (uint32_t i = 1; i <= n; i++)
		{
			out[i-1] = y[4*i-1];
		}
}

void Down8_Double(double *out, double *y, uint32_t n)
{
	for (uint32_t i = 1; i <= n; i++)
		{
			out[i-1] = y[8*i-1];
		}
}