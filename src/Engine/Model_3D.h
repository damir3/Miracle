#ifndef MODEL_3D_H_
#define MODEL_3D_H_

#include	"struct.h"

typedef struct
{
	char	id[8];
	int		numvertices;
	int		numfaces;
} mdl_header_t;

typedef struct
{
    int		a[3];
} mdl_face_t;

typedef struct
{
	int		numvertices;
	int		numfaces;
	float	radius;
    bitmap	texture;
    vec3_t	*vertices;
	vec3_t	*vnormals;
    mdl_face_t	*faces;
} model3d_t;

int		LoadModel(model3d_t *model, char *name);
void	CloseModel(model3d_t *model);

#endif