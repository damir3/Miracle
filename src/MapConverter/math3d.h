#ifndef MATH3D_H
#define MATH3D_H

#include "map.h"
#include "vector.h"

#ifdef __cplusplus
extern "C" {
#endif

#define pi	3.14159265358979323846
#define	DIST_EPSILON	(0.03125f)	// 1/32 epsilon to keep floating point happy

//	3D math
float	PointPlaneDist (vector pos, plane_t *plane);
float	det (float *x, float *y, float *z);
float	PointPointDist2 (vector a, vector b);
void	RotateVector (vector vec, vector ang);
void	RotateVector1 (vector vec, vector ang);
void	QuickRotateVector (vector vec, float matrix[3][3]);
void	TransformVector (vector vec, float matrix[3][3]);

float	BBoxRadius (vector mins, vector maxs, vector pos);
int		FindSector (int num, vector p);

void	MeshGetPoint (float u, float v, face_t *face, vector out);
void	MeshGetNormal (float u, float v, face_t *face, vector out);
float	PointBBoxfDist2 (vector pos, bboxf_t *bbox);

#ifdef __cplusplus
}
#endif

#endif