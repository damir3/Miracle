/*
	DrawModel.cpp	Copyright (C) 1998-1999 Damir Sagidullin
*/
#include	<Math.h>
#include	<Memory.h>

#include	"Const.h"
#include	"Draw_2D.h"	//	delete
#include	"DrawModel.h"
#include	"DrawFace.h"
#include	"DrawSpan.h"
#include	"SpanBuf.h"
#include	"MapFile.h"
#include	"Math_3D.h"
#include	"Render.h"
#include	"Surface.h"
#include	"Video.h"

extern int		mirror, gamma;
extern ushort	transparency_map[16][0x10000];
extern ushort	red_color_map[64][256];
extern ushort	green_color_map[64][256];
extern ushort	blue_color_map[64][256];
extern int				num_dyn_lights;
extern dynamic_light	dyn_lights[MAX_NUM_DYNAMIC_LIGHTS];

typedef void	(*DS)();
#define	MAX_NUM_MDL_VERTICES	0x2000

int			numobjects=0;
object_t	vis_objects[256];

static ushort	*alpha_map1, *alpha_map2;
static DS		DrawCurSpan;
static int		cur_sy, sy_global_start, sy_global_end;
static EdgeVertex	edge_buf[MAX_SY_SIZE][2];
static byte		*texture;
static float	max_dist;
static vec3_t	position;
static vec3_t	rgb[MAX_NUM_MDL_VERTICES];
static vec3_t	tvnormals[MAX_NUM_MDL_VERTICES];
static point_3d	tvertices[MAX_NUM_MDL_VERTICES];
static point_3d	*pts[32];
static int		num_stat_obj_lights, num_dyn_obj_lights;
static t_light	*stat_obj_lights[256];
static dynamic_light	*dyn_obj_lights[256];

void	ComputeFaceEdge(point_3d *a, point_3d *b)
{
    if((a->p[2]<0) && (b->p[2]<0))	return;

	int		right;
	if(a->sy > b->sy)
	{
		point_3d *temp = a;
		a = b;
		b = temp;
		right = 0;
	}
	else
		right = 1;

	int sy		= fix_to_int(a->sy);
	int sy_end	= fix_to_int(b->sy);

	if(sy>=sy_end)	return;

	if(sy<0)		sy = 0;
	if(sy_end>sy_size)	sy_end = sy_size;
	if(sy_global_start>sy)		sy_global_start = sy;
	if(sy_global_end<sy_end)	sy_global_end = sy_end;

	double	div = 65536.0 / (b->sy - a->sy);
	int		dsx = float_to_int((b->sx - a->sx) * div);
	float	ddz = float((b->div_z - a->div_z) * div);
	float	duz = float(((b->u*b->div_z) - (a->u*a->div_z)) * div);
	float	dvz = float(((b->v*b->div_z) - (a->v*a->div_z)) * div);
	float	drz = float(((b->r*b->div_z) - (a->r*a->div_z)) * div);
	float	dgz = float(((b->g*b->div_z) - (a->g*a->div_z)) * div);
	float	dbz = float(((b->b*b->div_z) - (a->b*a->div_z)) * div);


	double	temp = ((sy<<16) - a->sy)/65536.0;
	int		sx = a->sx + float_to_int((double) dsx * temp);
	float	uz = float((a->u*a->div_z) + (duz * temp));
	float	vz = float((a->v*a->div_z) + (dvz * temp));
	float	rz = float((a->r*a->div_z) + (drz * temp));
	float	gz = float((a->g*a->div_z) + (dgz * temp));
	float	bz = float((a->b*a->div_z) + (dbz * temp));
	float	dz = float(a->div_z + (ddz * temp));

	if(mirror)
	{
		right ^= 1;
		sx = (sx_size<<16)-65535-sx;
		dsx = -dsx;
	}

	for(; sy < sy_end; sy++)
	{
		edge_buf[sy][right].sx = sx;
		edge_buf[sy][right].dz = dz;
		edge_buf[sy][right].uz = uz;
		edge_buf[sy][right].vz = vz;
		edge_buf[sy][right].rz = rz;
		edge_buf[sy][right].gz = gz;
		edge_buf[sy][right].bz = bz;
		sx += dsx;
		dz += ddz;
		uz += duz;
		vz += dvz;
		rz += drz;
		gz += dgz;
		bz += dbz;
	}
}

void	DrawCurrentSpan()
{
	int		sx_start = fix_to_int(edge_buf[cur_sy][0].sx);
	int		sx_end = fix_to_int(edge_buf[cur_sy][1].sx);
	if(sx_start>=sx_end)	return;

	TSpan	cur_span;
	cur_span.sx_start = sx_start;
	cur_span.sx_end = sx_end;
	cur_span.sx0 = edge_buf[cur_sy][0].sx;
	cur_span.dz = edge_buf[cur_sy][0].dz;
	cur_span.ddz = (edge_buf[cur_sy][1].dz-edge_buf[cur_sy][0].dz)/(edge_buf[cur_sy][1].sx - edge_buf[cur_sy][0].sx);

	num_draw_spans = 0;
	if(!ClipSpan(cur_sy, &cur_span))	return;

	double	z = 65536.0 / edge_buf[cur_sy][0].dz;
	double	u0 = edge_buf[cur_sy][0].uz * z;
	double	v0 = edge_buf[cur_sy][0].vz * z;
	double	r0 = edge_buf[cur_sy][0].rz * z;
	double	g0 = edge_buf[cur_sy][0].gz * z;
	double	b0 = edge_buf[cur_sy][0].bz * z;
			z = 65536.0 / edge_buf[cur_sy][1].dz;
	double	u1 = edge_buf[cur_sy][1].uz * z;
	double	v1 = edge_buf[cur_sy][1].vz * z;
	double	r1 = edge_buf[cur_sy][1].rz * z;
	double	g1 = edge_buf[cur_sy][1].gz * z;
	double	b1 = edge_buf[cur_sy][1].bz * z;
	double	div = 65536.0 / (edge_buf[cur_sy][1].sx - edge_buf[cur_sy][0].sx);
	int		du = float_to_int((u1 - u0) * div);
	int		dv = float_to_int((v1 - v0) * div);
	int		dr = float_to_int((r1 - r0) * div);
	int		dg = float_to_int((g1 - g0) * div);
	int		db = float_to_int((b1 - b0) * div);

	for(int i=0, j=0; i<num_draw_spans; i++)
	{
		int		start = spans_start[i];
		int		dsx = spans_end[i] - start;
		if(dsx<=0)	continue;

		float	temp = start - (edge_buf[cur_sy][0].sx/65536.0f);

		int		u = float_to_int(u0 + (temp * du));
		int		v = float_to_int(v0 + (temp * dv));
		int		r = float_to_int(r0 + (temp * dr));
		int		g = float_to_int(g0 + (temp * dg));
		int		b = float_to_int(b0 + (temp * db));

		ushort	*dest = (ushort *)virtual_screen + screen_row_table[cur_sy] + start;
		while(dsx-- > 0)
		{
			int		pos = fix_int(u) + tex_row_table[fix_int(v)];
			pos += pos<<1;
			*dest++ = red_color_map[fix_int(r)][texture[pos]] +
					green_color_map[fix_int(g)][texture[pos+1]] +
					blue_color_map[fix_int(b)][texture[pos+2]];
			u += du;
			v += dv;
			r += dr;
			g += dg;
			b += db;
		}
		j++;
	}
	if(j)	AddSpan(cur_sy, &cur_span);
}
void	DrawCurrentTransparentSpan()
{
	int		sx_start = fix_to_int(edge_buf[cur_sy][0].sx);
	int		sx_end = fix_to_int(edge_buf[cur_sy][1].sx);
	if(sx_start>=sx_end)	return;

	TSpan	cur_span;
	cur_span.sx_start = sx_start;
	cur_span.sx_end = sx_end;
	cur_span.sx0 = edge_buf[cur_sy][0].sx;
	cur_span.dz = edge_buf[cur_sy][0].dz;
	cur_span.ddz = (edge_buf[cur_sy][1].dz-edge_buf[cur_sy][0].dz)/(edge_buf[cur_sy][1].sx - edge_buf[cur_sy][0].sx);

	num_draw_spans = 0;
	if(!ClipSpan(cur_sy, &cur_span))	return;

	double	z = 65536.0 / edge_buf[cur_sy][0].dz;
	double	u0 = edge_buf[cur_sy][0].uz * z;
	double	v0 = edge_buf[cur_sy][0].vz * z;
	double	r0 = edge_buf[cur_sy][0].rz * z;
	double	g0 = edge_buf[cur_sy][0].gz * z;
	double	b0 = edge_buf[cur_sy][0].bz * z;
			z = 65536.0 / edge_buf[cur_sy][1].dz;
	double	u1 = edge_buf[cur_sy][1].uz * z;
	double	v1 = edge_buf[cur_sy][1].vz * z;
	double	r1 = edge_buf[cur_sy][1].rz * z;
	double	g1 = edge_buf[cur_sy][1].gz * z;
	double	b1 = edge_buf[cur_sy][1].bz * z;
	double	div = 65536.0 / (edge_buf[cur_sy][1].sx - edge_buf[cur_sy][0].sx);
	int		du = float_to_int((u1 - u0) * div);
	int		dv = float_to_int((v1 - v0) * div);
	int		dr = float_to_int((r1 - r0) * div);
	int		dg = float_to_int((g1 - g0) * div);
	int		db = float_to_int((b1 - b0) * div);
	
	for(int i=0, j=0; i<num_draw_spans; i++)
	{
		int		start = spans_start[i];
		int		dsx = spans_end[i] - start;
		if(dsx<=0)	continue;

		float	temp = start - (edge_buf[cur_sy][0].sx/65536.0f);

		int		u = float_to_int(u0 + (temp * du));
		int		v = float_to_int(v0 + (temp * dv));
		int		r = float_to_int(r0 + (temp * dr));
		int		g = float_to_int(g0 + (temp * dg));
		int		b = float_to_int(b0 + (temp * db));

		ushort	*dest = (ushort *)virtual_screen + screen_row_table[cur_sy] + start;
		while(dsx-- > 0)
		{
			int		pos = fix_int(u) + tex_row_table[fix_int(v)];
			pos += pos<<1;
			
			ushort	c = red_color_map[fix_int(r)][texture[pos]] +
					green_color_map[fix_int(g)][texture[pos+1]] +
					blue_color_map[fix_int(b)][texture[pos+2]];
			*dest++ = alpha_map1[c] + alpha_map2[*dest];
			u += du;
			v += dv;
			r += dr;
			g += dg;
			b += db;
		}
		j++;
	}
	if(j)	AddSpan(cur_sy, &cur_span);
}

void	DrawTexturedFace(int n, point_3d **vert)
{
	sy_global_start = sy_size + 1;
	sy_global_end = -1;

	for(int i=0; i<n-1; i++)
		ComputeFaceEdge(vert[i], vert[i+1]);
	ComputeFaceEdge(vert[i], vert[0]);

	if(sy_global_start>=sy_global_end)	return;
	for(cur_sy = sy_global_start; cur_sy<sy_global_end; cur_sy++)
		DrawCurSpan();
}

int		VisCheck(int n)
{
	t_plane	*plane;
	while (n >= 0)
	{
		t_node	*node = &map.nodes[n];
		plane = &map.planes[node->planenum];
		float	dist = PointPlaneDist(position, plane);
		if(dist<0)
		{
			n = node->children[1];
			if(dist>-max_dist)
				if(VisCheck(node->children[0]))
					return	1;
		}
		else
		{
			n = node->children[0];
			if(dist<max_dist)
				if(VisCheck(node->children[1]))
					return	1;
		}
	}
	int		sector = ~n;
	return	(vis_sectors[sector>>3] & (1<<(sector&7)));
}
int		VisibleCheck(vec3_t pos, float dist)
{
	max_dist = dist;
	VectorCopy(pos, position);
	return	VisCheck(map.models[0].headnode[0]);
}

void	FindNearestLights(vec3_t position)
{
	num_stat_obj_lights = num_dyn_obj_lights = 0;
	float	dist;
	int		sector = FindSector(position);
	int		firstlight = map.sectors[sector].firstlight;
	int		numlights = map.sectors[sector].numlights;
	while(numlights--)
	{
		t_light	*light = &map.lights[map.lights_index[firstlight++]];
		dist = PointPointDist2(position, light->pos);
		if(dist<((light->i[0]+light->i[1]+light->i[2])*256))
			stat_obj_lights[num_stat_obj_lights++] = light;
	}
	numlights = num_dyn_lights;
	while(numlights--)
	{
		dynamic_light	*light = &dyn_lights[numlights];
		dist = PointPointDist2(position, light->pos);
		if(dist<light->max_radius2)
			dyn_obj_lights[num_dyn_obj_lights++] = light;
	}
}
vec3_t	sunlightdirection = {0, 0, 1};
vec3_t	sunlight = {96, 96, 64};
void	DrawModel_(object_t *obj)
{
	int		i, j, n, num;
	int		codes_or, codes_and;
	float	width, height, radius, d;
	vec3_t	pos, mins, maxs;
	vec3_t	add, c, temp;
	vec3_t	point;
	point_3d	**vlist;

	if(obj->scale<=0)	return;

	TModel	*model = obj->model;
	VectorCopy(obj->pos, pos);
	radius = model->radius*obj->scale;

	for(i=0; i<3; i++)
	{
		mins[i] = pos[i] - radius;
		maxs[i] = pos[i] + radius;
	}
	if(!BoxInFrustrum(mins, maxs))	return;

	if(!VisibleCheck(pos, radius))	return;
	//DrawString(8, 8, "Object Visible",255);
	if(obj->alpha<15)
	{
		DrawCurSpan = DrawCurrentTransparentSpan;
		alpha_map1 = transparency_map[obj->alpha];
		alpha_map2 = transparency_map[15-obj->alpha];
	} else
		DrawCurSpan = DrawCurrentSpan;

	float	transform_matrix[3][3];
	memset(transform_matrix, 0, sizeof(transform_matrix));
	transform_matrix[0][0] = mdl_scale[0]*obj->scale;
	transform_matrix[1][1] = mdl_scale[1]*obj->scale;
	transform_matrix[2][2] = mdl_scale[2]*obj->scale;
	RotateVector(transform_matrix[0], obj->angle);
	RotateVector(transform_matrix[1], obj->angle);
	RotateVector(transform_matrix[2], obj->angle);


	FindNearestLights(pos);
	for(i=0; i<model->numvertices; i++)
	{
		point[0] = DotProduct(transform_matrix[0], model->vertices[i]) + pos[0];
		point[1] = DotProduct(transform_matrix[1], model->vertices[i]) + pos[1];
		point[2] = DotProduct(transform_matrix[2], model->vertices[i]) + pos[2];
		tvnormals[i][0] = DotProduct(transform_matrix[0], model->vnormals[i]);
		tvnormals[i][1] = DotProduct(transform_matrix[1], model->vnormals[i]);
		tvnormals[i][2] = DotProduct(transform_matrix[2], model->vnormals[i]);

		c[0] = c[1] = c[2] = 96.0f + gamma;
		d = 0.5f + DotProduct(tvnormals[i], sunlightdirection);
		if(d>0)	VectorAddScale(c, sunlight, (d>1) ? 1:d, c);
		j = num_dyn_obj_lights;
		while(j--)
		{
			VectorSubtract(dyn_obj_lights[j]->pos, point, temp);
			VectorScale(temp, float(1.0/sqrt(DotProduct(temp, temp))), temp);
			d = 0.5f + DotProduct(temp, tvnormals[i]);
			if(d>0)
			{
				VectorScale(dyn_obj_lights[j]->i, ((d>1) ? 1:d)/PointPointDist2(point, dyn_obj_lights[j]->pos), add);
				VectorAdd(c, add, c);
			}
		}
		j = num_stat_obj_lights;
		while(j--)
		{
			VectorSubtract(stat_obj_lights[j]->pos, point, temp);
			VectorScale(temp, float(1.0/sqrt(DotProduct(temp, temp))), temp);
			d = 0.5f + DotProduct(temp, tvnormals[i]);
			if(d>0)
			{
				VectorScale(stat_obj_lights[j]->i, ((d>1) ? 1:d)/PointPointDist2(point, stat_obj_lights[j]->pos), add);
				VectorAdd(c, add, c);
			}
		}
		if(c[0]<0)	c[0] = 0;
		if(c[1]<0)	c[1] = 0;
		if(c[2]<0)	c[2] = 0;
		if(c[0]>255)	c[0] = 255;
		if(c[1]>255)	c[1] = 255;
		if(c[2]>255)	c[2] = 255;
		VectorScale(rgb[i], 0.25f, rgb[i]);
		if(obj->type==1)
		{
			rgb[i][0] = (128 + c[0])*0.125f;
			rgb[i][1] = (128 + c[1])*0.125f;
			rgb[i][2] = (128 + c[2])*0.125f;
		} else
		{
			rgb[i][0] = c[0]*0.25f;
			rgb[i][1] = c[1]*0.25f;
			rgb[i][2] = c[2]*0.25f;
		}
		TransformPoint(&tvertices[i], point);
	}

	width = (model->texture.width-1)*0.5f;
	height = (model->texture.height-1)*0.5f;
	clip_mode = 1;
	for(i=0; i<model->numfaces; i++)
	{
		codes_or = 0, codes_and = 0xff;
		for(j=0, n=3; j<n; j++)
		{
			num = model->faces[i].a[j];
			pts[j] = &tvertices[num];
			pts[j]->r = rgb[num][0];
			pts[j]->g = rgb[num][1];
			pts[j]->b = rgb[num][2];
			if(obj->type==1)
			{
				pts[j]->u = width *(1+ tvnormals[num][0]);
				pts[j]->v = height *(1+ tvnormals[num][1]);
			}
			codes_or |= pts[j]->ccodes;
			codes_and &= pts[j]->ccodes;
		}
		if(codes_and)	continue;
		if(!obj->type)
		{
			pts[0]->u = 2;
			pts[0]->v = 2;
			pts[1]->u = float(model->texture.width-2);
			pts[1]->v = 2;
			pts[2]->u = float(model->texture.width-2);
			pts[2]->v = float(model->texture.height-2);
		}
		n = ClipPoly(n, pts, codes_or, &vlist);
		if(!n)	continue;
		if(det( (float *) &pts[0]->p,
				(float *) &pts[1]->p,
				(float *) &pts[2]->p)<0)
		{
			SetTexture((short *)model->texture.bm, model->texture.width, model->texture.height);
			texture = (byte *)current_texture;
			DrawTexturedFace(n, vlist);
		}
	}
}
void	DrawModel(vec3_t pos, vec3_t angle, TModel *model, float scale, byte alpha, byte type)
{
	alpha >>= 4;
	if(!alpha)	return;
	if(numobjects>=256)	return;
	VectorCopy(pos, vis_objects[numobjects].pos);
	VectorCopy(angle, vis_objects[numobjects].angle);
	vis_objects[numobjects].model = model;
	vis_objects[numobjects].scale = scale;
	vis_objects[numobjects].alpha = alpha;
	vis_objects[numobjects].type = type;
	vis_objects[numobjects].mark = 1;
	numobjects++;
}