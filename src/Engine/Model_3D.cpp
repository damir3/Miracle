/*
	Model_3D.cpp	Copyright (C) 1998-1999 Damir Sagidullin
*/
#include	<stdio.h>
#include	<stdlib.h>
#include	<math.h>

#include	"MapFile.h"
#include	"Console.h"
#include	"render.h"
#include	"Math_3D.h"
#include	"Model_3D.h"
#include	"PCXFile.h"

extern	CPakFile	unpak;

int		LoadModel(model3d_t *model, char *name)
{
	if(!unpak.OpenPak(name))	return	0;
	char	*buf;
	if(unpak.ExtractByName("model.dat", &buf)<=0)
	{
		unpak.ClosePak();
		return	0;
	}
	memset(model, 0, sizeof(model3d_t));
	if(LoadPCX24((bitmap *)&model->texture, "texture.pcx", &unpak)<=0)
	{
		unpak.ClosePak();
		return	0;
	}
	unpak.ClosePak();

	mdl_header_t	*header = (mdl_header_t *)buf;
	int	cur_offset = sizeof(mdl_header_t);

	if(strcmp(header->id, "= Model"))
	{ 
		CPrintf("Unknown model format");
		return 0;
	}
	int		i, j, k;
	model->numvertices = header->numvertices;
	model->numfaces = header->numfaces;
	//model->numvertices = header->numfaces*3;

	model->vertices = (vec3_t *) malloc(model->numvertices*sizeof(vec3_t));
	model->vnormals = (vec3_t *) malloc(model->numvertices*sizeof(vec3_t));
	vec3_t	*v = (vec3_t *)(buf + cur_offset);
	memcpy(model->vertices, buf + cur_offset, model->numvertices*sizeof(vec3_t));
	memset(model->vnormals, 0, model->numvertices*sizeof(vec3_t));
	cur_offset += header->numvertices*sizeof(vec3_t);

	model->faces = (mdl_face_t *)malloc(model->numfaces*sizeof(mdl_face_t));
	memcpy(model->faces, buf + cur_offset, model->numfaces*sizeof(mdl_face_t));
	cur_offset += model->numfaces*sizeof(mdl_face_t);

	/*j=0;
	for(i=0; i<model->numfaces; i++)
	{
		mdl_face_t	*face = &model->faces[i];
		VectorCopy(v[face->a[0]], model->vertices[j]);
		face->a[0] = j++;
		VectorCopy(v[face->a[1]], model->vertices[j]);
		face->a[1] = j++;
		VectorCopy(v[face->a[2]], model->vertices[j]);
		face->a[2] = j++;
	}*/

	free(buf);
	//	Find object radius
	float	radius;
	model->radius = 0;
	vec3_t	mins, maxs;
	VectorCopy(model->vertices[0], mins);
	VectorCopy(maxs, mins);
	for(i=0; i<model->numvertices; i++)
	{
		radius = PointPointDist2(model->vertices[i], vec3_null);
		if(radius>model->radius)	model->radius = radius;
		for(j=0; j<3; j++)
		{
			if(mins[j] > model->vertices[i][j])
				mins[j] = model->vertices[i][j];
			if(maxs[j] < model->vertices[i][j])
				maxs[j] = model->vertices[i][j];
		}
	}
	model->radius = float(sqrt(model->radius));
	//	Find normal vectors at points (for lighting)
	int		*numnormals = (int *)malloc(model->numvertices*sizeof(int));
	memset(numnormals, 0, model->numvertices*sizeof(int));
	for(i=0; i<model->numfaces; i++)
	{
		mdl_face_t	*face = &model->faces[i];
		vec3_t	normal, a, b;
		VectorSubtract(model->vertices[face->a[2]], model->vertices[face->a[0]], a);
		VectorSubtract(model->vertices[face->a[1]], model->vertices[face->a[0]], b);
		CrossProduct (a, b, normal);
		VectorNormalize(normal);
		for(j=0; j<3; j++)
		{
			k = face->a[j];
			VectorAdd(model->vnormals[k], normal, model->vnormals[k]);
			numnormals[k]++;
		}
	}
	for(i=0; i<model->numvertices; i++)
	{
		if(numnormals[i]>1)
			VectorScale(model->vnormals[i], 1.0f/numnormals[i], model->vnormals[i]);
	}
	free(numnormals);
	return	1;
}

void	CloseModel(model3d_t *model)
{
	free(model->texture.bm);
	free(model->faces);
	free(model->vnormals);
	free(model->vertices);
}
