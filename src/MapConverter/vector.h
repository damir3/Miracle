#ifndef _VECTOR_H
#define _VECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

extern vector   vec3_null;

//	Standart 3D vector math

#define VectorsMiddle(a, b, c) {c[0]=(a[0]+b[0])*0.5f; c[1]=(a[1]+b[1])*0.5f; c[2]=(a[2]+b[2])*0.5f;}

float	DotProduct (vector a, vector b);
void	CrossProduct (vector a, vector b, vector cross);
void	VectorCopy (vector a, vector b);
void	SetVector(vector vec, float x, float y, float z);
void	VectorAdd (vector a, vector b, vector c);
void	VectorSubtract (vector a, vector b, vector c);
void	VectorScale (vector a, float scale, vector c);
void	VectorAddScale (vector a, vector b, float scale, vector c);
void	VectorInverse (vector v);
float	VectorLength (vector a);
void	VectorSet (vector vec, float x, float y, float z);
void	VectorNormalize (vector v);

#ifdef __cplusplus
}
#endif

#endif

