#ifndef _STRUCT_H
#define _STRUCT_H

typedef unsigned char	byte;
typedef unsigned short	ushort;
typedef unsigned int	uint;

typedef float vec_t;
typedef vec_t vec3_t[3];

typedef struct
{
	int		width, height;
	byte	pal[768];
	byte	*bm;
} bitmap8;

typedef struct
{
	byte	r, g, b, bpp;
	int		width, height;
	char	*bm;
} bitmap;

typedef struct
{
	int		width;
	int		height;
	int		bpp;
} vm;

typedef struct
{
	float	fraction;
	vec3_t	endpos;
	int		nodenum;
	char	side;
	char	allsolid, startsolid, contents;
	vec3_t	normal;
	float	dist;
} trace_t;

typedef struct
{
	vec3_t	origin;
	vec3_t	angles;
	vec3_t	matrix[3];
	int		sector;
} camera_t;

typedef struct
{
	enum	status { alive, dying, died };
	vec3_t	old_origin, origin, angles, velocity;
	vec3_t	mins, maxs;
	vec3_t	move_mins, move_maxs;
	camera_t	*camera;
	//model_t		*model;
	int		modelframe, flags;
	float	delta_z, old_anim_time, anim_time;
	short	waterlevel, contents;
} entity_t;

typedef struct
{
	int		sx, sy;
} scrpoint_t;

typedef struct
{
	short	left;
	short	top;
	short	right;
	short	bottom;
} rect_t;

#endif