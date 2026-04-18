/*
	Draw_3D.cpp		Copyright (C) 1998-1999 Damir Sagidullin
*/
#include	"Const.h"
//#include	"MapFile.h"
#include	"DrawModel.h"
#include	"Draw_3D.h"
#include	"DrawFace.h"
#include	"DrawSpan.h"
#include	"Math_3D.h"
#include	"SpanBuf.h"
#include	"Render.h"
#include	"Video.h"

#define		MIN_VISIBLE_DIST		1.0f
#define		MAX_NUM_VIS_ENTITIES	256
#define		MAX_NUM_SPRITES			256

int			cur_mark_num, num_ents, numsprites=0;
vis_ent_t	vis_ents[MAX_NUM_VIS_ENTITIES];
sprite_t	sprites[MAX_NUM_SPRITES];

extern int		sx_min, sx_max, sy_min, sy_max;
extern ushort	r_color_map[64][256];
extern ushort	g_color_map[64][256];
extern ushort	b_color_map[32][256];
extern ushort	transparency_map[16][0x10000];
extern int		scr_face_edge[MAX_SY_SIZE][2];
extern double	proj_scale_x, proj_scale_y;
extern int		mirror;

static point_3d	sprite_vertices[4];
static point_3d	*sprite_vertices_list[4];
static float	f_du, f_dv, f_dz;
static int		sy_start, sy_end, global_sx_start, global_sy_start, du;

int		ComputeEdges(sprite_t *sprite)
{
	int		i, j, n=4, codes_or = 0, codes_and = 0xff;
	if(sprite->transform_pos[2]<MIN_VISIBLE_DIST)	return	0;
	float	scale[2];
	scale[0] = float(proj_scale_x*bitmap_scale[0]*sprite->scale);
	scale[1] = float(proj_scale_y*bitmap_scale[1]*sprite->scale);
	float	width = sprite->bitmap->width*scale[0]*0.5f;
	float	height = sprite->bitmap->height*scale[1]*0.5f;
	sprite_vertices[0].p[0] = sprite->transform_pos[0] - width;
	sprite_vertices[1].p[0] = sprite->transform_pos[0] - width;
	sprite_vertices[2].p[0] = sprite->transform_pos[0] + width;
	sprite_vertices[3].p[0] = sprite->transform_pos[0] + width;
	sprite_vertices[0].p[1] = sprite->transform_pos[1] - height;
	sprite_vertices[1].p[1] = sprite->transform_pos[1] + height;
	sprite_vertices[2].p[1] = sprite->transform_pos[1] + height;
	sprite_vertices[3].p[1] = sprite->transform_pos[1] - height;
	for(i=0; i<n; i++)
	{
		sprite_vertices[i].p[2] = sprite->transform_pos[2];
		sprite_vertices_list[i] = &sprite_vertices[i];
		ProjectPointToScreen(sprite_vertices_list[i]);
		CodePoint(sprite_vertices_list[i]);
		codes_or |= sprite_vertices[i].ccodes;
		codes_and &= sprite_vertices[i].ccodes;
	}
	if(codes_and)	return	0;

	float	dist;
	if(sprite->bitmap->width > sprite->bitmap->height)
		dist = sprite->bitmap->width*1.4f;
	else
		dist = sprite->bitmap->height*1.4f;
	if(!VisibleCheck(sprite->pos, dist))	return	0;

	point_3d	**vlist;
	clip_mode = 0;
	n = ClipPoly(n, sprite_vertices_list, codes_or, &vlist);
	if(!n)	return	0;
	int		ymin, ymax;
	ymin = ymax = vlist[0]->sy;
	for (i=1; i<n; i++)
	{
		if (vlist[i]->sy < ymin) ymin = vlist[i]->sy;
		else if (vlist[i]->sy > ymax) ymax = vlist[i]->sy;
	}
	j = n-1;
	for (i=0; i<n; i++)
	{
		ComputeEdge(vlist[i], vlist[j]);
		j = i;
	}
	f_dz = 65536.0f/sprite->transform_pos[2];
	f_du = sprite->transform_pos[2]/scale[0];
	f_dv = sprite->transform_pos[2]/scale[1];
	du = int(f_du*0x10000);
	sy_start = fix_to_int(ymin);
	sy_end = fix_to_int(ymax);
	global_sx_start = sprite_vertices[1].sx;
	global_sy_start = sprite_vertices[1].sy;
	return	1;
}
void	DrawSprite_(sprite_t *sprite)
{
	if(!ComputeEdges(sprite))	return;
	switch(sprite->type)
	{
	case	0:
		{
			ushort	*map1, *map2;
			if(sprite->alpha<15)
			{
				map1 = transparency_map[sprite->alpha];
				map2 = transparency_map[15-sprite->alpha];
			}
			for(; sy_start<sy_end; sy_start++)
			{
				int	sx_start = fix_to_int(scr_face_edge[sy_start][0]);
				int	sx_end = fix_to_int(scr_face_edge[sy_start][1]);
				if(sx_start>=sx_end)	continue;
				TSpan	cur_span;
				cur_span.sx_start = sx_start;
				cur_span.sx_end = sx_end;
				cur_span.sx0 = sx_start;
				cur_span.dz = f_dz;
				cur_span.ddz = 0;

				num_draw_spans = 0;
				if(!ClipSpan(sy_start, &cur_span))	continue;
				int		v = float_to_int(((sy_start<<16) - global_sy_start)*f_dv);
				ushort	*out = (ushort *)sprite->bitmap->bm + (v>>16)*sprite->bitmap->width;

				for(int i=0, j=0; i<num_draw_spans; i++)
				{
					sx_start = spans_start[i];
					int		dsx = spans_end[i] - sx_start;
					if(dsx<=0)	continue;
					int		u = float_to_int(((sx_start<<16) - global_sx_start)*f_du);
					ushort	*dest = (ushort *)virtual_screen + screen_row_table[sy_start] + sx_start;
					if(sprite->alpha<15)
					{
						while(dsx--)
						{
							*dest++ = map1[out[u>>16]] + map2[*dest];
							u += du;
						}
					} else
					{
						while(dsx--)
						{
							*dest++ = out[u>>16];
							u += du;
						}
					}
					j++;
				}
				if(j)	AddSpan(sy_start, &cur_span);
			}
		}
		break;
	case	1:
		{
			ushort	*map1, *map2;
			if(sprite->alpha<15)
			{
				map1 = transparency_map[sprite->alpha];
				map2 = transparency_map[15-sprite->alpha];
			}
			for(; sy_start<sy_end; sy_start++)
			{
				int	sx_start = fix_to_int(scr_face_edge[sy_start][0]);
				int	sx_end = fix_to_int(scr_face_edge[sy_start][1]);
				if(sx_start>=sx_end)	continue;
				TSpan	cur_span;
				cur_span.sx_start = sx_start;
				cur_span.sx_end = sx_end;
				cur_span.sx0 = sx_start;
				cur_span.dz = f_dz;
				cur_span.ddz = 0;

				num_draw_spans = 0;
				if(!ClipSpan(sy_start, &cur_span))	continue;
				int		v = float_to_int(((sy_start<<16) - global_sy_start)*f_dv);
				ushort	*out = (ushort *)sprite->bitmap->bm + (v>>16)*sprite->bitmap->width;

				for(int i=0, j=0; i<num_draw_spans; i++)
				{
					sx_start = spans_start[i];
					int		dsx = spans_end[i] - sx_start;
					if(dsx<=0)	continue;
					int		u = float_to_int(((sx_start<<16) - global_sx_start)*f_du);
					ushort	*dest = (ushort *)virtual_screen + screen_row_table[sy_start] + sx_start;
					if(sprite->alpha<15)
					{
						while(dsx--)
						{
							ushort	c = out[u>>16];
							if(c)	*dest++ = map1[c] + map2[*dest];
							else	dest++;
							u += du;
						}
					} else
					{
						while(dsx--)
						{
							ushort	c = out[u>>16];
							if(c)	*dest++ = c;
							else	dest++;
							u += du;
						}
					}
					j++;
				}
				//if(j)	AddSpan(sy_start, &cur_span);
			}
		}
		break;
	case	2:
		{
			for(; sy_start<sy_end; sy_start++)
			{
				int	sx_start = fix_to_int(scr_face_edge[sy_start][0]);
				int	sx_end = fix_to_int(scr_face_edge[sy_start][1]);
				if(sx_start>=sx_end)	continue;
				TSpan	cur_span;
				cur_span.sx_start = sx_start;
				cur_span.sx_end = sx_end;
				cur_span.sx0 = sx_start;
				cur_span.dz = f_dz;
				cur_span.ddz = 0;

				num_draw_spans = 0;
				if(!ClipSpan(sy_start, &cur_span))	continue;
				int		v = float_to_int(((sy_start<<16) - global_sy_start)*f_dv);
				byte	*out = (byte *)sprite->bitmap->bm + (v>>16)*sprite->bitmap->width*3;

				for(int i=0, j=0; i<num_draw_spans; i++)
				{
					sx_start = spans_start[i];
					int		dsx = spans_end[i] - sx_start;
					if(dsx<=0)	continue;
					int		u = float_to_int(((sx_start<<16) - global_sx_start)*f_du);
					ushort	*dest = (ushort *)virtual_screen + screen_row_table[sy_start] + sx_start;
					while(dsx--)
					{
						ushort	c = *dest;
						byte	r = c>>10;
						byte	g = (c>>5)&63;
						byte	b = c&31;
						int	u_ = u>>16;
						u_ += u_<<1;
						*dest++ = r_color_map[r][out[u_]]+
								g_color_map[g][out[u_+1]]+
								b_color_map[b][out[u_+2]];
						u += du;
					}
					j++;
				}
				//if(j)	AddSpan(sy_start, &cur_span);
			}
		}
		break;
	}
	
}

void	DrawSprite(vec3_t pos, bitmap *bitmap, float scale, byte alpha, byte type)
{
	alpha >>= 4;
	if(!alpha)	return;
	if(numsprites>=MAX_NUM_SPRITES)	return;
	VectorCopy(pos, sprites[numsprites].pos);
	sprites[numsprites].bitmap = bitmap;
	sprites[numsprites].scale = scale;
	sprites[numsprites].alpha = alpha;
	sprites[numsprites].type = type;
	sprites[numsprites].mark = 0;
	numsprites++;
}

void	Transform3DBitmapsPoints()
{
	//	преобразуем координаты
	cur_mark_num ++;
	for(int i=0; i<numsprites; i++)
	{
		TransformPointRaw(sprites[i].transform_pos, sprites[i].pos);
		if(sprites[i].transform_pos[2]<MIN_VISIBLE_DIST)
			sprites[i].mark=cur_mark_num;
	}
}
void	DrawEnts()
{
	int		i = num_ents, j, max_num;
	//	сортируем спрайты и объекты
	while(i--)
	{
		max_num = j = i;
		float	max_z = vis_ents[max_num].z;
		while(j--)
		{
			if(max_z<vis_ents[j].z)
			{
				max_z = vis_ents[j].z;
				max_num = j;
			}
		}
		switch(vis_ents[max_num].type)
		{
		case	1:
			//	рисуем спрайт
			DrawSprite_((sprite_t *) vis_ents[max_num].ent);
			break;
		case	2:
			//	рисуем объект
			break;
		}
		if (max_num!=i)	vis_ents[max_num] = vis_ents[i];
	}
}
void	DrawOtherEnts()
{
	num_ents=0;
	//	находим все спрайты, которые находятся за плоскостью
	for(int i=0; i<numsprites; i++)
	{
		if(sprites[i].mark<cur_mark_num)
		{
			vis_ents[num_ents].ent = (void *) &sprites[i];
			vis_ents[num_ents].z = sprites[i].transform_pos[2];
			vis_ents[num_ents].type = 1;	//	спрайт
			sprites[i].mark = cur_mark_num;
			num_ents++;
		}
	}
	DrawEnts();
}
void	FindAndDrawEnts(t_plane *plane)
{
	num_ents=0;
	//	находим все спрайты, которые находятся перед плоскостью
	for(int i=0; i<numsprites; i++)
	{
		if(sprites[i].mark<cur_mark_num)
		if(PointPlaneDist(sprites[i].pos, plane)<0)
		{
			vis_ents[num_ents].ent = (void *) &sprites[i];
			vis_ents[num_ents].z = sprites[i].transform_pos[2];
			vis_ents[num_ents].type = 1;	//	спрайт
			sprites[i].mark = cur_mark_num;
			num_ents++;
		}
	}
	DrawEnts();
}
//	-------------- DrawLightFlare --------------
void	DrawLightFlare(t_light *light, float scale)
{
	if(!light->volume_light)	return;
	if(!(vis_sectors[light->sector>>3]&(1<<(light->sector&7))))	return;
	point_3d	transform_point;
	TransformPointRaw(transform_point.p, light->pos);
	if(transform_point.p[2]<1)	return;
	ProjectPointToScreen(&transform_point);
	int		sx = transform_point.sx>>16;
	transform_point.sy >>= 16;
	if((sx<=sx_min) || (sx>=sx_max))	return;
	if((transform_point.sy<=sy_min) || (transform_point.sy>=sy_max))	return;

	byte	*bitmap = map.textures[light->volume_light].offsets[0];
	int		bm_width = map.textures[light->volume_light].width;
	int		bm_height = map.textures[light->volume_light].height;

	f_dz = 1.0f/transform_point.p[2];
	if(transform_point.p[2]<bm_width)	scale *= bm_width*f_dz;
	if(transform_point.p[2]>512)	scale *= 512*f_dz;

	scale *= transform_point.p[2]/256;

	float	width = float(bm_width*scale*0.5*proj_scale_x);
	float	height = float(bm_height*scale*0.5*proj_scale_y);
	sprite_vertices[0].p[0] = transform_point.p[0] - width;
	sprite_vertices[1].p[0] = transform_point.p[0] - width;
	sprite_vertices[2].p[0] = transform_point.p[0] + width;
	sprite_vertices[3].p[0] = transform_point.p[0] + width;
	sprite_vertices[0].p[1] = transform_point.p[1] - height;
	sprite_vertices[1].p[1] = transform_point.p[1] + height;
	sprite_vertices[2].p[1] = transform_point.p[1] + height;
	sprite_vertices[3].p[1] = transform_point.p[1] - height;
	int		i=4, codes_and=0xff;
	while(i--)
	{
		sprite_vertices[i].p[2] = transform_point.p[2];
		ProjectPointToScreen(&sprite_vertices[i]);
		CodePoint(&sprite_vertices[i]);
		codes_and &= sprite_vertices[i].ccodes;
	}
	if(codes_and)	return;
	if(mirror)
	{
		transform_point.sx = (sx_size<<16)-65535-transform_point.sx;
		int		sx1 = sprite_vertices[2].sx;
		int		sx2 = sprite_vertices[1].sx;
		sprite_vertices[1].sx = (sx_size<<16)-65535-sx1;
		sprite_vertices[2].sx = (sx_size<<16)-65535-sx2;
	}

	if(!ClipPixel(transform_point.sx, transform_point.sy, 65536.0f*f_dz))	return;

	f_du = float(transform_point.p[2]/(proj_scale_x*scale));
	f_dv = float(transform_point.p[2]/(proj_scale_y*scale));
	du = int(f_du*0x10000);
	global_sx_start = sprite_vertices[1].sx;
	global_sy_start = sprite_vertices[1].sy;
	int		sy_start = fix_to_int(sprite_vertices[1].sy);
	int		sy_end = fix_to_int(sprite_vertices[0].sy);
	int		sx_start = fix_to_int(sprite_vertices[1].sx);
	int		sx_end = fix_to_int(sprite_vertices[2].sx);
	if(sy_start<sy_min)	sy_start=sy_min;
	if(sy_end>sy_max)	sy_end=sy_max;
	if(sx_start<sx_min)	sx_start=sx_min;
	if(sx_end>sx_max)	sx_end=sx_max;
	int		dsx = sx_end- sx_start;
	if(dsx<=0)	return;

	for(; sy_start<sy_end; sy_start++)
	{
		int		v = float_to_int(((sy_start<<16) - global_sy_start)*f_dv);
		int		u = float_to_int(((sx_start<<16) - global_sx_start)*f_du);

		byte	*out = bitmap + (v>>16)*bm_width*3;
		ushort	*dest = (ushort *)virtual_screen + screen_row_table[sy_start] + sx_start;
		i = dsx;
		while(i--)
		{
			ushort	c = *dest;
			byte	r = c>>10;
			byte	g = (c>>5)&63;
			byte	b = c&31;
			int	u_ = u>>16;
			u_ += u_<<1;
			*dest++ = r_color_map[r][out[u_]]+
					g_color_map[g][out[u_+1]]+
					b_color_map[b][out[u_+2]];
			u += du;
		}
	}
}