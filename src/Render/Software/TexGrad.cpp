/*
	TexGrad.cpp		Copyright (C) 1998-1999 Damir Sagidullin
*/
#include	<Math.h>

#include	"Const.h"
#include	"Math_3D.h"
#include	"MapFile.h"
#include	"TexGrad.h"
#include	"DrawSpan.h"
#include	"Video.h"

int		minmiplevel, maxmiplevel;

#define DIST	256.0    // dist to switch first mip level
const float	one_div_dist = 1.0/DIST;

extern TMap		map;
extern float	z_gradient[3];
extern float	tex_gradient[9];
extern int		mirror;

void	ComputeFaceMipLevel(int n, point_3d **vl)
{
	int		i;
	float	zmin, zmax;

	zmin = zmax = vl[0]->p[2];
	for (i=1; i<n; i++)
	{
		if (vl[i]->p[2] < zmin)	zmin = vl[i]->p[2];
		else
		if (vl[i]->p[2] > zmax)	zmax = vl[i]->p[2];
	}
	zmin *= float(mip_scale*one_div_dist);
	zmax *= float(mip_scale*one_div_dist);
	if(zmin < 2)
	{
		if(zmin < 1)
			minmiplevel = 0;
		else
			minmiplevel = 1;
	} else
	{
		if(zmin < 4)
			minmiplevel = 2;
		else
			minmiplevel = 3;
	}
	if(zmax < 2)
	{
		if(zmax < 1)
			maxmiplevel = 0;
		else
			maxmiplevel = 1;
	} else
	{
		if(zmax < 4)
			maxmiplevel = 2;
		else
			maxmiplevel = 3;
	}
}
extern double	proj_scale_x, proj_scale_y;
void	ComputeZGradients(t_plane *plane)
{
	vec3_t	W;
	double	gradient_scale = -65536.0/DistToCamera(plane);
	TransformVector(W, plane->normal);
	W[0] *= float(gradient_scale/(proj_scale_x*proj_scale_x));
	if(mirror)	W[0] = -W[0];
	W[1] = -W[1]*float(gradient_scale/(proj_scale_y*proj_scale_y));
	W[2] *= float(gradient_scale);
	W[2] -= (W[0]*xcenter) + (W[1]*ycenter);
	z_gradient[0] = W[2];
	z_gradient[1] = W[0];
	z_gradient[2] = W[1];
}
void	ComputeTextureGradients(int face, int tex, float u, float v)
{
	float	rescale;
	vec3_t	P, M, N;

	u -= map.texinfo[tex].vecs[0][3];
	v -= map.texinfo[tex].vecs[1][3];

	
	TransformVector(M, map.faces_consts[face].u);
	TransformVector(N, map.faces_consts[face].v);
	if(face_type==3)	// sky
		TransformVector(P, map.faces_consts[face].w);
	else	// wall or water
		TransformPointRaw(P, map.faces_consts[face].w);

	P[0] += u * M[0] + v * N[0];
	P[1] += u * M[1] + v * N[1];
	P[2] += u * M[2] + v * N[2];

	tex_gradient[0] = (P[0]*N[1] - P[1]*N[0]);
	tex_gradient[1] = (P[1]*N[2] - P[2]*N[1]);
	tex_gradient[2] = (P[0]*N[2] - P[2]*N[0]);
	tex_gradient[3] = (P[1]*M[0] - P[0]*M[1]);
	tex_gradient[4] = (P[2]*M[1] - P[1]*M[2]);
	tex_gradient[5] = (P[2]*M[0] - P[0]*M[2]);
	tex_gradient[6] = (N[0]*M[1] - N[1]*M[0]);
	tex_gradient[7] = (N[1]*M[2] - N[2]*M[1]);
	tex_gradient[8] = (N[0]*M[2] - N[2]*M[0]);
	if(mirror)
	{
		tex_gradient[1] = -tex_gradient[1];
		tex_gradient[4] = -tex_gradient[4];
		tex_gradient[7] = -tex_gradient[7];
	}

	tex_gradient[0] -= tex_gradient[1]*xcenter + tex_gradient[2]*ycenter;
	tex_gradient[3] -= tex_gradient[4]*xcenter + tex_gradient[5]*ycenter;
	tex_gradient[6] -= tex_gradient[7]*xcenter + tex_gradient[8]*ycenter;

	if ((face_type!=2) && (face_type!=3))
	{
		rescale = (8 >> minmiplevel) / 8.0f;
		tex_gradient[0] *= rescale;
		tex_gradient[1] *= rescale;
		tex_gradient[2] *= rescale;
		tex_gradient[3] *= rescale;
		tex_gradient[4] *= rescale;
		tex_gradient[5] *= rescale;
	}
}
void	ProjectVectorToPlane(vec3_t out, vec3_t in, vec3_t norm, vec3_t plane)
{
	float	dot = -DotProduct(in, plane)/DotProduct(norm, plane);
	VectorCopy(in, out);
	if(dot != 0)
	{
		out[0] += norm[0]*dot;
		out[1] += norm[1]*dot;
		out[2] += norm[2]*dot;
	}
}
void	ComputeOriginVector(vec3_t out, t_plane *plane)
{
	float	d =  plane->dist / DotProduct(out, plane->normal);
	out[0] *= d;
	out[1] *= d;
	out[2] *= d;
}
void	ComputeDynamicTextureGradients(int face, int tex, float u, float v, t_plane *plane)
{
	float	rescale;
	vec3_t	mu, mv, mw;
	vec3_t	P, M, N;

	CrossProduct(map.texinfo[tex].vecs[0], map.texinfo[tex].vecs[1], mw);
	ProjectVectorToPlane(mu, map.texinfo[tex].vecs[0], mw, plane->normal);
	ProjectVectorToPlane(mv, map.texinfo[tex].vecs[1], mw, plane->normal);
	ComputeOriginVector(mw, plane);

	TransformVector(M, mu);
	TransformVector(N, mv);
	TransformPointRaw(P, mw);

	u -= map.texinfo[tex].vecs[0][3];
	v -= map.texinfo[tex].vecs[1][3];

	P[0] += u * M[0] + v * N[0];
	P[1] += u * M[1] + v * N[1];
	P[2] += u * M[2] + v * N[2];

	tex_gradient[0] = (P[0]*N[1] - P[1]*N[0]);
	tex_gradient[1] = (P[1]*N[2] - P[2]*N[1]);
	tex_gradient[2] = (P[0]*N[2] - P[2]*N[0]);
	tex_gradient[3] = (P[1]*M[0] - P[0]*M[1]);
	tex_gradient[4] = (P[2]*M[1] - P[1]*M[2]);
	tex_gradient[5] = (P[2]*M[0] - P[0]*M[2]);
	tex_gradient[6] = (N[0]*M[1] - N[1]*M[0]);
	tex_gradient[7] = (N[1]*M[2] - N[2]*M[1]);
	tex_gradient[8] = (N[0]*M[2] - N[2]*M[0]);
	if(mirror)
	{
		tex_gradient[1] = -tex_gradient[1];
		tex_gradient[4] = -tex_gradient[4];
		tex_gradient[7] = -tex_gradient[7];
	}

	tex_gradient[0] -= tex_gradient[1]*xcenter + tex_gradient[2]*ycenter;
	tex_gradient[3] -= tex_gradient[4]*xcenter + tex_gradient[5]*ycenter;
	tex_gradient[6] -= tex_gradient[7]*xcenter + tex_gradient[8]*ycenter;

	
	if ((face_type!=2) && (face_type!=3))
	{
		rescale = (8 >> minmiplevel) / 8.0f;
		tex_gradient[0] *= rescale;
		tex_gradient[1] *= rescale;
		tex_gradient[2] *= rescale;
		tex_gradient[3] *= rescale;
		tex_gradient[4] *= rescale;
		tex_gradient[5] *= rescale;
	}
}
void	ComputeMarkTextureGradients(wmark_t *mark)
{
	vec3_t	P, M, N;
	float	u, v;
	TransformVector(M, mark->u);
	TransformVector(N, mark->v);
	TransformPointRaw(P, mark->w);

	u = mark->bm->width*0.5f;
	v = mark->bm->height*0.5f;
	P[0] += u * M[0] + v * N[0];
	P[1] += u * M[1] + v * N[1];
	P[2] += u * M[2] + v * N[2];

	tex_gradient[0] = (P[0]*N[1] - P[1]*N[0]);
	tex_gradient[1] = (P[1]*N[2] - P[2]*N[1]);
	tex_gradient[2] = (P[0]*N[2] - P[2]*N[0]);
	tex_gradient[3] = (P[1]*M[0] - P[0]*M[1]);
	tex_gradient[4] = (P[2]*M[1] - P[1]*M[2]);
	tex_gradient[5] = (P[2]*M[0] - P[0]*M[2]);
	tex_gradient[6] = (N[0]*M[1] - N[1]*M[0]);
	tex_gradient[7] = (N[1]*M[2] - N[2]*M[1]);
	tex_gradient[8] = (N[0]*M[2] - N[2]*M[0]);
	if(mirror)
	{
		tex_gradient[1] = -tex_gradient[1];
		tex_gradient[4] = -tex_gradient[4];
		tex_gradient[7] = -tex_gradient[7];
	}

	tex_gradient[0] -= tex_gradient[1]*xcenter + tex_gradient[2]*ycenter;
	tex_gradient[3] -= tex_gradient[4]*xcenter + tex_gradient[5]*ycenter;
	tex_gradient[6] -= tex_gradient[7]*xcenter + tex_gradient[8]*ycenter;
}