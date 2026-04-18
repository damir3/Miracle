
#include <stdio.h>

#include "map.h"
#include "math3d.h"
#include "types.h"

float	max_light_radius = 768;
float	max_light_radius2;

byte	ambient_color[3] = {64, 64, 64};
byte	sun_color[3] = {160, 160, 128};
//byte	ambient_color[3] = {128, 128, 128};
//byte	sun_color[3] = {127, 127, 127};

byte	lightmap[256][256][3];

vector	sun_pos = {0, -2000, 40000};
//vector	sun_pos = {0, -200, 40000};

extern float	dist_eps;

int		compute_curves;

int		TraceLine (vector start_, vector end_, trace_t *trace);

void	BlurLightmap (int width, int height)
{
	int		u, v, c, j, n;
	for (v=0; v<height; v++)
	{
		for (u=0; u<width; u++)
		{
			for (j=0; j<3; j++)
			{
				n = 1;
				c = lightmap[v][u][j];
				if (u > 0) c += lightmap[v][u-1][j], n++;
				if (u < width-1) c += lightmap[v][u+1][j], n++;
				if (v > 0)
				{
					c += lightmap[v-1][u][j], n++;
					if (u > 0) c += lightmap[v-1][u-1][j], n++;
					if (u < width-1) c += lightmap[v-1][u+1][j], n++;
				}
				if (v < height-1)
				{
					c += lightmap[v+1][u][j], n++;
					if (u > 0) c += lightmap[v+1][u-1][j], n++;
					if (u < width-1) c += lightmap[v+1][u+1][j], n++;
				}
				lightmap[v][u][j] = c/n;
			}
		}
	}
}

void	ClearLightmap (int width, int height)
{
	int		u, v;
	for (v=0; v<height; v++)
	{
		for (u=0; u<width; u++)
		{
			lightmap[v][u][0] = ambient_color[0];
			lightmap[v][u][1] = ambient_color[1];
			lightmap[v][u][2] = ambient_color[2];
		}
	}
}

void	ComputeFaceLightmap (face_t *face)
{
	int		i, u, v, j, c;
	int		color[3];
	float	dist, t;
	light_t	*light;
	byte	*dst_light;
	vector	cur_vertex, du, dv, begin_vertex, origin;
	int		width, height;
	trace_t	trace;

	compute_curves = FALSE;

	VectorScale (face->lm_vecs[0], 1.0f/256.0f, du);
	VectorScale (face->lm_vecs[1], 1.0f/256.0f, dv);
	width = face->lm_size[0]*2;
	height = face->lm_size[1]*2;

	ClearLightmap (width, height);

	VectorAddScale (face->lm_orig, face->plane.normal, 8.0, origin);
	VectorAddScale (origin, du, (float)face->lm_offset[0]*2, origin);
	VectorAddScale (origin, dv, (float)face->lm_offset[1]*2, origin);

	// adding sun light
	dist_eps = DIST_EPSILON;
	t = PointPlaneDist (sun_pos, &face->plane);
	//t = DotProduct(sun_dir, face->plane.normal);
	if (t > -DIST_EPSILON)
	{
		VectorCopy (origin, begin_vertex);

		for (v=0; v<height; v++)
		{
			VectorCopy (begin_vertex, cur_vertex);
			VectorAdd (begin_vertex, dv, begin_vertex);
			for (u=0; u<width; u++)
			{
				if (TraceLine (sun_pos, cur_vertex, &trace))
				{
					for (j=0; j<3; j++)
						lightmap[v][u][j] += sun_color[j];
				}
				VectorAdd (cur_vertex, du, cur_vertex);
			}
		}
	}

	// adding lights
	dist_eps = 0;
	light = map.lights;
	for (i=0; i<map.numlights; i++, light++)
	{
		dist = PointPlaneDist (light->pos, &face->plane);
		if (dist < 0 || dist > max_light_radius) continue;

		if (PointBBoxfDist2 (light->pos, &face->bbox) > max_light_radius2)
			continue;

		VectorCopy (origin, begin_vertex);

		for (v=0; v<height; v++)
		{
			VectorCopy (begin_vertex, cur_vertex);
			VectorAdd (begin_vertex, dv, begin_vertex);
			for (u=0; u<width; u++)
			{
				dist = PointPointDist2 (light->pos, cur_vertex);

				if ((dist < max_light_radius2)
				&&	TraceLine (light->pos, cur_vertex, &trace))
				{
					dist = 1.0f/dist;
					color[0] = (int)(dist * light->i[0]);
					color[1] = (int)(dist * light->i[1]);
					color[2] = (int)(dist * light->i[2]);

					j = 0;
					if (color[1] > color[j]) j = 1;
					if (color[2] > color[j]) j = 2;

					if (lightmap[v][u][j] + color[j] > 255)
					{
						t = (float)(255 - lightmap[v][u][j])/color[j];
						color[0] = (int)(color[0]*t);
						color[1] = (int)(color[1]*t);
						color[2] = (int)(color[2]*t);
					}

					for (j=0; j<3; j++)
					{
						c = color[j] + lightmap[v][u][j];
						lightmap[v][u][j] = (c>255) ? 255 : c;
					}
				}
				VectorAdd (cur_vertex, du, cur_vertex);
			}
		}
	}

	BlurLightmap (width, height);

	// resize lightmap and put it to lightdata
	dst_light = (byte *)map.lightmapbanks[face->lm_texnum].data[face->lm_offset[1]][face->lm_offset[0]];
	for (v=0; v<height; v+=2)
	{
		for (u=0; u<width; u+=2, dst_light+=3)
		{
			dst_light[0] = (lightmap[v][u][0]+lightmap[v][u+1][0]+lightmap[v+1][u][0]+lightmap[v+1][u+1][0])/4;
			dst_light[1] = (lightmap[v][u][1]+lightmap[v][u+1][1]+lightmap[v+1][u][1]+lightmap[v+1][u+1][1])/4;
			dst_light[2] = (lightmap[v][u][2]+lightmap[v][u+1][2]+lightmap[v+1][u][2]+lightmap[v+1][u+1][2])/4;
		}
		dst_light += (128 - face->lm_size[0])*3;
	}
}

void	ComputeMeshLightmap (face_t *face)
{
	int		i, u, v, j, c;
	int		color[3];
	float	dist, dot;
	float	s, t, ds, dt, f;
	light_t	*light;
	byte	*dst_light;
	vector	vertex, normal, dir;
	int		width, height;
	trace_t	trace;

	compute_curves = TRUE;

	width = face->lm_size[0]*2;
	height = face->lm_size[1]*2;

	ClearLightmap (width, height);

	ds = 0.5f/(face->lm_size[0]-1);
	dt = 0.5f/(face->lm_size[1]-1);

	// adding sun light
	dist_eps = DIST_EPSILON;

	t = -dt;
	for (v=0; v<height; v++)
	{
		s = -ds;
		for (u=0; u<width; u++)
		{
			MeshGetPoint (s, t, face, vertex);
			MeshGetNormal (s, t, face, normal);
			VectorSubtract (sun_pos, vertex, dir);
			VectorNormalize (dir);
			dot = DotProduct (normal, dir);
			if (dot > 0)
			{
				if (TraceLine (sun_pos, vertex, &trace))
				{
					dot += .5f;
					if (dot > 1.0f) dot = 1.0f;
					for (j=0; j<3; j++)
						lightmap[v][u][j] += (int)(dot*sun_color[j]);
				}
			}
			s += ds;
		}
		t += dt;
	}

	// adding lights
	dist_eps = 0;
	for (i=0; i<map.numlights; i++)
	{
        light =  map.lights + i;
		//dist = PointPlaneDist (light->pos, &face->plane);
		//if (dist < 0 || dist > max_light_radius) continue;
		if (PointBBoxfDist2 (light->pos, &face->bbox) > max_light_radius2) continue;

		t = -dt;
		for (v=0; v<height; v++)
		{
			s = -ds;
			for (u=0; u<width; u++)
			{
				MeshGetPoint (s, t, face, vertex);
				MeshGetNormal (s, t, face, normal);
				//VectorAddScale (vertex, normal, 8, vertex);
				VectorSubtract (light->pos, vertex, dir);

				dist = DotProduct (dir, dir);

				if (DotProduct (normal, dir) > 0 &&
				    dist < max_light_radius2 &&
				    TraceLine (light->pos, vertex, &trace))
				{
					dist = 1.0f/dist;
					color[0] = (int)(dist * light->i[0]);
					color[1] = (int)(dist * light->i[1]);
					color[2] = (int)(dist * light->i[2]);

					j = 0;
					if (color[1] > color[j]) j = 1;
					if (color[2] > color[j]) j = 2;

					if (lightmap[v][u][j] + color[j] > 255)
					{
						f = (float)(255 - lightmap[v][u][j])/color[j];
						color[0] = (int)(color[0]*f);
						color[1] = (int)(color[1]*f);
						color[2] = (int)(color[2]*f);
					}

					for (j=0; j<3; j++)
					{
						c = color[j] + lightmap[v][u][j];
						lightmap[v][u][j] = (c>255) ? 255 : c;
					}
				}
				s += ds;
			}
			t += dt;
		}
	}

	BlurLightmap (width, height);

	// resize lightmap and put it to lightdata
	dst_light = (byte *)map.lightmapbanks[face->lm_texnum].data[face->lm_offset[1]][face->lm_offset[0]];
	for (v=0; v<height; v+=2)
	{
		for (u=0; u<width; u+=2, dst_light+=3)
		{
			dst_light[0] = (lightmap[v][u][0]+lightmap[v][u+1][0]+lightmap[v+1][u][0]+lightmap[v+1][u+1][0])/4;
			dst_light[1] = (lightmap[v][u][1]+lightmap[v][u+1][1]+lightmap[v+1][u][1]+lightmap[v+1][u+1][1])/4;
			dst_light[2] = (lightmap[v][u][2]+lightmap[v][u+1][2]+lightmap[v+1][u][2]+lightmap[v+1][u+1][2])/4;
		}
		dst_light += (128 - face->lm_size[0])*3;
	}
}

void	ComputeLightData ()
{
	int		i, nummeshes, numfaces;

	max_light_radius2 = max_light_radius * max_light_radius;

	for (nummeshes=0; nummeshes<map.numfaces; nummeshes++)
		if (map.faces[nummeshes].facetype != FACETYPE_MESH) break;

	if (nummeshes)
	{
		InitProgress ("Compute curves lighting");
		for (i=0; i<nummeshes; i++)
		{
			if (map.faces[i].lm_texnum < 0) continue;
			switch (map.faces[i].facetype)
			{
			case FACETYPE_NORMAL:
				ComputeFaceLightmap (map.faces + i);
				break;
			case FACETYPE_MESH:
				ComputeMeshLightmap (map.faces + i);
				break;
			}
			UpdateProgress ((float)i/(nummeshes-1));
		}
        ComputeMeshLightmap (map.faces + 40);
	}

	InitProgress ("Compute lighting");
	numfaces = map.numfaces - nummeshes;
	for (i=nummeshes; i<map.numfaces; i++)
	{
		if (map.faces[i].lm_texnum < 0) continue;
		switch (map.faces[i].facetype)
		{
		case FACETYPE_NORMAL:
			//printf ("\r%dn", i);
			ComputeFaceLightmap (map.faces + i);
			break;
		case FACETYPE_MESH:
			//printf ("\r%dm", i);
			ComputeMeshLightmap (map.faces + i);
			break;
		}
		UpdateProgress ((float)i/(numfaces-1));
		//printf ("\rface[%d]", i);
	}
}
