/*
	Light.cpp	Copyright (C) 1998-1999 Damir Sagidullin
*/
#include	<Stdio.h>
#include	<Memory.h>

#include	"MapFile.h"
#include	"Collide.h"
#include	"Math_3D.h"
#include	"Render.h"
#include	"Console.h"
#include	"Move.h"
#include	"Light.h"
#include	"Video.h"

static int		col[3];
static float	dist1;
static byte		*light;
static int	current_face;

void	CalcRGB(omni_light *l)
{
	col[0] = light[0] + int(dist1*l->r_rate);
	col[1] = light[1] + int(dist1*l->g_rate);
	col[2] = light[2] + int(dist1*l->b_rate);
}

int		CheckCross(vec3_t A, vec3_t B, int planenum, int side);

void	AddOmniLight(omni_light *l)
{
	CPrintf("Add new source of light");
	CPrintf("(R:%d, G:%d, B:%d)", int(l->r_rate/4096), int(l->g_rate/4096), int(l->b_rate/4096));
	CPrintf("Light pos(%f,%f,%f)", l->pos[0], l->pos[1], l->pos[2]);
	float	a[3], b[3], c[3], d[3];
    float	temp, dist;
    int		i, j, k, u0,v0,u1,v1,maxnum, face;
    float	maxrate;
	float	*vecs_u, *vecs_v;
	plane_t	*plane;
    vec3_t	cur_vertex, begin_v, du, dv;
	if(l->r_rate>l->g_rate)
	{
		if(l->r_rate>l->b_rate)
		{
			maxnum = 0;
			maxrate = l->r_rate;
		}
		else
		{
			maxnum = 2;
			maxrate = l->b_rate;
		}
	} else
	{
		if(l->g_rate>l->b_rate)
		{
			maxnum = 1;
			maxrate = l->g_rate;
		} else
		{
			maxnum = 2;
			maxrate = l->b_rate;
		}
	}
	for(face=0; face<map.numfaces; face++)
    {
		if((map.faces[face].type==2) || (map.faces[face].type==3))	continue;
		plane =	&map.planes[map.faces[face].planenum];
        if(map.faces[face].side)
        	dist = -PointPlaneDist(l->pos, plane);
		else
			dist = PointPlaneDist(l->pos, plane);
		if((dist<0) || (dist>=l->max_radius))	continue;
		face_consts	*consts = &map.faces_consts[face];
		i = map.faces[face].lightofs;
		if (i == -1)	continue;
		light = (byte *)(map.light + i);
		current_face = face;
		u0 = consts->u0;
		v0 = consts->v0;
		u1 = consts->u1;
		v1 = consts->v1;
		du[0] = consts->u[0]*16.0f;
		du[1] = consts->u[1]*16.0f;
		du[2] = consts->u[2]*16.0f;
		dv[0] = consts->v[0]*16.0f;
		dv[1] = consts->v[1]*16.0f;
		dv[2] = consts->v[2]*16.0f;
		int		width =	(u1-u0)>>4;
		int		height = (v1-v0)>>4;
		vecs_u = map.texinfo[map.faces[face].texinfo].vecs[0];
        vecs_v = map.texinfo[map.faces[face].texinfo].vecs[1];
		a[0] = plane->normal[0];
		b[0] = plane->normal[1];
		c[0] = plane->normal[2];
		d[0] = plane->dist;
		a[1] = vecs_u[0];
        b[1] = vecs_u[1];
        c[1] = vecs_u[2];
		d[1] = u0-vecs_u[3];
        a[2] = vecs_v[0];
        b[2] = vecs_v[1];
        c[2] = vecs_v[2];
		d[2] = v0-vecs_v[3];
        temp = 1.0f/det(a,b,c);
        float	max_radius_2 = l->max_radius*l->max_radius;
		begin_v[0] = det(d,b,c) * temp;
		begin_v[1] = det(a,d,c) * temp;
		begin_v[2] = det(a,b,d) * temp;
		for (j=0; j<=height; j ++)
		{
			VectorCopy(begin_v, cur_vertex);
			VectorAdd(begin_v, dv, begin_v);
			for (i=0; i<=width; i++, light += 3)
			{
				vec3_t	d;
				VectorSubtract(cur_vertex, l->pos, d);
				VectorAdd(cur_vertex, du, cur_vertex);
        		dist = (d[0]*d[0]) + (d[1]*d[1]) + (d[2]*d[2]);
                if(dist<max_radius_2)
                {
					dist1 = 1.0f/dist;
					if(CheckCross(cur_vertex, l->pos, map.faces[face].planenum, map.faces[face].side))	continue;
					CalcRGB(l);
					if(col[maxnum]>255)
					{
						dist1 = (255-light[maxnum])/maxrate;
						CalcRGB(l);
					}
					for(k=0; k<3; k++)
					{
						if(col[k]>255)	light[k] = 255;
						else			light[k] = col[k];
					}
                }
			}
		}
	}
	mgl.Seti(GAMMA, gamma);
	//RefreshSurfaceCache();
}


void	OmniLight()
{
	omni_light	light;
	sscanf(cargs[1], "%f %f %f", &light.r_rate, &light.g_rate, &light.b_rate);
	light.max_radius = 576;
	VectorCopy(light.pos, entity.origin);
	light.r_rate *= 0x1000;
	light.g_rate *= 0x1000;
	light.b_rate *= 0x1000;

	AddOmniLight(&light);
}
static vec3_t vec3_null = {0,0,0};
bool	RecursiveCheck(int num, float p1f, float p2f, vec3_t p1, vec3_t p2, trace_t *trace)
{
	t_node	*node;
	plane_t	*plane;
	vec3_t	mid;
	float	dist1, dist2, frac, midf;
	int		i, side;
	if (num < 0)	return	true;
	node = &map.nodes[num];
	plane = &map.planes[node->planenum];
	if (plane->type < 3)
	{
		dist1 = p1[plane->type] - plane->dist;
		dist2 = p2[plane->type] - plane->dist;
	}
	else
	{
		dist1 = DotProduct (plane->normal, p1) - plane->dist;
		dist2 = DotProduct (plane->normal, p2) - plane->dist;
	}
	if (dist1 >= 0 && dist2 >= 0)
		return RecursiveCheck (node->children[0], p1f, p2f, p1, p2, trace);
	if (dist1 < 0 && dist2 < 0)
		return RecursiveCheck (node->children[1], p1f, p2f, p1, p2, trace);
	if (dist1 < 0)	frac = (dist1 + DIST_EPSILON)/(dist1-dist2);
	else	frac = (dist1 - DIST_EPSILON)/(dist1-dist2);
	if (frac < 0)	frac = 0;
	if (frac > 1)	frac = 1;

	midf = p1f + (p2f-p1f)*frac;
	for (i=0 ; i<3 ; i++)	mid[i] = p1[i] + frac*(p2[i]-p1[i]);

	side = (dist1 < 0);
	if (!RecursiveCheck (node->children[side], p1f, midf, p1, mid, trace) )
		return	false;
	if (FindSector (node->children[side^1], mid) != 0)
		return	RecursiveCheck (node->children[side^1], midf, p2f, mid, p2, trace);

	if(!side)
	{
		VectorCopy (plane->normal, trace->normal);
		trace->dist = plane->dist;
	}
	else
	{
		VectorSubtract (vec3_null, plane->normal, trace->normal);
		trace->dist = -plane->dist;
	}
	trace->nodenum = num;
	trace->side = side;
	trace->fraction = midf;
	VectorCopy (mid, trace->endpos);

	return false;
}

int		CheckCross(vec3_t A, vec3_t B, int planenum, int side)
{
	trace_t		trace;
	vec3_t		start_l, end_l;
	plane_t		*plane = &map.planes[planenum];

	memset (&trace, 0, sizeof(trace_t));
	trace.fraction = 1;
	trace.allsolid = true;
	VectorCopy(B, start_l);
	VectorCopy(A, end_l);
	if(side)
	{
		end_l[0] -= plane->normal[0]*DIST_EPSILON;
		end_l[1] -= plane->normal[1]*DIST_EPSILON;
		end_l[2] -= plane->normal[2]*DIST_EPSILON;
	} else
	{
		end_l[0] += plane->normal[0]*DIST_EPSILON;
		end_l[1] += plane->normal[1]*DIST_EPSILON;
		end_l[2] += plane->normal[2]*DIST_EPSILON;
	}
	VectorCopy (end_l, trace.endpos);

	RecursiveCheck(map.models[0].headnode[0], 0, 1, start_l, end_l, &trace);
	if((trace.endpos[0]==end_l[0]) && (trace.endpos[1]==end_l[1]) && (trace.endpos[2]==end_l[2]))	return	0;
	float	dist = DotProduct (trace.normal, end_l) - trace.dist;
	if(dist>-8)	return	0;
	return	1;
}
