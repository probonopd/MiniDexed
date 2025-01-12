#include <cmath>
#include <iostream>
#include "Sinh.h"
#include "ASinh.h"
#include "Cosh.h"
#include "HyperbolicTables.h"


using namespace std;

float SINH( float x)
{
	int flag = 1;
	
	if (x < 0)
	{
		flag = -1;
		x = -x;
	}
	
	float SinH;
	
	if( x > SINH_fim)
	{
		SinH = (Sinh[SINH_N-1])*flag;
	}
	else
	{
		float naux = x*SINH_Idx;
		int n = round(naux);
		SinH = Sinh[n]*flag;
	}
	
	return SinH;
}

float COSH( float x)
{
	if (x < 0)
	{
		x = -x;
	}
	
	float CosH;
	
	if( x > COSH_fim)
	{
		CosH = Cosh[COSH_N-1];
	}
	else
	{
		float naux = x*COSH_Idx;
		int n = round(naux);
		CosH = Cosh[n];
	}
	
	return CosH;
}

float ASINH( float x)
{
	int flag = 1;
	
	if (x < 0)
	{
		flag = -1;
		x = -x;
	}
	
	float ASinH;
	
	if( x > ASINH_fim)
	{
		ASinH = (ASinh[ASINH_N-1])*flag;
	}
	else
	{
		float naux = x*ASINH_Idx;
		int n = round(naux);
		ASinH = ASinh[n]*flag;
	}
	
	return ASinH;
}