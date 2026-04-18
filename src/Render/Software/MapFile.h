#ifndef _MAPFILE_H
#define _MAPFILE_H

#include	"Struct.h"

#define	CONTENTS_EMPTY	-1
#define	CONTENTS_SOLID	-2
#define	CONTENTS_WATER	-3
#define	CONTENTS_SLIME	-4
#define	CONTENTS_LAVA	-5
#define	CONTENTS_SKY	-6

#define	AMBIENT_WATER	0
#define	AMBIENT_SKY		1
#define	AMBIENT_SLIME	2
#define	AMBIENT_LAVA	3

#define	MAX_MAP_NODES			32767
#define	MAX_MAP_FACES			65535

typedef float vec3_t[3];

#define	MAX_MAP_HULLS	4
typedef struct
{
	float	mins[3], maxs[3];
	float	origin[3];
	int		headnode[MAX_MAP_HULLS];
	int		visleafs;
	int		firstface, numfaces;
} model_t;

#define	MIPLEVELS		4
typedef struct miptex_s
{
	char	name[32];
	byte	type;
	byte	r, g, b;
	int		width, height;
	byte	*offsets[MIPLEVELS];
} t_texture;

typedef struct
{
	float	point[3];
} t_vertex;

typedef struct
{//	уравнение плоскоти
	float	normal[3];
	float	dist;
	byte	type;
	byte	signbits;
	byte	pad[2];
} t_plane;

typedef struct	wmark_t
{//	след на стене
	ushort	node, tracknum;
	byte	side, type;
	vec3_t	u, v, w;
	vec3_t	p[4];
	bitmap	*bm;
	wmark_t	*nextwmark;
} wmark_t;

typedef struct
{
	int		planenum;
	short	children[2];
	short	mins[3];
	short	maxs[3];
	ushort	firstface;
	ushort	numfaces;
	wmark_t	*firstwmark;
} t_node;

typedef struct
{
	int		planenum;
	short	children[2];
} t_clipnode;

typedef struct texinfo_s
{
	float	vecs[2][4];
	int		texnum;
	int		flags;
} t_texinfo;

typedef struct
{
	char	type;
	char	name[32];
} map_texture;

typedef struct
{
	unsigned short	v[2];
} t_edge;

#define	MAXLIGHTMAPS	4
typedef struct
{
	short	planenum;
	char	side;
	char	type;

	int		firstedge;
	short	numedges;
	short	texinfo;

	byte	styles[MAXLIGHTMAPS];
	int		lightofs;
} t_face;

#define	NUM_AMBIENTS	4
typedef struct
{
	int		contents;
	int		visofs;

	short	mins[3];
	short	maxs[3];

	unsigned short	firstmarksurface;
	unsigned short	nummarksurfaces;
	unsigned short	firstportal;
	unsigned short	numportals;
	unsigned short	firstlight;
	unsigned short	numlights;

	byte	ambient_level[NUM_AMBIENTS];
} t_sector;

typedef struct
{
	short	planenum;
	short	side;
	int		sector[2];
	int		first_vertex, num_vertices;
} t_portal;

typedef struct
{
	vec3_t	pos;
	float	i[3];
	float	k_min, period;	//	k!=1, если динамический ист. света
	short	sector, volume_light;
} t_light;

typedef struct
{
	int		offset;
	int		length;
} header_lump;

#define	NUM_HEADER_LUMPS	18
typedef struct
{
	char		id[16];
	header_lump	lumps[NUM_HEADER_LUMPS];
} map_header;

typedef struct
{
	byte	r;
	byte	g;
	byte	b;
} t_lightdata;


typedef struct
{//	данные о полигоне, которые просчитываются при загрузке карты
	byte	r, g, b;	//	средняя освещенность полигона
	int		u0, v0, u1, v1;
	float	u[3];
	float	v[3];
	float	w[3];
} face_consts;

typedef struct
{//	dynamic	wall
	int		model, sounds;
	float	dist, radius;
	int		direction, action_time_end;
	int		cur_action, end_action;
	vec3_t	pos, prev_pos;
	vec3_t	angle, prev_angle, turn_pos;
	vec3_t	begin_pos, begin_angle;
	ushort	first_sector, num_sectors;
	char	*targetname, *actions;
	float	prev_tmatrix[3][3], tmatrix[3][3];
} d_wall_t;

typedef struct
{//	static wall
	int		model;
	ushort	first_sector, num_sectors;
} s_wall_t;


typedef struct
{
	vec3_t	pos;
	vec3_t	angle;
	ushort	targetnum;
	char	*targetname, *target;
} t_illusion_portal;
//	fog
#define	MAX_NUM_FOG_PLANES	16
typedef struct
{
	vec3_t	mins, maxs;
	float	color[3];
	float	density;
	int		numplanes;
	t_plane	planes[MAX_NUM_FOG_PLANES];
} fog_t;

typedef struct
{
	int			map_already_loaded;
	int			nummodels;
	model_t		*models;
	int			visdatasize;
	byte		*vismap;
	int			numtextures;
	t_texture	*textures;
	int			numsectors;
	t_sector	*sectors;
	int			numplanes;
	t_plane		*planes;
	int			numvertices;
	t_vertex	*vertices;
	int			numnodes;
	t_node		*nodes;
	int			numtexinfo;
	t_texinfo	*texinfo;
	int			numfaces;
	t_face		*faces;
	face_consts	*faces_consts;
	int			numedges;
	t_edge		*edges;
	int			nummarksurfaces;
	ushort		*marksurfaces;
	int			numsurfedges;
	int			*surf_edges;
	int			lightdatasize;
	t_lightdata	*lightmaps;
	t_plane		*edge_planes[2];
	int			numportals;
	t_portal	*portals;
	int			numsurfportals;
	int			*surf_portals;
	int			numlights;
	t_light		*lights;
	int			numlightsindex;
	ushort		*lights_index;
	int			numclipnodes;
	t_clipnode	*clipnodes;
	//	dynamic walls
	int			numdwalls;
	d_wall_t	*dwalls;
	//	static walls
	int			numswalls;
	s_wall_t	*swalls;
	int			num_ent_sectors;
	ushort		*ent_sectors;
	int			num_i_portals;
	t_illusion_portal	*i_portals;
	int			numfogs;
	fog_t		*fogs;
} TMap;

#endif
