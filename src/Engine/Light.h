#ifndef	_LIGHT_H
#define	_LIGHT_H

#include	"math_3d.h"

typedef	struct
{
	vec3_t	pos;
    float	r_rate, g_rate, b_rate;	// rate = light*(radius)^2
    float	max_radius;
} omni_light;

bool	RecursiveCheck(int, float, float, vec3_t, vec3_t, trace_t *);
void	AddOmniLight(omni_light *l);
void	OmniLight();

#endif