#ifndef _MATH_3D_H
#define _MATH_3D_H

#include	"MapFile.h"
#include	"struct.h"

#define pi	3.14159265358979323846
#define	DIST_EPSILON	(0.03125f)	// 1/32 epsilon to keep floating point happy

#define VectorInvert(a,b) {b[0]=-a[0];b[1]=-a[1];b[2]=-a[2];}
#define VectorsMiddle(a, b, c) {c[0]=(a[0]+b[0])*0.5f; c[1]=(a[1]+b[1])*0.5f; c[2]=(a[2]+b[2])*0.5f;}

extern vec3_t	vec3_null;

//	Standart 3D vector math
float	DotProduct (vec3_t a, vec3_t b);
void	CrossProduct (vec3_t a, vec3_t b, vec3_t cross);
void	VectorCopy (vec3_t a, vec3_t b);
void	VectorScale (vec3_t a, float scale, vec3_t b);
void	VectorInverse (vec3_t v);
float	VectorLength (vec3_t a);
void	VectorNormalize (vec3_t v);
void	VectorAdd (vec3_t a, vec3_t b, vec3_t c);
void	VectorSubtract (vec3_t a, vec3_t b, vec3_t c);
void	VectorAddScale (vec3_t a, vec3_t b, float scale, vec3_t c);

//	3D math
float	PointPlaneDist (vec3_t pos, plane_t *plane);
float	det (float *x, float *y, float *z);
float	PointPointDist2 (vec3_t a, vec3_t b);
void	RotateVector (vec3_t vec, vec3_t ang);
void	RotateVector1 (vec3_t vec, vec3_t ang);
void	QuickRotateVector (vec3_t vec, float matrix[3][3]);
void	TransformVector (vec3_t vec, float matrix[3][3]);

float	BBoxRadius (vec3_t mins, vec3_t maxs, vec3_t pos);
void	FindBrushBBox (vec3_t mins, vec3_t maxs, int numplanes, plane_t *planes);

#endif