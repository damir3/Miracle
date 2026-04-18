#ifndef DRAWMODEL_H_
#define DRAWMODEL_H_

#include	"MapFile.h"

typedef struct
{
    int		sx, edge_sx;
    float	uz, vz, dz, rz, gz, bz;
} EdgeVertex;

typedef struct
{
    int		a[3];
} TFace;

typedef struct
{
	int		numvertices;
	int		numfaces;
	float	radius;
    bitmap	texture;
    vec3_t	*vertices;
	vec3_t	*vnormals;
    TFace	*faces;
} TModel;

typedef struct
{
	vec3_t	pos, angle;
	float	scale;
	TModel	*model;
	byte	alpha, type;
	short	mark;
} object_t;

extern int		numobjects;
extern object_t	vis_objects[256];

int		VisibleCheck(vec3_t pos, float dist);
void	DrawModel_(object_t *obj);

void	DrawModel(vec3_t pos, vec3_t angle, TModel *model, float scale, byte alpha, byte type);

#endif