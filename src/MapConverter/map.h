#ifndef MAPFILE_H
#define MAPFILE_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define	MAX_NUM_FOG_PLANES		16
#define	MAX_NUM_FOGS			16
#define	MAX_NUM_DYNAMIC_LIGHTS	256

enum
{
    FACETYPE_NORMAL		= 1,
    FACETYPE_MESH		= 2,
    FACETYPE_TRISURF	= 3,
    FACETYPE_FLARE		= 4
};

#define	AMBIENT_WATER	0
#define	AMBIENT_SKY		1
#define	AMBIENT_SLIME	2
#define	AMBIENT_LAVA	3

#define MAX_MAP_WALLS			1024	//	static and dynamic walls
#define	MAX_MAP_SECTORS			0x10000
#define	MAX_ENT_SECTORS			0x10000
#define MAX_NUM_ACTIONS			0x2000
#define MAX_ACTIONS_DATA_SIZE	0x10000
#define MAX_NUM_TEXTURES		0x400
#define	MAX_NUM_ENV_TEXTURES	0x20
#define	MAX_MAP_LIGHTS			0x1000

#define	MAX_MAP_MODELS		1024
#define	MAX_MAP_BRUSHES		8192
#define	MAX_MAP_ENTITIES	2048
#define	MAX_MAP_ENTSTRING	0x40000

#define MAX_DYNAMIC_LIGHTS	64
#define	MAX_FOGS			64
#define	MAX_FOG_PLANES		16

#define	MAX_MAP_PLANES		65536
#define	MAX_MAP_NODES		65536
#define	MAX_MAP_BRUSHSIDES	65536
#define	MAX_MAP_LEAFS		65536
#define	MAX_MAP_VERTS		65536
#define	MAX_MAP_FACES		65536
#define	MAX_MAP_LEAFFACES	65536
#define	MAX_MAP_LEAFBRUSHES 65536
#define	MAX_MAP_PORTALS		65536
#define	MAX_MAP_LIGHTING	0x200000
#define	MAX_MAP_VISIBILITY	0x100000

typedef float	texcoord_t[2];	// Texture s&t coordinates
//typedef int		bbox_t[6];		// Integer bounding box (mins, maxs)
//typedef float	bboxf_t[6];		// Float bounding box
typedef byte	colour_t[4];	// RGBA

typedef struct
{
	int		mins[3];
	int		maxs[3];
} bbox_t;

typedef struct
{
	float	mins[3];
	float	maxs[3];
} bboxf_t;

typedef struct
{
    bboxf_t	bbox;
    int		firstface, numfaces;
	int		firstbrush, numbrushes;
} model_t;

typedef struct
{
	vector	normal;
	float	dist;
} plane_t;

typedef struct
{
    int		plane;			// Dividing plane
    int		children[2];	// Left and right node.  Negatives are leafs
    bbox_t	bbox;
} node_t;

typedef struct
{
    int		cluster;		// Visibility cluster number
    int		area;			// area minus one ???
    bbox_t	bbox;
    int		firstface, numfaces;
    int		firstbrush, numbrushes;
} sector_t;

typedef struct
{
	int		firstside;
	int		numsides;
	int		contents;
} brush_t;

typedef struct
{
	int		planenum;
	int		content;
} brushside_t;

typedef struct
{
    int		shader;			// Shader reference
    int		unknown;
    int		facetype;		// FACETYPE enum
    int		firstvert, numverts;
    int		firstelem, numelems;
    int		lm_texnum;		// lightmap info
    int		lm_offset[2];
    int		lm_size[2];
	vector	lm_orig;
	float	lm_vecs[2][4];
    vector	v_orig;			// FACETYPE_NORMAL only
	plane_t	plane;
    bboxf_t	bbox;			// FACETYPE_MESH only
    int		mesh_cp[2];		// mesh control point dimensions
} face_t;

typedef struct
{
    char	name[64];
	int		surface_flags;
    int		content_flags;
} shaderref_t;

typedef struct
{
    int		numclusters;   // Number of PVS clusters
    int		rowsize;
    byte	data[1];
} visibility_t;

typedef struct
{
    vector		v_point;	// World coords
    texcoord_t	tex_st;		// Texture coords
    texcoord_t	lm_st;		// Lightmap texture coords
    vector		v_norm;		// Normal
    colour_t	colour;		// Colour used for vertex lighting
} vertex_t;

// -------------------------------------------------------------

#define	MIPLEVELS	4
typedef struct
{//	текстура
	char	name[31];
	byte	type;
	ushort	firstlayer;
	ushort	numlayers;
	bitmap	bitmaps[MIPLEVELS];
} texture_t;

#define	CONTENTS_SOLID			1		// an eye is never valid in a solid
#define	CONTENTS_LAVA			8
#define	CONTENTS_SLIME			16
#define	CONTENTS_WATER			32
#define	CONTENTS_FOG			64

#define	CONTENTS_AREAPORTAL		0x8000

#define	CONTENTS_PLAYERCLIP		0x10000
#define	CONTENTS_MONSTERCLIP	0x20000
//bot specific contents types
#define	CONTENTS_TELEPORTER		0x40000
#define	CONTENTS_JUMPPAD		0x80000
#define CONTENTS_CLUSTERPORTAL	0x100000
#define CONTENTS_DONOTENTER		0x200000

#define	CONTENTS_ORIGIN			0x1000000	// removed before bsping an entity

#define	CONTENTS_BODY			0x2000000	// should never be on a brush, only in game
#define	CONTENTS_CORPSE			0x4000000
#define	CONTENTS_DETAIL			0x8000000	// brushes not used for the bsp
#define	CONTENTS_STRUCTURAL		0x10000000	// brushes used for the bsp
#define	CONTENTS_TRANSLUCENT	0x20000000	// don't consume surface fragments inside
#define	CONTENTS_TRIGGER		0x40000000
#define	CONTENTS_NODROP			0x80000000	// don't leave bodies or items (death fog, lava)

#define	SURF_NODAMAGE			0x1		// never give falling damage
#define	SURF_SLICK				0x2		// effects game physics
#define	SURF_SKY				0x4		// lighting from environment map
#define	SURF_LADDER				0x8
#define	SURF_NOIMPACT			0x10	// don't make missile explosions
#define	SURF_NOMARKS			0x20	// don't leave missile marks
#define	SURF_FLESH				0x40	// make flesh sounds and effects
#define	SURF_NODRAW				0x80	// don't generate a drawsurface at all
#define	SURF_HINT				0x100	// make a primary bsp splitter
#define	SURF_SKIP				0x200	// completely ignore, allowing non-closed brushes
#define	SURF_NOLIGHTMAP			0x400	// surface doesn't need a lightmap
#define	SURF_POINTLIGHT			0x800	// generate lighting info at vertexes
#define	SURF_METALSTEPS			0x1000	// clanking footsteps
#define	SURF_NOSTEPS			0x2000	// no footstep sounds
#define	SURF_NONSOLID			0x4000	// don't collide against curves with this set
#define SURF_LIGHTFILTER		0x8000	// act as a light filter during q3map -light
#define	SURF_ALPHASHADOW		0x10000	// do per-pixel light shadow casting in q3map
#define	SURF_NODLIGHT			0x20000	// don't dlight even if solid (solid lava, skies)

typedef struct
{//	источник света (надо переделать)
	vector	pos;
	float	i[3];
	float	k_min, period;	//	k!=1, если динамический ист. света
	short	sector, flare;
} light_t;

typedef struct
{//	формат lightmap'ы
	byte	data[128][128][3];
} lmbank_t;

typedef struct
{
	bitmap	right, left;
	bitmap	front, back;
	bitmap	up, down;
} env_tex_t;

// each area has a list of portals that lead into other areas
// when portals are closed, other areas may not be visible or
// hearable even if the vis info says that it should be

enum {TYPE_PORTAL = 0, TYPE_MIRROR};

typedef struct
{
	ushort	planenum;
	byte	side, type;

	short	model;
	int		firstedge;
	ushort	numedges;
	short	texinfo;
	ushort	firstfaces;
	ushort	numfaces;
} portal_t;

#define	TEX_USE_BLEND		1

#define	TEX_USE_SCROLL		4
#define	TEX_USE_ROTATE		8
#define TEX_USE_DISTALPHA	16

#define	TEX_USE_WAVEALPHA	1024
#define	TEX_USE_WAVECOLOR	2048
#define	TEX_USE_WAVESCROLL	4096
#define	TEX_USE_WAVESCALE	8192

enum
{
	MAPPING_ORTHO = 0,
	MAPPING_ENV_SPHERE
};

typedef struct
{
	uint	mapping_type;
	uint	mode;
	uint	texture;

	ushort	sfactor, dfactor;	//	BlendFunc

	struct
	{
		byte	color[4];
		float	u0, v0;
		float	su, sv;
		float	sin_a, cos_a;
	} deformation;

	float	nearalpha, faralpha, neardist, fardist;

	struct
	{
		byte	color[4];			//	r, g, b, a
		byte	wcolor[4];			//	wave r, g, b, a
		float	Ta, Ta1, Ta2;
		float	Tc, Tc1, Tc2;

		float	u0, v0;				//	offset
		float	du, dv;				//	scroll
		float	wdu, wdv, Tscroll;	//	wave scroll

		float	su, sv;				//	scale
		float	wsu, wsv, Tscale;	//	wave scale

		float	angle;				//	rotate
		float	dangle;				//	dynamic rotate
	} animinfo;
} layer_t;

typedef struct
{
	int			numshaders;
	shaderref_t	*shaders;

	int			numplanes;
	plane_t		*planes;

	int			numnodes;
	node_t		*nodes;

	int			numsectors;
	sector_t	*sectors;

	int			numsectorfaces;
	int			*sectorfaces;

	int			numsectorbrushes;
	int			*sectorbrushes;

	int			nummodels;
	model_t		*models;

	int			numbrushes;
	brush_t		*brushes;

	int			numbrushsides;
	brushside_t	*brushsides;

	int			numvertexes;
	vertex_t	*vertexes;

	int			numelements;
	int			*elements;

	//	faces
	int			numfaces;
	face_t		*faces;

	int			numlightmapbanks;
	lmbank_t	*lightmapbanks;

	int			visibilitysize;
	visibility_t	*vis;

	// --------------------------------------------------------

	int			numsectorlights;
	ushort		*sectorlights;

	//	lights
	int			numlights;
	light_t		*lights;

	int			numvisportals;
	portal_t	*visportals;

	int			numportalfaces;
	ushort		*portalfaces;
} TMap;

extern TMap			map;

int		ReadMap (char *filename);
void	WriteMap (char *filename);
int		LoadEntities (char *data, int size);
void	ComputeLightData ();

void    InitProgress (char *str);
void    UpdateProgress (float t);

#ifdef __cplusplus
}
#endif

#endif
