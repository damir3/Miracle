/*
	Math_3D.cpp		Copyright (C) 1998-1999 Damir Sagidullin
*/
#include	<Memory.h>
#include	<Math.h>

#include	"Console.h"
#include	"Const.h"
#include	"Math_3D.h"
#include	"MapFile.h"
#include	"Video.h"

vec3_t	vec3_null = {0,0,0};

float	det (float *x, float *y, float *z)
{
	float a1 = x[0]*((y[1]*z[2])-(y[2]*z[1]));
    float a2 = y[0]*((z[1]*x[2])-(z[2]*x[1]));
    float a3 = z[0]*((x[1]*y[2])-(x[2]*y[1]));
    return	a1+a2+a3;
}

float	PointPlaneDist (vec3_t pos, plane_t *plane)
{
	if (plane->type < 3)
		return	pos[plane->type] - plane->dist;
	else
		return	DotProduct (plane->normal, pos) - plane->dist;
}

float	PointPointDist2 (vec3_t a, vec3_t b)
{
	vec3_t	sub;
	VectorSubtract(a, b, sub);
	return	(sub[0]*sub[0]) + (sub[1]*sub[1]) + (sub[2]*sub[2]);
}

void	RotateVector1 (vec3_t vec, vec3_t ang)
{
	double	sin_, cos_, angle;
    double	x, y, z;
	// крен
	angle = ang[1] * pi / 0x8000;
    sin_ = sin(angle);
	cos_ = cos(angle);
	z =  cos_ * vec[2] + sin_ * vec[0];
	x = -sin_ * vec[2] + cos_ * vec[0];
	vec[0] = float(x);
	vec[2] = float(z);
	// тангаж
	angle = ang[0] * pi / 0x8000;
	sin_ = sin(angle);
	cos_ = cos(angle);
	y =  cos_ * vec[1] + sin_ * vec[2];
	z = -sin_ * vec[1] + cos_ * vec[2];
	vec[1] = float(y);
	vec[2] = float(z);
	// рыскание
	angle = ang[2] * pi / 0x8000;
    sin_ = sin(angle);
	cos_ = cos(angle);
	x =  cos_ * vec[0] + sin_ * vec[1];
	y = -sin_ * vec[0] + cos_ * vec[1];
	vec[0] = float(x);
	vec[1] = float(y);
}

void	RotateVector (vec3_t vec, vec3_t ang)
{
	double	sin_, cos_, angle;
    double	x, y, z;
	// рыскание
	angle = ang[2] * pi / 0x8000;
    sin_ = sin(angle);
	cos_ = cos(angle);
	x =  cos_ * vec[0] + sin_ * vec[1];
	y = -sin_ * vec[0] + cos_ * vec[1];
	vec[0] = float(x);
	vec[1] = float(y);
	// тангаж
	angle = ang[0] * pi / 0x8000;
	sin_ = sin(angle);
	cos_ = cos(angle);
	y =  cos_ * vec[1] + sin_ * vec[2];
	z = -sin_ * vec[1] + cos_ * vec[2];
	vec[1] = float(y);
	vec[2] = float(z);
	// крен
	angle = ang[1] * pi / 0x8000;
    sin_ = sin(angle);
	cos_ = cos(angle);
	z =  cos_ * vec[2] + sin_ * vec[0];
	x = -sin_ * vec[2] + cos_ * vec[0];
	vec[0] = float(x);
	vec[2] = float(z);
}

float	DotProduct (vec3_t a, vec3_t b)
{
	return	(a[0]*b[0]) + (a[1]*b[1]) + (a[2]*b[2]);
}

void	CrossProduct (vec3_t a, vec3_t b, vec3_t cross)
{
	cross[0] = (a[1] * b[2]) - (a[2] * b[1]);
	cross[1] = (a[2] * b[0]) - (a[0] * b[2]);
	cross[2] = (a[0] * b[1]) - (a[1] * b[0]);
}

void	VectorCopy (vec3_t a, vec3_t b)
{
	memcpy(b, a, sizeof(vec3_t));
}

void	VectorAdd (vec3_t a, vec3_t b, vec3_t c)
{
	c[0] = a[0] + b[0];
	c[1] = a[1] + b[1];
	c[2] = a[2] + b[2];
}

void	VectorSubtract (vec3_t a, vec3_t b, vec3_t c)
{
	c[0] = a[0] - b[0];
	c[1] = a[1] - b[1];
	c[2] = a[2] - b[2];
}

void	VectorAddScale (vec3_t a, vec3_t b, float scale, vec3_t c)
{
	c[0] = a[0] + (b[0]*scale);
	c[1] = a[1] + (b[1]*scale);
	c[2] = a[2] + (b[2]*scale);
}

void	VectorScale (vec3_t a, float scale, vec3_t b)
{
	b[0] = a[0]*scale;
	b[1] = a[1]*scale;
	b[2] = a[2]*scale;
}

void	VectorInverse (vec3_t v)
{
	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
}

float	VectorLength (vec3_t a)
{
	return	float(sqrt(DotProduct(a, a)));	//	FIXME
}

void	VectorNormalize (vec3_t v)
{
	float	length = VectorLength (v);
	if (length)	VectorScale(v, 1.0f/length, v);
}

void	QuickRotateVector (vec3_t vec, float matrix[3][3])
{
	vec3_t	tmp;
	VectorCopy(vec, tmp);
	vec[0] = tmp[0]*matrix[0][0] + tmp[1]*matrix[1][0] + tmp[2]*matrix[2][0];
	vec[1] = tmp[0]*matrix[0][1] + tmp[1]*matrix[1][1] + tmp[2]*matrix[2][1];
	vec[2] = tmp[0]*matrix[0][2] + tmp[1]*matrix[1][2] + tmp[2]*matrix[2][2];
}

void	TransformVector (vec3_t vec, float matrix[3][3])
{
	vec3_t	temp;
	VectorCopy(vec, temp);
	vec[0] = DotProduct(temp, matrix[0]);
	vec[1] = DotProduct(temp, matrix[1]);
	vec[2] = DotProduct(temp, matrix[2]);
}

float	BBoxRadius(vec3_t mins, vec3_t maxs, vec3_t pos)
{//	наждение радуса области (с центром в данной точке), которая
//	охватывает bounding box
	int		i, j, k;
	vec3_t	point;
	float	cur_radius, radius=0;

	for(i=0; i<2; i++)
	{
		if(i)	point[0] = mins[0];
		else	point[0] = maxs[0];
		for(j=0; j<2; j++)
		{
			if(j)	point[1] = mins[1];
			else	point[1] = maxs[1];
			for(k=0; k<2; k++)
			{
				if(k)	point[2] = mins[2];
				else	point[2] = maxs[2];
				cur_radius = PointPointDist2(point, pos);
				if(radius<cur_radius)	radius = cur_radius;
			}
		}
	}
	return	float(sqrt(radius));
}

vec3_t	vec3_maxs = {1000000, 1000000, 1000000};
vec3_t	vec3_mins = {-1000000, -1000000, -1000000};

void	FindBrushBBox(vec3_t mins, vec3_t maxs, int numplanes, plane_t *planes)
{
	int		i, j, n0, n1, n2;
	vec3_t	a, b, c, d, cross;
	float	temp;

	if(numplanes<4)
	{
		//CPrintf("FindBrushBBox(): Closed brush can't have only %d planes", numplanes);
		VectorCopy(vec3_mins, mins);
		VectorCopy(vec3_maxs, maxs);
		return;
	}

	VectorCopy(vec3_maxs, mins);
	VectorCopy(vec3_mins, maxs);

	for(n2=0; n2<numplanes-2; n2++)
	{
		a[2] = planes[n2].normal[0];
		b[2] = planes[n2].normal[1];
		c[2] = planes[n2].normal[2];
		d[2] = planes[n2].dist;
		for(n1=n2+1; n1<numplanes-1; n1++)
		{
			a[1] = planes[n1].normal[0];
			b[1] = planes[n1].normal[1];
			c[1] = planes[n1].normal[2];
			d[1] = planes[n1].dist;
			j = 0;
			for(n0=n1+1; n0<numplanes; n0++)
			{
				a[0] = planes[n0].normal[0];
				b[0] = planes[n0].normal[1];
				c[0] = planes[n0].normal[2];
				d[0] = planes[n0].dist;
				temp = det(a,b,c);
				if(temp == 0)	continue;
				temp = 1.0f / temp;
				cross[0] = det(d,b,c) * temp;
				cross[1] = det(a,d,c) * temp;
				cross[2] = det(a,b,d) * temp;
				for (i=0; i<numplanes; i++)
				{
					if((i==n0) || (i==n1) || (i==n2))	continue;
					if(PointPlaneDist(cross, &planes[i])>0)	break;
				}
				if (i<numplanes)	continue;
				j++;
				for (i=0; i<3; i++)
				{
					if(mins[i] > cross[i])	mins[i] = cross[i];
					if(maxs[i] < cross[i])	maxs[i] = cross[i];
				}
			}
			if(j<2)
			{
				//CPrintf("FindBrushBBox(): This brush is not closed");
				VectorCopy(vec3_mins, mins);
				VectorCopy(vec3_maxs, maxs);
				return;
			}
		}
	}
}
