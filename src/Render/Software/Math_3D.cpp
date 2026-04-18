/*
	Math_3D.cpp		Copyright (C) 1998-1999 Damir Sagidullin
*/
#include	<Memory.h>
#include	<Math.h>

#include	"Const.h"
#include	"Math_3D.h"
#include	"MapFile.h"
#include	"Video.h"

#define	MAX_PORTAL_DEPTH	8

static int		depth=-1;
static float	temp_main_matrix[MAX_PORTAL_DEPTH][3][3];
static float	temp_view_matrix[MAX_PORTAL_DEPTH][3][3];
static t_plane	temp_frustrum_planes[MAX_PORTAL_DEPTH+1][4];
static vec3_t	temp_cam_pos[MAX_PORTAL_DEPTH];
//	матрицы преобразования координат
float	main_matrix[3][3];
float	view_matrix[3][3];
vec3_t	cam_pos;
t_plane	*frustrum_planes = temp_frustrum_planes[0];
vec3_t	vec3_null = {0,0,0};

static float	near_clip = float(0.01), near_code = float(16.0);

double	chop_temp, where;
double	proj_scale_x, proj_scale_y, mip_scale;
float	xcenter, ycenter;
double	right_clip, left_clip, top_clip, bottom_clip;
int		sx_min, sx_max, sy_min, sy_max;
char	clip_mode;

rect_t	clip_rect;
float	mdl_scale[3] = {1, 1, 1};
float	bitmap_scale[2] = {1, 1};

float	view_angle = 90, view_size = 100;
double	ctg_fov;

void	ComputeViewMatrix ()
{
	for (int i=0; i < 3; i++)
	{
		view_matrix[0][i] = float(main_matrix[0][i]*proj_scale_x);
        view_matrix[1][i] = main_matrix[1][i];
		view_matrix[2][i] = float(main_matrix[2][i]*proj_scale_y);
	}
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

void	SetViewInfo (vec3_t pos, vec3_t ang)
{//	задаем положение и направление камеры
	VectorCopy(pos, cam_pos);
	memset(main_matrix, 0, sizeof(main_matrix));
	main_matrix[0][0] = main_matrix[1][1] = main_matrix[2][2] = 1;
	RotateVector1(main_matrix[0], ang);
	RotateVector1(main_matrix[1], ang);
	RotateVector1(main_matrix[2], ang);
	ComputeViewMatrix();
}

float	DistToCamera (t_plane *plane)
{
	if (plane->type < 3)
		return	cam_pos[plane->type] - plane->dist;
	else
		return	DotProduct (plane->normal, cam_pos) - plane->dist;
}

float	Dist2FromCamera (vec3_t in)
{
	vec3_t	temp;
    VectorSubtract(in, cam_pos, temp);
	return	DotProduct(temp, temp);
}

void	ComputeViewFrustrum ()
{//	расчитываем плоскости, которыми ограничена область видимости камеры
	frustrum_planes[0].normal[0] = float(main_matrix[1][0] - main_matrix[0][0]*ctg_fov);
	frustrum_planes[0].normal[1] = float(main_matrix[1][1] - main_matrix[0][1]*ctg_fov);
	frustrum_planes[0].normal[2] = float(main_matrix[1][2] - main_matrix[0][2]*ctg_fov);
	frustrum_planes[0].dist = DotProduct(frustrum_planes[0].normal, cam_pos);

	frustrum_planes[1].normal[0] = float(main_matrix[1][0] + main_matrix[0][0]*ctg_fov);
	frustrum_planes[1].normal[1] = float(main_matrix[1][1] + main_matrix[0][1]*ctg_fov);
	frustrum_planes[1].normal[2] = float(main_matrix[1][2] + main_matrix[0][2]*ctg_fov);
	frustrum_planes[1].dist = DotProduct(frustrum_planes[1].normal, cam_pos);

	frustrum_planes[2].normal[0] = float(main_matrix[1][0] + main_matrix[2][0]*ctg_fov);
	frustrum_planes[2].normal[1] = float(main_matrix[1][1] + main_matrix[2][1]*ctg_fov);
	frustrum_planes[2].normal[2] = float(main_matrix[1][2] + main_matrix[2][2]*ctg_fov);
	frustrum_planes[2].dist = DotProduct(frustrum_planes[2].normal, cam_pos);

	frustrum_planes[3].normal[0] = float(main_matrix[1][0] - main_matrix[2][0]*ctg_fov);
	frustrum_planes[3].normal[1] = float(main_matrix[1][1] - main_matrix[2][1]*ctg_fov);
	frustrum_planes[3].normal[2] = float(main_matrix[1][2] - main_matrix[2][2]*ctg_fov);
	frustrum_planes[3].dist = DotProduct(frustrum_planes[3].normal, cam_pos);
}

void	TransformPointRaw (vec3_t out, vec3_t in)
{
	vec3_t	temp;
	VectorSubtract(in, cam_pos, temp);
	out[0] = DotProduct(view_matrix[0], temp);
	out[2] = DotProduct(view_matrix[1], temp);
	out[1] = DotProduct(view_matrix[2], temp);
}

void	TransformVector (vec3_t out, vec3_t in)
{
	vec3_t	temp;
	VectorCopy(in, temp);
	out[0] = DotProduct(view_matrix[0], temp);
	out[2] = DotProduct(view_matrix[1], temp);
	out[1] = DotProduct(view_matrix[2], temp);
}

void	ProjectPointToScreen (point_3d *p)
{
	if (p->p[2] >= near_clip)
	{
		double div = 1.0 / p->p[2];
		p->div_z = float(div*65536.0);
		p->sx = float_to_fix( p->p[0]*div + xcenter);
		p->sy = float_to_fix(-p->p[1]*div + ycenter);
	}
}

void	CodePoint (point_3d *p)
{
	p->ccodes = (p->p[2] > 0) ? 0 : CC_BEHIND;
	if (p->p[0] > p->p[2] * right_clip)	p->ccodes |= CC_OFF_RIGHT;
	if (p->p[0] < p->p[2] * left_clip)	p->ccodes |= CC_OFF_LEFT;
	if (p->p[1] > p->p[2] * top_clip)		p->ccodes |= CC_OFF_TOP;
	if (p->p[1] < p->p[2] * bottom_clip)	p->ccodes |= CC_OFF_BOT;
}

void	TransformPoint (point_3d *p, vec3_t v)
{
	TransformPointRaw(p->p, v);
	CodePoint(p);
	ProjectPointToScreen(p);
}

float	PointPointDist2 (vec3_t a, vec3_t b)
{
	vec3_t	sub;
	VectorSubtract(a, b, sub);
	return	(sub[0]*sub[0]) + (sub[1]*sub[1]) + (sub[2]*sub[2]);
}

void	CalculateCameraParameters ()
{
	float	size = view_size*0.01f;
	double	fov = view_angle*pi/360;
	ctg_fov = 1.0/tan(fov);
	mip_scale = 640/(size*sx_size);
	if(view_angle<90)
		mip_scale /= ctg_fov;
	double	temp = ctg_fov*size;
	proj_scale_x = (sx_size*0.5)*temp;
	proj_scale_y = (sy_size*2.0/3.0)*temp;

	int		sx = (sx_size>>1); 
	int		sy = (sy_size>>1); 
	xcenter = sx - 0.5f;
	ycenter = sy - 0.5f;
	right_clip  =  float(floor(sx*size));
	left_clip   = -float(floor(sx*size));
	top_clip    =  float(floor(sy*size));
	bottom_clip = -float(floor(sy*size));
	sx_min = float_to_int(left_clip + sx);
	sx_max = float_to_int(right_clip + sx);
	sy_min = float_to_int(-top_clip + sy);
	sy_max = float_to_int(-bottom_clip + sy);
}

float	det (float *x, float *y, float *z)
{
	float a1 = x[0]*((y[1]*z[2])-(y[2]*z[1]));
    float a2 = y[0]*((z[1]*x[2])-(z[2]*x[1]));
    float a3 = z[0]*((x[1]*y[2])-(x[2]*y[1]));
    return	a1+a2+a3;
}

float	PointPlaneDist (vec3_t pos, t_plane *plane)
{	//	находит положение точки относительно плоскости (0, 1)
	if (plane->type < 3)
		return	pos[plane->type] - plane->dist;
	else
		return	DotProduct (plane->normal, pos) - plane->dist;
}

void	SaveViewInfo ()
{
	depth++;
	memcpy(temp_main_matrix[depth], main_matrix, sizeof(main_matrix));
	memcpy(temp_view_matrix[depth], view_matrix, sizeof(view_matrix));
	frustrum_planes = temp_frustrum_planes[depth+1];
	VectorCopy(cam_pos, temp_cam_pos[depth]);
}
void	RestoreViewInfo ()
{
	memcpy(main_matrix, temp_main_matrix[depth], sizeof(main_matrix));
	memcpy(view_matrix, temp_view_matrix[depth], sizeof(view_matrix));
	frustrum_planes = temp_frustrum_planes[depth];
	VectorCopy(temp_cam_pos[depth], cam_pos);
	depth--;
}
void	RestorePreviousViewInfo (int sub_depth)
{
	int		new_depth = depth - sub_depth;
	memcpy(main_matrix, temp_main_matrix[new_depth], sizeof(main_matrix));
	memcpy(view_matrix, temp_view_matrix[new_depth], sizeof(view_matrix));
	VectorCopy(temp_cam_pos[new_depth], cam_pos);
}
int		BoxInsidePlane (float *mins, float *maxs, t_plane *plane)
{
	float	pt[3];
	for (int i=0; i < 3; i++)
		if(*(int *)&plane->normal[i]>=0)
			pt[i] = maxs[i];
		else
			pt[i] = mins[i];
	return	DotProduct(plane->normal, pt)>plane->dist;
}

int		BoxInFrustrum (float *mins, float *maxs)
{
	if(!BoxInsidePlane(mins, maxs, frustrum_planes+0)
	|| !BoxInsidePlane(mins, maxs, frustrum_planes+1)
	|| !BoxInsidePlane(mins, maxs, frustrum_planes+2)
	|| !BoxInsidePlane(mins, maxs, frustrum_planes+3))
		return	0;
	return	1;
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
void	QuickRotateVector (vec3_t vec, float matrix[3][3])
{
	vec3_t	tmp;
	VectorCopy(vec, tmp);
	vec[0] = tmp[0]*matrix[0][0] + tmp[1]*matrix[1][0] + tmp[2]*matrix[2][0];
	vec[1] = tmp[0]*matrix[0][1] + tmp[1]*matrix[1][1] + tmp[2]*matrix[2][1];
	vec[2] = tmp[0]*matrix[0][2] + tmp[1]*matrix[1][2] + tmp[2]*matrix[2][2];
}
void	QuickTransformVector (vec3_t vec, float matrix[3][3])
{
	vec3_t	tmp;
	VectorCopy(vec, tmp);
	vec[0] = DotProduct(tmp, matrix[0]);
	vec[1] = DotProduct(tmp, matrix[1]);
	vec[2] = DotProduct(tmp, matrix[2]);
}

void	CrossProduct (vec3_t a, vec3_t b, vec3_t cross)
{
	cross[0] = (a[1]*b[2]) - (a[2]*b[1]);
	cross[1] = (a[2]*b[0]) - (a[0]*b[2]);
	cross[2] = (a[0]*b[1]) - (a[1]*b[0]);
}

float	DotProduct (vec3_t a, vec3_t b)
{
	return	(a[0]*b[0]) + (a[1]*b[1]) + (a[2]*b[2]);
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

void	VectorScale (vec3_t a, float scale, vec3_t c)
{
	c[0] = a[0] * scale;
	c[1] = a[1] * scale;
	c[2] = a[2] * scale;
}

void	VectorAddScale (vec3_t a, vec3_t b, float scale, vec3_t c)
{
	c[0] = a[0] + (b[0]*scale);
	c[1] = a[1] + (b[1]*scale);
	c[2] = a[2] + (b[2]*scale);
}

void	VectorInverse (vec3_t v)
{
	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
}

int		ClipScreenX(int a)
{
	if (a<0)		a = 0;
	if (a>=sx_size)	a = sx_size-1;
	return	a;
}

int		ClipScreenY(int a)
{
	if (a<0)		a = 0;
	if (a>=sy_size)	a = sy_size-1;
	return	a;
}