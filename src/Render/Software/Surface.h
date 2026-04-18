#ifndef _SURFACE_H
#define _SURFACE_H

#include	"drawspan.h"
#include	"MapFile.h"

#define	MAX_NUM_DYNAMIC_LIGHTS	256

typedef struct
{
	int		next_cache;
	int		face;
	int		mip_level;
	float	u,v;
	texmap	*bm;
} surface_t;

typedef	struct
{
	vec3_t	pos;
    float	i[3];
    float	max_radius, max_radius2;
} dynamic_light;

void	InitCache();
void	RemakeCache();
void	GetTextureMap(texmap *texture, int face, int texnum, float *u, float *v);
void	RefreshSurfaceCache();

#endif
