#ifndef _MATH_3D_H
#define _MATH_3D_H

#include	"MapFile.h"

#define pi	3.14159265358979323846
#define	DIST_EPSILON	(0.03125f)	// 1/32 epsilon to keep floating point happy

typedef struct
{
	vec3_t	p;
	int		sx,sy;
	float	div_z, u, v, r, g, b;
	float	dist;
	byte	ccodes;
} point_3d;

//	Standart 3D vector math
float	DotProduct (vec3_t a, vec3_t b);
void	CrossProduct (vec3_t a, vec3_t b, vec3_t cross);
void	VectorCopy (vec3_t a, vec3_t b);
void	VectorAdd (vec3_t a, vec3_t b, vec3_t c);
void	VectorSubtract (vec3_t a, vec3_t b, vec3_t c);
void	VectorScale (vec3_t a, float scale, vec3_t c);
void	VectorAddScale (vec3_t a, vec3_t b, float scale, vec3_t c);
void	VectorInverse (vec3_t v);
//	3D math
float	PointPlaneDist(vec3_t pos, t_plane *plane);
float	DistToCamera(t_plane *plane);
void	RotateVector(vec3_t vec, vec3_t ang);
void	QuickRotateVector(vec3_t vec, float matrix[3][3]);
void	QuickTransformVector(vec3_t vec, float matrix[3][3]);
float	PointPointDist2(vec3_t a, vec3_t b);
void	ComputeViewMatrix();
void	SetViewInfo(vec3_t loc, vec3_t ang);
void	TransformVector(vec3_t out, vec3_t in);
float	Dist2FromCamera(vec3_t loc);
float	det(float *x, float *y, float *z);
void	ComputeViewFrustrum();
int		BoxInFrustrum(float *mins, float *maxs);
void	ProjectPointToScreen(point_3d *p);
void	CodePoint(point_3d *p);
void	TransformPoint(point_3d *p, vec3_t v);
void	TransformPointRaw(vec3_t out, vec3_t in);
void	CrossPoint(point_3d *out, point_3d *a, point_3d *b);
void	CalculateCameraParameters();
void	SaveViewInfo();
void	RestoreViewInfo();
void	RestorePreviousViewInfo(int sub_depth);
int		ClipScreenX(int a);
int		ClipScreenY(int a);

extern rect_t	clip_rect;
extern float	mdl_scale[3];
extern float	bitmap_scale[2];
extern float	main_matrix[3][3];
extern float	view_matrix[3][3];
extern vec3_t	cam_pos, vec3_null;
extern double	right_clip, left_clip, top_clip, bottom_clip;
extern float	xcenter, ycenter;
extern double	mip_scale, where;
extern float	view_size, view_angle;
extern t_plane	*frustrum_planes;
extern char		clip_mode;

#define CC_OFF_LEFT		1
#define CC_OFF_RIGHT	2
#define CC_OFF_TOP		4
#define CC_OFF_BOT		8
#define CC_BEHIND		16
#define CC_OUTSIDE		32

#endif