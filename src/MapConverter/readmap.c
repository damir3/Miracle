/*
	readmap.c		Copyright (C) 2000 Damir Sagidullin
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "map.h"
#include "vector.h"

//------------------- DEFINITIONS & CONSTANTS -------------------//

#define BSPHEADERID  (*(int*)"IBSP")
#define BSPVERSION 46

#define	BIG_NUMBER 1000000000

enum
{
    LUMP_ENTITIES = 0,
    LUMP_SHADERREFS,
    LUMP_PLANES,
    LUMP_NODES,
    LUMP_LEAFS,
    LUMP_LFACES,				// leaf faces
    LUMP_LBRUSHES,			// leaf brushes
    LUMP_MODELS,
    LUMP_BRUSHES,
	LUMP_BRUSH_SIDES,
    LUMP_VERTS,
    LUMP_ELEMS,
    LUMP_FOG,
    LUMP_FACES,
    LUMP_LIGHTMAPS,
    LUMP_LIGHTGRID,
    LUMP_VISIBILITY,
    NUM_LUMPS
};

static struct header
{
    int		id, ver;
    struct
	{
		int		fileofs;
		int		filelen;
	} lumps[NUM_LUMPS];
} *bspheader;

typedef struct
{
	int		shader;
    int		unknown;
    int		facetype;
    int		firstvert, numverts;
    int		firstelem, numelems;
    int		lm_texnum;
    int		lm_offset[2];
    int		lm_size[2];
    vector	v_orig;
    bboxf_t	bbox;
    vector	v_norm;
    int		mesh_cp[2];
} map_face_t;

//-------------------------- VARIABLES --------------------------//

//static bsp_header	*bspheader;
static char		*mapdata;
static int		datasize;
static int		load_map_error;

TMap	map;

//-------------------------- FUNCTIONS --------------------------//

void	ComputeFaces ()
{
	int		i, j, k, u, v;
	vector	a, b;
	float	d, du1, du2, dv1, dv2, s, t;
	face_t		*face;
	vertex_t	*vertex;

	for (i=0, face = map.faces; i<map.numfaces; i++, face++)
	{
		switch (face->facetype)
		{
		case FACETYPE_NORMAL:
			vertex = map.vertexes + face->firstvert;

			map.faces[i].plane.dist = DotProduct (face->plane.normal, vertex->v_point);

			VectorSet (face->bbox.mins, BIG_NUMBER, BIG_NUMBER, BIG_NUMBER);
			VectorSet (face->bbox.maxs, -BIG_NUMBER, -BIG_NUMBER, -BIG_NUMBER);
			for (j=0; j<face->numverts; j++, vertex++)
			{
				for (k=0; k<3; k++)
				{
					if (face->bbox.mins[k] > vertex->v_point[k])
						face->bbox.mins[k] = vertex->v_point[k];
					if (face->bbox.maxs[k] < vertex->v_point[k])
						face->bbox.maxs[k] = vertex->v_point[k];
				}
			}

			vertex = map.vertexes + face->firstvert;
			if (face->lm_texnum >= 0)
			{
				for (j=0; j<face->numverts - 2; j++, vertex++)
				{
					du1 = vertex[1].lm_st[0] - vertex[0].lm_st[0];
					dv1 = vertex[1].lm_st[1] - vertex[0].lm_st[1];
					
					du2 = vertex[2].lm_st[0] - vertex[0].lm_st[0];
					dv2 = vertex[2].lm_st[1] - vertex[0].lm_st[1];
					
					d = du1*dv2 - du2*dv1;
					if (d != 0) break;
				}

				VectorSubtract (vertex[1].v_point, vertex[0].v_point, a);
				VectorSubtract (vertex[2].v_point, vertex[0].v_point, b);

				face->lm_vecs[0][0] = a[0]*dv2 - b[0]*dv1;
				face->lm_vecs[0][1] = a[1]*dv2 - b[1]*dv1;
				face->lm_vecs[0][2] = a[2]*dv2 - b[2]*dv1;
				VectorScale (face->lm_vecs[0], 1.0f/d, face->lm_vecs[0]);
				face->lm_vecs[0][3] = DotProduct (face->lm_vecs[0], vertex->v_point) - vertex->lm_st[0];

				face->lm_vecs[1][0] = b[0]*du1 - a[0]*du2;
				face->lm_vecs[1][1] = b[1]*du1 - a[1]*du2;
				face->lm_vecs[1][2] = b[2]*du1 - a[2]*du2;
				VectorScale (face->lm_vecs[1], 1.0f/d, face->lm_vecs[1]);
				face->lm_vecs[1][3] = DotProduct (face->lm_vecs[1], vertex->v_point) - vertex->lm_st[1];

				VectorAddScale (vertex->v_point, face->lm_vecs[0], -vertex->lm_st[0], face->lm_orig);
				VectorAddScale (face->lm_orig, face->lm_vecs[1], -vertex->lm_st[1], face->lm_orig);

				/*vertex = map.vertexes + face->firstvert;
				CPrintf ("Face[%d]: u(%.2f,%.2f,%.2f) v(%.2f,%.2f,%.2f) d=%f", i,
					face->lm_vecs[0][0], face->lm_vecs[0][1], face->lm_vecs[0][2],
					face->lm_vecs[1][0], face->lm_vecs[1][1], face->lm_vecs[1][2], d);
				CPrintf ("\tofs=%dx%d size=%dx%d", face->lm_offset[0], face->lm_offset[1], face->lm_size[0], face->lm_size[1]);
				for (j=0; j<face->numverts; j++)
				{
					CPrintf ("\tvertex[%d]\t(%.2f,%.2f,%.2f) (%.2f,%.2f)", j, vertex[j].v_point[0], vertex[j].v_point[1], vertex[j].v_point[2], vertex[j].lm_st[0], vertex[j].lm_st[1]);
					VectorAddScale (face->lm_orig, face->lm_vecs[0], vertex[j].lm_st[0], a);
					VectorAddScale (a, face->lm_vecs[1], vertex[j].lm_st[1], a);
					CPrintf ("\t\t\t\t(%.2f,%.2f,%.2f)", a[0], a[1], a[2]);
				}*/
			} else
			{
				for (j=0; j<face->numverts - 2; j++, vertex++)
				{
					du1 = vertex[1].tex_st[0] - vertex[0].tex_st[0];
					dv1 = vertex[1].tex_st[1] - vertex[0].tex_st[1];
					
					du2 = vertex[2].tex_st[0] - vertex[0].tex_st[0];
					dv2 = vertex[2].tex_st[1] - vertex[0].tex_st[1];
					
					d = du1*dv2 - du2*dv1;
					if (d != 0) break;
				}
				
				VectorSubtract (vertex[1].v_point, vertex[0].v_point, a);
				VectorSubtract (vertex[2].v_point, vertex[0].v_point, b);

				face->lm_vecs[0][0] = a[0]*dv2 - b[0]*dv1;
				face->lm_vecs[0][1] = a[1]*dv2 - b[1]*dv1;
				face->lm_vecs[0][2] = a[2]*dv2 - b[2]*dv1;
				VectorScale (face->lm_vecs[0], 1.0f/(16.0f*d), face->lm_vecs[0]);
				face->lm_vecs[0][3] = DotProduct (face->lm_vecs[0], vertex->v_point) - vertex->lm_st[0];
				
				face->lm_vecs[1][0] = b[0]*du1 - a[0]*du2;
				face->lm_vecs[1][1] = b[1]*du1 - a[1]*du2;
				face->lm_vecs[1][2] = b[2]*du1 - a[2]*du2;
				VectorScale (face->lm_vecs[1], 1.0f/(16.0f*d), face->lm_vecs[1]);
				face->lm_vecs[1][3] = DotProduct (face->lm_vecs[1], vertex->v_point) - vertex->lm_st[1];

				/*vertex = map.vertexes + face->firstvert;
				printf ("Face[%d]: u(%.2f,%.2f,%.2f) v(%.2f,%.2f,%.2f) d=%f", i,
					face->lm_vecs[0][0], face->lm_vecs[0][1], face->lm_vecs[0][2],
					face->lm_vecs[1][0], face->lm_vecs[1][1], face->lm_vecs[1][2], d);
				for (j=0; j<face->numverts; j++)
				{
					printf ("\tvertex[%d]\t(%.2f,%.2f,%.2f) (%.2f,%.2f)", j, vertex[j].v_point[0], vertex[j].v_point[1], vertex[j].v_point[2], vertex[j].lm_st[0], vertex[j].lm_st[1]);
					VectorAddScale (face->lm_orig, face->lm_vecs[0], vertex[j].lm_st[0], a);
					VectorAddScale (a, face->lm_vecs[1], vertex[j].lm_st[1], a);
					printf ("\t\t\t\t(%.2f,%.2f,%.2f)", a[0], a[1], a[2]);
				}*/
				VectorAddScale (vertex->v_point, face->lm_vecs[0], -vertex->lm_st[0], face->lm_orig);
				VectorAddScale (face->lm_orig, face->lm_vecs[1], -vertex->lm_st[1], face->lm_orig);
			}
			break;
		case FACETYPE_MESH:
			/*vertex = map.vertexes + face->firstvert;
			printf ("\tmesh(%dx%d): ofs=%dx%d size=%dx%d", face->mesh_cp[0], face->mesh_cp[1], face->lm_offset[0], face->lm_offset[1], face->lm_size[0], face->lm_size[1]);
			for (j=0; j<face->numverts; j++)
			{
				printf ("\tvertex[%d]\t(%.2f,%.2f,%.2f) (%.2f,%.2f)", j, vertex[j].v_point[0], vertex[j].v_point[1], vertex[j].v_point[2], vertex[j].lm_st[0]*128, vertex[j].lm_st[1]*128);
			}*/
            vertex = map.vertexes + face->firstvert;
            for (v=0; v<face->mesh_cp[1]; v++) {
                for (u=0; u<face->mesh_cp[0]; u++) {
                    s = (float)u / (float)(face->mesh_cp[0]-1);
                    t = (float)v / (float)(face->mesh_cp[1]-1);
                    vertex->lm_st[0] = (s * (face->lm_size[0]-1) + face->lm_offset[0] + 0.5f) / 128.0f;
                    vertex->lm_st[1] = (t * (face->lm_size[1]-1) + face->lm_offset[1] + 0.5f) / 128.0f;
                    vertex++;
                }
            }
			break;
		}
	}
}

static int	ReadFaces ()
{
	map_face_t	*face;
	int		i, size, len;

	size = sizeof (map_face_t);
	len = bspheader->lumps[LUMP_FACES].filelen;

    map.numfaces = len / size;

	if (map.numfaces * size != len)
		printf ("Error: odd faces lump size=%d (face size=%d)\n", LUMP_FACES, len, size);

	map.faces = malloc (map.numfaces * sizeof (face_t));

	face = (map_face_t *)(mapdata + bspheader->lumps[LUMP_FACES].fileofs);
	for (i=0; i<map.numfaces; i++, face++)
	{
		map.faces[i].shader = face->shader;
		map.faces[i].facetype = face->facetype;
		map.faces[i].firstvert = face->firstvert;
		map.faces[i].numverts = face->numverts;
		map.faces[i].firstelem = face->firstelem;
		map.faces[i].numelems = face->numelems;
		map.faces[i].lm_texnum = face->lm_texnum;
		map.faces[i].lm_offset[0] = face->lm_offset[0];
		map.faces[i].lm_offset[1] = face->lm_offset[1];
		map.faces[i].lm_size[0] = face->lm_size[0];
		map.faces[i].lm_size[1] = face->lm_size[1];
		VectorCopy (face->v_orig, map.faces[i].v_orig);
		map.faces[i].bbox = face->bbox;
		VectorCopy (face->v_norm, map.faces[i].plane.normal);
		map.faces[i].mesh_cp[0] = face->mesh_cp[0];
		map.faces[i].mesh_cp[1] = face->mesh_cp[1];
	}

    return map.numfaces;
}

static int	ReadLump (int lump, void **mem, int elem_size)
{
    int		len = bspheader->lumps[lump].filelen;
    int		num = len / elem_size;

	if (num * elem_size != len)
		printf ("Error: odd lump[%d] size=%d (elem size=%d)\n", lump, len, elem_size);

    *mem = malloc(len);
    memcpy (*mem, mapdata + bspheader->lumps[lump].fileofs, len);

    return num;
}

void	SaveLump (char *filename, int lump, void *mem)
{
    int		len = bspheader->lumps[lump].filelen;

	memcpy (mapdata + bspheader->lumps[lump].fileofs, mem, len);
}

void	ShowMapInfo ()
{
	printf ("\t%d shader\n", map.numshaders);
	printf ("\t%d planes\n", map.numplanes);
	printf ("\t%d models\n", map.nummodels);
	printf ("\t%d sectors\n", map.numsectors);
	printf ("\t%d sector faces\n", map.numsectorfaces);
	printf ("\t%d sector brushes\n", map.numsectorbrushes);
	printf ("\t%d vertexes\n", map.numvertexes);
	printf ("\t%d elements\n", map.numelements);
	printf ("\t%d faces\n", map.numfaces);
	printf ("\t%d brushes\n", map.numbrushes);
	printf ("\t%d brush sides\n", map.numbrushsides);
	printf ("\t%d lightmap banks\n", map.numlightmapbanks);
	printf ("\t%d visibility size\n", map.visibilitysize);
}

int		ReadMap (char *filename)
{
	FILE	*f;

	f = fopen (filename, "rb");
	if (f == NULL)
	{
		printf ("Can't open file \"%s\"\n", filename);
		return FALSE;
	}

	printf ("Reading map \"%s\"\n", filename);
	fseek (f, 0, SEEK_END);
	datasize = ftell (f);
	mapdata = (char *)malloc (datasize);
	fseek (f, 0, SEEK_SET);
	fread (mapdata, 1, datasize, f);
	fclose (f);

	bspheader = (struct header*)mapdata;
    if (bspheader->id != BSPHEADERID)
	{
		printf ("Not a bsp file: %s\n", filename);
		free (mapdata);
		return FALSE;
	}
    if (bspheader->ver != BSPVERSION)
	{
		printf ("Bad bsp file version\n");
		free (mapdata);
		return FALSE;
	}

	// --------------------- Loading data ---------------------
	map.numshaders = ReadLump (LUMP_SHADERREFS, &map.shaders, sizeof(shaderref_t));
	map.numplanes = ReadLump (LUMP_PLANES, &map.planes, sizeof(plane_t));
	map.numnodes = ReadLump (LUMP_NODES, &map.nodes, sizeof(node_t));
	map.numsectors = ReadLump (LUMP_LEAFS, &map.sectors, sizeof(sector_t));
	map.numsectorfaces = ReadLump (LUMP_LFACES, &map.sectorfaces, sizeof(int));
	map.numsectorbrushes = ReadLump (LUMP_LBRUSHES, &map.sectorbrushes, sizeof(int));
	map.nummodels = ReadLump (LUMP_MODELS, &map.models, sizeof(model_t));
	map.numvertexes = ReadLump (LUMP_VERTS, &map.vertexes, sizeof(vertex_t));
	map.numelements = ReadLump (LUMP_ELEMS, &map.elements, sizeof(int));
	ReadFaces ();
	map.numbrushes = ReadLump (LUMP_BRUSHES, &map.brushes, sizeof(brush_t));
	map.numbrushsides = ReadLump (LUMP_BRUSH_SIDES, &map.brushsides, sizeof(brushside_t));
	map.numlightmapbanks = ReadLump (LUMP_LIGHTMAPS, &map.lightmapbanks, sizeof(lmbank_t));
	map.visibilitysize = ReadLump (LUMP_VISIBILITY, &map.vis, 1);
	LoadEntities (mapdata + bspheader->lumps[LUMP_ENTITIES].fileofs, bspheader->lumps[LUMP_ENTITIES].filelen);

	printf ("Read %d lights\n", map.numlights);

	ShowMapInfo ();

	if (load_map_error) return FALSE;

	ComputeFaces ();
	return TRUE;
}

void	WriteMap (char *filename)
{
	FILE	*f;

	SaveLump (filename, LUMP_LIGHTMAPS, map.lightmapbanks);

	printf ("Writing map \"%s\"\n", filename);

	f = fopen (filename, "wb");

	fwrite (mapdata, 1, datasize, f);

	fclose (f);
}