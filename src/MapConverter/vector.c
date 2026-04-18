/*
	vector.c		Copyright (C) 1998-2000 Damir Sagidullin
*/
#include <memory.h>
#include <math.h>

#include "types.h"
#include "vector.h"

//------------------- DEFINITIONS & CONSTANTS -------------------//

//-------------------------- VARIABLES --------------------------//

vector	vec3_null = {0,0,0};

//-------------------------- FUNCTIONS --------------------------//

float	DotProduct (vector a, vector b)
{
	return	(a[0]*b[0]) + (a[1]*b[1]) + (a[2]*b[2]);
}

void	CrossProduct (vector a, vector b, vector cross)
{
	cross[0] = (a[1]*b[2]) - (a[2]*b[1]);
	cross[1] = (a[2]*b[0]) - (a[0]*b[2]);
	cross[2] = (a[0]*b[1]) - (a[1]*b[0]);
}

void	VectorCopy (vector a, vector b)
{
	memcpy(b, a, sizeof(vector));
}

void	VectorAdd (vector a, vector b, vector c)
{
	c[0] = a[0] + b[0];
	c[1] = a[1] + b[1];
	c[2] = a[2] + b[2];
}

void	VectorSubtract (vector a, vector b, vector c)
{
	c[0] = a[0] - b[0];
	c[1] = a[1] - b[1];
	c[2] = a[2] - b[2];
}

void	VectorScale (vector a, float scale, vector c)
{
	c[0] = a[0] * scale;
	c[1] = a[1] * scale;
	c[2] = a[2] * scale;
}

void	VectorAddScale (vector a, vector b, float scale, vector c)
{
	c[0] = a[0] + (b[0]*scale);
	c[1] = a[1] + (b[1]*scale);
	c[2] = a[2] + (b[2]*scale);
}

void	VectorInverse (vector v)
{
	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
}

void	VectorSet (vector vec, float x, float y, float z)
{
	vec[0] = x;
	vec[1] = y;
	vec[2] = z;
}

float	VectorLength (vector a)
{
	return	(float)(sqrt(DotProduct(a, a)));	//	FIXME
}

void	VectorNormalize (vector v)
{
	float	length = VectorLength (v);
	if (length)	VectorScale(v, 1.0f/length, v);
}
