#ifndef _DRAW3D_H
#define _DRAW3D_H

#include	"MapFile.h"

typedef struct
{
	void	*ent;
	float	z;
	int		type;
} vis_ent_t;

typedef struct
{
	vec3_t	pos, transform_pos;
	float	scale;
	bitmap	*bitmap;
	short	mark;
	byte	alpha, type;
} sprite_t;

extern int	numsprites, cur_mark_num;

void	Transform3DBitmapsPoints();
void	FindAndDrawEnts(t_plane *plane);
void	DrawOtherEnts();
void	DrawLightFlare(t_light *light, float scale);

void	DrawSprite(vec3_t pos, bitmap *bitmap, float scale, byte alpha, byte type);

#endif