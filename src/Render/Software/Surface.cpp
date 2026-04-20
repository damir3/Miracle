/*
	Surface.cpp		Copyright (C) 1998-1999 Damir Sagidullin
*/
#include	<Stdio.h>	//	sprintf
#include	<Stdlib.h>
#include	<Memory.h>
#include	<Math.h>

#include	"MapFile.h"
#include	"Math_3D.h"
#include	"Surface.h"
#include	"TexGrad.h"
#include	"Video.h"

#define	MAX_CACHE_SIZE	(1024*1024*4)

//	BSP map
extern TMap		map;
//	color tables (needed to create lighting)
extern ushort	red_color_map[256][256];
extern ushort	green_color_map[256][256];
extern ushort	blue_color_map[256][256];
extern int		gamma;
//	static texture cache
static char	*cache=NULL;
static int	*surface_cache[MIPLEVELS];
static int	cache_begin, cache_end, cur_cache;
//	used for dynamically lit textures
byte		blank_light[2024];
//	polygon color (if rendered without texture)
byte		stat_r, stat_g, stat_b;
//	global variables
int			shift, global_step, global_row, lightmap_width;
t_lightdata	*cur_light;
//	For dynamic lighting
int				num_dlights=0;
dynamic_light	*dlights[MAX_NUM_DYNAMIC_LIGHTS];
ushort		dynamic_texture[0x10000 + ((sizeof(surface_t) + sizeof(texmap))>>1)];
//	Static texture cache processing functions
void	FreeNextSurface ()
{	//	frees the earliest texture from cache
	surface_t	*surf = (surface_t *)(cache + cache_begin);
	surface_cache[surf->mip_level][surf->face] = -1;
	cache_begin = surf->next_cache;
}

int		CacheAllocate (int nes_size)
{	//	allocates necessary cache for texture
	nes_size += sizeof(surface_t) + sizeof(texmap);
	surface_t	*surf = (surface_t *)(cache + cur_cache);
	int		size;
	do
	{
		if (cache_end>cache_begin)
		{
			size = MAX_CACHE_SIZE - cache_end;
			if (size<nes_size)
			{
				while (cache_begin<nes_size)
				{
					FreeNextSurface ();
				}
				cur_cache = 0;
				surf->next_cache = 0;
				cache_end = nes_size;
				return	0;
			}
		} else
		{
			size = cache_begin - cache_end;
			if (size<nes_size)
			{
				FreeNextSurface ();
				size = cache_begin - cache_end;
			}
		}
	}	while (size<nes_size);
	cur_cache = cache_end;
	surf->next_cache = cur_cache;
	cache_end += nes_size;
	return	cur_cache;
}
void	RemakeCache ()
{	//	creates new cache for new map
	for (int j=0; j<MIPLEVELS; j++)
	{
		surface_cache[j] = (int *)realloc (surface_cache[j], map.numfaces*sizeof(int));
		memset (surface_cache[j], -1, map.numfaces*sizeof(int));
	}
	cache_begin = 0;
	cur_cache = 0;
	cache_end = sizeof(surface_t);
	surface_t	*surf = (surface_t *)cache;
	surf->face = 0;
}

void	RefreshSurfaceCache ()
{	//	completely clears cache
	if (map.numfaces)
	{
		for (int j=0; j<MIPLEVELS; j++)
			memset (surface_cache[j], -1, map.numfaces*sizeof(int));
	}
	cache_begin = 0;
	cur_cache = 0;
	cache_end = sizeof(surface_t);
	surface_t	*surf = (surface_t *)cache;
	surf->face = 0;
}

void	InitCache ()
{	//	allocates memory for cache
	cache = (char *)malloc (MAX_CACHE_SIZE);
	for (int j=0; j<MIPLEVELS; j++)	surface_cache[j] = NULL;
	map.numfaces = 0;
}
void	BuildSurfaceBlock (ushort *out, bitmap *bm, int x, int y)
{
	int		u, v;
	int		row = global_row - global_step;
	int		width = (bm->width<<1) + bm->width;
	byte	*texture = (byte *)bm->bm + y*width + (x<<1) + x;
	width -= (global_step<<1) + global_step;

	if (draw_mode&DRAW_LIGHT)
	{
		int		r, dr, r0, r1, r2, r3;
		int		g, dg, g0, g1, g2, g3;
		int		b, db, b0, b1, b2, b3;
		r0 = cur_light[0].r<<14;
		r1 = cur_light[1].r<<14;
		r2 = cur_light[lightmap_width].r<<14;
		r3 = cur_light[lightmap_width+1].r<<14;
		r2 = (r2 - r0) >> shift;
		r3 = (r3 - r1) >> shift;

		g0 = cur_light[0].g<<14;
		g1 = cur_light[1].g<<14;
		g2 = cur_light[lightmap_width].g<<14;
		g3 = cur_light[lightmap_width+1].g<<14;
		g2 = (g2 - g0) >> shift;
		g3 = (g3 - g1) >> shift;

		b0 = cur_light[0].b<<14;
		b1 = cur_light[1].b<<14;
		b2 = cur_light[lightmap_width].b<<14;
		b3 = cur_light[lightmap_width+1].b<<14;
		b2 = (b2 - b0) >> shift;
		b3 = (b3 - b1) >> shift;

		v = global_step;
		if (draw_mode&DRAW_TEXTURED)
		{	//	if colored texture
			while ((v--)>0)
			{
				r = r0;
				dr = (r1 - r0) >> shift;
				g = g0;
				dg = (g1 - g0) >> shift;
				b = b0;
				db = (b1 - b0) >> shift;
				u = global_step;
				/*_asm
				{
					xor	eax, eax
					nop
					nop
					nop
					xor	eax, eax
				}*/
				while (u--)
				{
					*out++=	red_color_map[r>>16][texture[0]] +
							green_color_map[g>>16][texture[1]] +
							blue_color_map[b>>16][texture[2]];
					texture += 3;
					r += dr;
					g += dg;
					b += db;
				}
				/*_asm
				{
					xor	ebx, ebx
					nop
					nop
					nop
					xor	ebx, ebx
				}*/
				out += row;
				texture += width;
				r0 += r2;
				r1 += r3;
				g0 += g2;
				g1 += g3;
				b0 += b2;
				b1 += b3;
			}
		} else
		{	//	if uncolored texture
			while (v--)
			{
				r = r0;
				dr = (r1 - r0) >> shift;
				g = g0;
				dg = (g1 - g0) >> shift;
				b = b0;
				db = (b1 - b0) >> shift;
				u = global_step;
				while (u--)
				{
					*out++ = red_color_map[r>>16][stat_r] +
						green_color_map[g>>16][stat_g] +
						blue_color_map[b>>16][stat_b];
					texture += 3;
					r += dr;
					g += dg;b += db;
				}
				out += row;
				texture += width;
				r0 += r2;
				r1 += r3;
				g0 += g2;
				g1 += g3;
				b0 += b2;
				b1 += b3;
			}
		}
	} else
	{	//	if unlit colored texture
		v = global_step;
		while (v--)
		{
			u = global_step;
			while (u--)
			{
				*out++ = red_color_map[32][texture[0]] +
					green_color_map[32][texture[1]] +
					blue_color_map[32][texture[2]];
				texture += 3;
			}
			out += row;
			texture += width;
		}
	}
}
float	fog_planes_cam_dist[MAX_NUM_FOG_PLANES];
void	SetFog (fog_t *fog)
{
	for (int i=0; i<fog->numplanes; i++)
		fog_planes_cam_dist[i] = PointPlaneDist (cam_pos, &fog->planes[i]);
}
float	ClipFogDist (fog_t *fog, vec3_t v)
{
	int		i, f=1;
	float	d1, d2, where;
	vec3_t	cpos, ppos;

	VectorCopy (cam_pos, cpos);
	VectorCopy (v, ppos);
	for (i=0; i<fog->numplanes; i++)
	{
		if (!f)	d1 = fog_planes_cam_dist[i];
		else	d1 = PointPlaneDist (cpos, &fog->planes[i]);
		d2 = PointPlaneDist (ppos, &fog->planes[i]);
		if (d1>=0)
		{
			if (d2>=0)	return	1.0f;
			where = d1 / (d1 - d2);
			cpos[0] += (ppos[0] - cpos[0]) * where;
			cpos[1] += (ppos[1] - cpos[1]) * where;
			cpos[2] += (ppos[2] - cpos[2]) * where;
			f = 0;
		} else
		{
			if (d2>=0)
			{
				where = d2 / (d2 - d1);
				ppos[0] += (cpos[0] - ppos[0]) * where;
				ppos[1] += (cpos[1] - ppos[1]) * where;
				ppos[2] += (cpos[2] - ppos[2]) * where;
			}
		}
	}
	return	float(sqrt(PointPointDist2 (cpos, ppos)));
}
void	GetTextureMap (texmap *texture, int face, int texnum, float *u, float *v)
{
	int		i,j, u0,v0,u1,v1, step, x,y,x0, num_dynamic_ceils=0;
	int		width, height, lightmapsize;
	bitmap	raw;
	surface_t	*surf;
	//	find desired texture
	if ((face_type==2) || (face_type==3))
	{//	water or sky
		raw.bm	= (char *)map.textures[texnum].offsets[0];
		raw.width	= map.textures[texnum].width;
		raw.height	= map.textures[texnum].height;
	} else
	{
		raw.bm	= (char *)map.textures[texnum].offsets[minmiplevel];
		raw.width	= map.textures[texnum].width >> minmiplevel;
		raw.height	= map.textures[texnum].height >> minmiplevel;
	}
	if (!(draw_mode&DRAW_TEXTURED))
	{
		stat_r = map.textures[texnum].r;
		stat_g = map.textures[texnum].g;
		stat_b = map.textures[texnum].b;
		if (!(draw_mode&DRAW_LIGHT))
		{	//	if rendering an uncolored unlit polygon
			face_color = red_color_map[map.faces_consts[face].r>>2][stat_r] +
						green_color_map[map.faces_consts[face].g>>2][stat_g] +
						blue_color_map[map.faces_consts[face].b>>2][stat_b];
			*u = 0;
			*v = 0;
			texture->height = raw.height;
			texture->width = raw.width;
			texture->bitmap = (short *)(raw.bm);
			return;
		}
	}

	if ((face_type==2) || (face_type==3))
	{	//	if water, lava or clouds
		*u = 0;
		*v = 0;
		texture->height = raw.height;
		texture->width = raw.width;
		texture->bitmap = (short *)(raw.bm);
		return;
	}
	//	get texture edges of polygon
	face_consts	*consts = &map.faces_consts[face];
	u0 = consts->u0;
	v0 = consts->v0;
	u1 = consts->u1;
	v1 = consts->v1;
	lightmapsize = (((u1-u0)>>4)+1)*(((v1-v0)>>4)+1)*3;

	if (num_dlights)
	{	//	if there are dynamic light sources
		t_plane *plane = &map.planes[map.faces[face].planenum];
		float	a[3], b[3], c[3], d[3];
		float	temp, dist;
		float	*vecs_u, *vecs_v;
		vec3_t	cur_vertex, begin_v, du, dv;
		VectorScale (consts->u, 16, du);
		VectorScale (consts->v, 16, dv);
		width =	(u1-u0)>>4;
		height = (v1-v0)>>4;
		cur_light = (t_lightdata *)blank_light;
		if (map.faces[face].lightofs != -1)
		{
			memcpy (blank_light, map.lightmaps + map.faces[face].lightofs, lightmapsize);
			int		num = lightmapsize;
			while (num-->0)
			{
				int		c = blank_light[num] + gamma;
				if (c<0)		c=0;
				if (c>255)	c=255;
				blank_light[num] = c;
			}
		} else
		{
			int		g = gamma;
			if (g<0)		g=0;
			if (g>255)	g=255;
			memset (blank_light, g, lightmapsize);
		}
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
        temp = 1.0f/det (a,b,c);
		begin_v[0] = det (d,b,c) * temp;
		begin_v[1] = det (a,d,c) * temp;
		begin_v[2] = det (a,b,d) * temp;
		SetFog (&map.fogs[0]);
		for (int	vn=0; vn<=height; vn++)
		{
			VectorCopy (begin_v, cur_vertex);
			VectorAdd (begin_v, dv, begin_v);
			for (int	un=0; un<=width; un++)
			{
				for (int dl_num=0; dl_num<num_dlights; dl_num++)
				{
					dist = PointPointDist2 (cur_vertex, dlights[dl_num]->pos);
					if (dist<dlights[dl_num]->max_radius2)
					{
						int		r, g, b;
					//if (dist<dlights[dl_num]->max_radius2*1.5f)
					//{
						temp = 1.0f/dist;
						r = int (dlights[dl_num]->i[0]*temp) + cur_light->r;
						g = int (dlights[dl_num]->i[1]*temp) + cur_light->g;
						b = int (dlights[dl_num]->i[2]*temp) + cur_light->b;
						if (r>=255)	cur_light->r = 255;
						else		cur_light->r = r;
						if (g>=255)	cur_light->g = 255;
						else		cur_light->g = g;
						if (b>=255)	cur_light->b = 255;
						else		cur_light->b = b;
						num_dynamic_ceils++;
					}
				}
				/*float	fogk = ClipFogDist (&map.fogs[0], cur_vertex)*map.fogs[0].density;
				if (fogk)
				{
					int		r, g, b;
					r = int (map.fogs[0].color[0]*fogk) + cur_light->r;
					g = int (map.fogs[0].color[1]*fogk) + cur_light->g;
					b = int (map.fogs[0].color[2]*fogk) + cur_light->b;
					if (r>=255)	cur_light->r = 255;
					else		cur_light->r = r;
					if (g>=255)	cur_light->g = 255;
					else		cur_light->g = g;
					if (b>=255)	cur_light->b = 255;
					else		cur_light->b = b;
					num_dynamic_ceils++;
				}*/
				cur_light ++;
				VectorAdd (cur_vertex, du, cur_vertex);
			}
		}
	}
	cur_light = (t_lightdata *)blank_light;
	width = texture->width = (u1-u0) >> minmiplevel;
	height = texture->height = (v1-v0) >> minmiplevel;
	if (!num_dynamic_ceils)
	{	//	if texture is not lit by dynamic light
		int surf_ofs = surface_cache[minmiplevel][face];
		if (surf_ofs>0)
		{
			surf = (surface_t *)(cache + surf_ofs);
			*texture = *surf->bm;
			*u = surf->u;
			*v = surf->v;
			return;
		}
		if (map.faces[face].lightofs != -1)
		{
			memcpy (blank_light, map.lightmaps + map.faces[face].lightofs, lightmapsize);
			int		num = lightmapsize;
			while (num-->0)
			{
				int		c = blank_light[num] + gamma;
				if (c<0)	c=0;
				if (c>255)	c=255;
				blank_light[num] = c;
			}
		} else
		{
			int		g = gamma;
			if (g<0)	g=0;
			if (g>255)	g=255;
			memset (blank_light, g, lightmapsize);
		}
		surface_cache[minmiplevel][face] = CacheAllocate ((width * height)<<1);
		char *cache_offset = cache + surface_cache[minmiplevel][face];
		texture->bitmap = (short *)(cache_offset + sizeof(surface_t) + sizeof(texmap));
		surf = (surface_t *) cache_offset;
		surf->bm = (texmap *)(cache_offset + sizeof(surface_t));
	} else
	{	//	if texture is lit by dynamic light
		texture->bitmap = (short *)(dynamic_texture + sizeof(surface_t) + sizeof(texmap));
		surf = (surface_t *) dynamic_texture;
		surf->bm = (texmap *)(dynamic_texture + sizeof(surface_t));
	}

	*surf->bm = *texture;
	surf->face = face;
	surf->mip_level = minmiplevel;
	*u = surf->u = float(u0);
	*v = surf->v = float(v0);
	
	step = 16 >> minmiplevel;
	shift = 4 - minmiplevel;

	lightmap_width = ((u1 - u0) >> 4)+1;

	u0 >>= minmiplevel;
	v0 >>= minmiplevel;

	global_step = step;
	global_row  = width;

	y  = v0%raw.height;
    if (y<0)	y += raw.height;
	x0 = u0%raw.width;
    if (x0<0)	x0 += raw.width;

	for (j=0; j<height; j += step)
	{
		x = x0;
		for (i=0; i<width; i += step)
		{
			BuildSurfaceBlock((ushort *)texture->bitmap + (j*width + i), &raw, x,y);
			x += step;
			cur_light++;
			if (x >= raw.width)	x -= raw.width;
		}
		cur_light++;
		y += step;
        if (y >= raw.height)	y -= raw.height;
	}
}
