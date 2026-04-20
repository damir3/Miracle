/*
	math3d.c		Copyright (C) 1998-2000 Damir Sagidullin
*/
#include <memory.h>
#include <math.h>

#include "math3d.h"
#include "map.h"

//------------------- DEFINITIONS & CONSTANTS -------------------//

//-------------------------- VARIABLES --------------------------//

vector	vec3_maxs = {1000000, 1000000, 1000000};
vector	vec3_mins = {-1000000, -1000000, -1000000};
float	dist_eps = 0;

extern int		compute_curves;

//-------------------------- FUNCTIONS --------------------------//

float	det (float *x, float *y, float *z)
{
	float a1 = x[0]*((y[1]*z[2])-(y[2]*z[1]));
    float a2 = y[0]*((z[1]*x[2])-(z[2]*x[1]));
    float a3 = z[0]*((x[1]*y[2])-(x[2]*y[1]));
    return	a1+a2+a3;
}

float	PointPlaneDist (vector pos, plane_t *plane)
{
	/*if (plane->type < 3)
		return	pos[plane->type] - plane->dist;
	else*/
	return	DotProduct (plane->normal, pos) - plane->dist;
}

float	PointPointDist2 (vector a, vector b)
{
	vector	sub;
	VectorSubtract(a, b, sub);
	return	(sub[0]*sub[0]) + (sub[1]*sub[1]) + (sub[2]*sub[2]);
}

void	RotateVector1 (vector vec, vector ang)
{
	double	sin_, cos_, angle;
    double	x, y, z;
	// roll
	angle = ang[1] * pi / 0x8000;
    sin_ = sin(angle);
	cos_ = cos(angle);
	z =  cos_ * vec[2] + sin_ * vec[0];
	x = -sin_ * vec[2] + cos_ * vec[0];
	vec[0] = (float)(x);
	vec[2] = (float)(z);
	// pitch
	angle = ang[0] * pi / 0x8000;
	sin_ = sin(angle);
	cos_ = cos(angle);
	y =  cos_ * vec[1] + sin_ * vec[2];
	z = -sin_ * vec[1] + cos_ * vec[2];
	vec[1] = (float)(y);
	vec[2] = (float)(z);
	// yaw
	angle = ang[2] * pi / 0x8000;
    sin_ = sin(angle);
	cos_ = cos(angle);
	x =  cos_ * vec[0] + sin_ * vec[1];
	y = -sin_ * vec[0] + cos_ * vec[1];
	vec[0] = (float)(x);
	vec[1] = (float)(y);
}

void	RotateVector (vector vec, vector ang)
{
	double	sin_, cos_, angle;
    double	x, y, z;
	// yaw
	angle = ang[2] * pi / 0x8000;
    sin_ = sin(angle);
	cos_ = cos(angle);
	x =  cos_ * vec[0] + sin_ * vec[1];
	y = -sin_ * vec[0] + cos_ * vec[1];
	vec[0] = (float)(x);
	vec[1] = (float)(y);
	// pitch
	angle = ang[0] * pi / 0x8000;
	sin_ = sin(angle);
	cos_ = cos(angle);
	y =  cos_ * vec[1] + sin_ * vec[2];
	z = -sin_ * vec[1] + cos_ * vec[2];
	vec[1] = (float)(y);
	vec[2] = (float)(z);
	// roll
	angle = ang[1] * pi / 0x8000;
    sin_ = sin(angle);
	cos_ = cos(angle);
	z =  cos_ * vec[2] + sin_ * vec[0];
	x = -sin_ * vec[2] + cos_ * vec[0];
	vec[0] = (float)(x);
	vec[2] = (float)(z);
}

void	QuickRotateVector (vector vec, float matrix[3][3])
{
	vector	tmp;
	VectorCopy (vec, tmp);
	vec[0] = tmp[0]*matrix[0][0] + tmp[1]*matrix[1][0] + tmp[2]*matrix[2][0];
	vec[1] = tmp[0]*matrix[0][1] + tmp[1]*matrix[1][1] + tmp[2]*matrix[2][1];
	vec[2] = tmp[0]*matrix[0][2] + tmp[1]*matrix[1][2] + tmp[2]*matrix[2][2];
}

void	TransformVector (vector vec, float matrix[3][3])
{
	vector	temp;
	VectorCopy (vec, temp);
	vec[0] = DotProduct(temp, matrix[0]);
	vec[1] = DotProduct(temp, matrix[1]);
	vec[2] = DotProduct(temp, matrix[2]);
}

float	BBoxRadius (vector mins, vector maxs, vector pos)
{//	finding radius of area (centered at given point) which
//	encompasses bounding box
	int		i, j, k;
	vector	point;
	float	cur_radius, radius=0;

	for (i=0; i<2; i++)
	{
		if (i)	point[0] = mins[0];
		else	point[0] = maxs[0];
		for (j=0; j<2; j++)
		{
			if (j)	point[1] = mins[1];
			else	point[1] = maxs[1];
			for (k=0; k<2; k++)
			{
				if (k)	point[2] = mins[2];
				else	point[2] = maxs[2];
				cur_radius = PointPointDist2 (point, pos);
				if (radius<cur_radius)	radius = cur_radius;
			}
		}
	}
	return	(float)(sqrt(radius));
}

int		FindSector (int num, vector p)
{
	while (num >= 0)
	{
		node_t	*node = &map.nodes[num];
		num = node->children[PointPlaneDist(p, &map.planes[node->plane])<0];
	}
	return	~num;
}

//-------------------------- VARIABLES --------------------------//

static vector	position;

static trace_t	*trace;
static vector	move_from, move_to;

//-------------------------- FUNCTIONS --------------------------//


/*static void	CheckLineBrushCollision (int brush_idx)
{
	brush_t		*brush = map.brushes + brush_idx;
	plane_t		*plane;

	float	p1[3], p2[3], mid[3], d1, d2, d;
	float	dot1, dot2;
	int		i, k, num, start_solid;
	int		cside;
	plane_t	cplane;
	shaderref_t	*shader = map.shaders + brush->contents;

	//if (shader->content_flags > 0x10000) return;
	//if (shader->surface_flags & 4) return;
    if (shader->content_flags >= CONTENTS_AREAPORTAL) return;
    if (shader->surface_flags & SURF_SKY) return;
    if (compute_curves && shader->surface_flags & SURF_NODRAW) return;

	d1 = 0.0; d2 = trace->fraction;
	VectorCopy (move_from, p1);
	VectorCopy (move_to, p2);

	start_solid = TRUE;

	num = brush->numsides;
	cside = -1;

	for (i=k=0; i<num; i++)
	{
		plane = map.planes + map.brushsides[brush->firstside + i].planenum;

		dot1 = DotProduct (plane->normal, p1) - plane->dist - dist_eps;
		dot2 = DotProduct (plane->normal, p2) - plane->dist - dist_eps;

		if (dot1>=0 && dot2>=0)	break;
		if (dot1<0 && dot2<0)	continue;

		d = dot1/(dot1 - dot2);

		VectorSubtract (p2, p1, mid);
		VectorAddScale (p1, mid, d, mid);
		d = d1 + (d2-d1)*d;

		if (dot1 >= 0)
		{
			VectorCopy (mid, p1);
			d1 = d;
			cplane = *plane;
			start_solid = FALSE;
			cside = i;
		} else
		{
			VectorCopy (mid, p2);
			d2 = d;
		}
	}

	if (i==num && cside>=0)
	{
		if (start_solid)
		{
			VectorCopy (move_from, move_to);
			trace->fraction = 0.0;
			trace->contents = brush->contents;
			return;
		}
		VectorCopy (p1, move_to);
		trace->fraction = d1;
		//VectorCopy (cplane.normal, trace->normal);
		//trace->dist = cplane.dist;
		//trace->contents = brush->contents;
	}
}

static void	RecursiveCheckLineSectorsCollision (int n)
{
	plane_t	*plane;
	node_t	*node;
	sector_t	*sector;
	int		i, brush;
	float	dist1, dist2;

	while (n >= 0)
	{
		node = map.nodes + n;
		plane = map.planes + node->plane;

		dist1 = PointPlaneDist (move_from, plane);
		dist2 = PointPlaneDist (move_to, plane);
		if ((dist1>=0) && (dist2>=0)) n = node->children[0];
		else
		if ((dist1<0) && (dist2<0)) n = node->children[1];
		else
		{
			n = node->children[0];
			RecursiveCheckLineSectorsCollision (node->children[1]);
		}
	}
	n = ~n;
	if (n>0)
	{
		sector = map.sectors + n;
		for (i=0; i<sector->numbrushes; i++)
		{
			brush = map.sectorbrushes[i + sector->firstbrush];
			CheckLineBrushCollision (brush);
		}
	}
}

static void	CheckLineModelCollision (int model, vector from, vector to, trace_t *trace_struct)
{
	int		i, brush;

	VectorCopy (from, move_from);
	VectorCopy (to, move_to);
	trace = trace_struct;

	for (i=0; i<3; i++)
	{
		if (from[i] > to[i])
		{
			if (from[i] < map.models[model].bbox.mins[i]) break;
			if (to[i] > map.models[model].bbox.maxs[i]) break;
		} else
		{
			if (to[i] < map.models[model].bbox.mins[i]) break;
			if (from[i] > map.models[model].bbox.maxs[i]) break;
		}
	}
	if (i < 3) return;

	if (model == 0)
		RecursiveCheckLineSectorsCollision (0);
	else
	{
		brush = map.models[model].firstbrush;
		i = map.models[model].numbrushes;
		while ((i--) > 0) CheckLineBrushCollision (brush++);
	}

	VectorCopy (move_from, from);
	VectorCopy (move_to, to);
}

int		TraceLine (vector start_, vector end_, trace_t *trace)
{
	int		model;

	VectorCopy (start_, trace->start);
	VectorCopy (end_, trace->end);
	trace->fraction = 1.0f;

	CheckLineModelCollision (0, trace->start, trace->end, trace);
	if (trace->fraction < 1.0f) return FALSE;
	for (model=1; model<map.nummodels; model++)
	{
		//printf ("\r%d", model);
		CheckLineModelCollision (model, trace->start, trace->end, trace);
		if (trace->fraction < 1.0f) return FALSE;
	}

	return TRUE;
}*/
static void	CheckLineBrushCollision (int brush_idx)
{
	brush_t		*brush = map.brushes + brush_idx;
	plane_t		*plane;

	float	p1[3], p2[3], mid[3], d1, d2, d;
	float	dot1, dot2;
	int		i, num, start_solid;
	int		cside;
	plane_t	cplane;

	if (map.shaders[brush->contents].content_flags >= CONTENTS_AREAPORTAL) return;
	if (map.shaders[brush->contents].surface_flags & SURF_SKY) return;
	if (compute_curves && map.shaders[brush->contents].surface_flags & SURF_NODRAW) return;

	d1 = 0.0; d2 = trace->fraction;
	VectorCopy (move_from, p1);
	VectorCopy (move_to, p2);

	start_solid = 1;

	num = brush->numsides;
	cside = -1;

	for (i=0; i<num; i++)
	{
		plane = map.planes + map.brushsides[brush->firstside + i].planenum;

		dot1 = DotProduct (plane->normal, p1) - plane->dist;// - dist_eps;
		dot2 = DotProduct (plane->normal, p2) - plane->dist;// - dist_eps;

		if (dot1>=0 && dot2>=0)	break;
		if (dot1<0 && dot2<0)	continue;

		d = dot1/(dot1 - dot2);

		VectorSubtract (p2, p1, mid);
		VectorAddScale (p1, mid, d, mid);
		d = d1 + (d2-d1)*d;

		if (dot1 >= 0) {
			VectorCopy (mid, p1);
			d1 = d;
			cplane = *plane;
			start_solid = 0;
			cside = i;
		} else {
			VectorCopy (mid, p2);
			d2 = d;
		}
	}

	if (i==num && cside>=0)
	{
		//if (start_solid)
		//{
		//	VectorCopy (move_from, move_to);
		//	trace->fraction = 0.0;
		//	return;
		//}
		VectorCopy (p1, move_to);
		trace->fraction = d1;
	}
}

static void	RecursiveCheckLineSectorsCollision (int n)
{
	plane_t	*plane;
	node_t	*node;
	sector_t	*sector;
	int		i, brush;
	float	dist1, dist2;

	while (n >= 0)
	{
		node = map.nodes + n;
		plane = map.planes + node->plane;

		dist1 = PointPlaneDist (move_from, plane);
		dist2 = PointPlaneDist (move_to, plane);
		if ((dist1>=0) && (dist2>=0)) n = node->children[0];
		else
		if ((dist1<0) && (dist2<0)) n = node->children[1];
		else
		{
			n = node->children[0];
			RecursiveCheckLineSectorsCollision (node->children[1]);
		}
	}
	sector = map.sectors + (~n);
	for (i=0; i<sector->numbrushes; i++)
	{
		brush = map.sectorbrushes[i + sector->firstbrush];
		CheckLineBrushCollision (brush);
	}
}

static void	CheckLineModelCollision (int model, vector from, vector to, trace_t *trace_struct)
{
	int		i, brush;

	VectorCopy (from, move_from);
	VectorCopy (to, move_to);
	trace = trace_struct;

	for (i=0; i<3; i++)
	{
		if (from[i] > to[i])
		{
			if (from[i] < map.models[model].bbox.mins[i]) break;
			if (to[i] > map.models[model].bbox.maxs[i]) break;
		} else
		{
			if (to[i] < map.models[model].bbox.mins[i]) break;
			if (from[i] > map.models[model].bbox.maxs[i]) break;
		}
	}
	if (i < 3) return;

	if (model == 0)
		RecursiveCheckLineSectorsCollision (0);
	else
	{
		brush = map.models[model].firstbrush;
		i = map.models[model].numbrushes;
		while ((i--) > 0) CheckLineBrushCollision (brush++);
	}

	VectorCopy (move_from, from);
	VectorCopy (move_to, to);
}

int		TraceLine (vector start_, vector end_, trace_t *trace)
{
	int		model;

	VectorCopy (start_, trace->start);
	VectorCopy (end_, trace->end);
	trace->fraction = 1.0f;

	CheckLineModelCollision (0, trace->start, trace->end, trace);
	if (trace->fraction < 1.0f) return FALSE;
	for (model=1; model<map.nummodels; model++)
	{
		//printf ("\r%d", model);
		CheckLineModelCollision (model, trace->start, trace->end, trace);
		if (trace->fraction < 1.0f) return FALSE;
	}

	return TRUE;
}

void	MeshGetPoint (float u, float v, face_t *face, vector out)
{
	int		i, x, y;
	float	t;
    vector	tp[3];
	vertex_t	*p = map.vertexes + face->firstvert;

	x = (int)(u*(face->mesh_cp[0]>>1));
	y = (int)(v*(face->mesh_cp[1]>>1));

	p += x*2 + (y*2*face->mesh_cp[0]);

	t = u*(face->mesh_cp[0]>>1) - x;
	for (i=0; i<3; i++)
	{
		VectorScale (p[0].v_point, (1-t)*(1-t), tp[i]);
		VectorAddScale (tp[i], p[1].v_point, 2*t*(1-t), tp[i]);
		VectorAddScale (tp[i], p[2].v_point, t*t, tp[i]);
		p += face->mesh_cp[0];
	}
	t = v*(face->mesh_cp[1]>>1) - y;
	VectorScale (tp[0], (1-t)*(1-t), out);
	VectorAddScale (out, tp[1], 2*t*(1-t), out);
	VectorAddScale (out, tp[2], t*t, out);
}

void	MeshGetNormal (float u, float v, face_t *face, vector out)
{
	int		i, x, y;
	float	t;
    vector	tp[3];
	vertex_t    *p = map.vertexes + face->firstvert;

	x = (int)(u*(face->mesh_cp[0]>>1));
	y = (int)(v*(face->mesh_cp[1]>>1));

	p += x*2 + (y*2*face->mesh_cp[0]);

	t = u*(face->mesh_cp[0]>>1) - x;
	for (i=0; i<3; i++)
	{
		VectorScale (p[0].v_norm, (1-t)*(1-t), tp[i]);
		VectorAddScale (tp[i], p[1].v_norm, 2*t*(1-t), tp[i]);
		VectorAddScale (tp[i], p[2].v_norm, t*t, tp[i]);
		p += face->mesh_cp[0];
	}
	t = v*(face->mesh_cp[1]>>1) - y;
	VectorScale (tp[0], (1-t)*(1-t), out);
	VectorAddScale (out, tp[1], 2*t*(1-t), out);
	VectorAddScale (out, tp[2], t*t, out);
}

float	PointBBoxfDist2 (vector pos, bboxf_t *bbox)
{
	int		i;
	vector	dir;

	for (i=0; i<3; i++)
	{
		if (pos[i] < bbox->mins[i])
			dir[i] = bbox->mins[i] - pos[i];
		else if (pos[i] > bbox->maxs[i])
			dir[i] = pos[i] - bbox->maxs[i];
		else dir[i] = 0;
	}
	return DotProduct (dir, dir);
}
