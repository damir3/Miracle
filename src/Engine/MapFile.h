#ifndef _MAPFILE_H
#define _MAPFILE_H

#include	"Struct.h"
//	num fog planes
#define	MAX_NUM_FOG_PLANES	16
#define	MAX_NUM_FOGS		16
//	some consts
#define	LUMP_VERTICES		0
#define	LUMP_EDGES			1
#define	LUMP_SURFEDGES		2
#define	LUMP_PLANES			3
#define	LUMP_FACES			4
#define	LUMP_MARKSURFACES	5
#define	LUMP_NODES			6
#define	LUMP_SECTORS		7
#define	LUMP_LIGHTING		8
#define	LUMP_TEXINFO		9
#define	LUMP_TEXTURES		10
#define	LUMP_VISIBILITY		11
#define	LUMP_MODELS			12
#define	LUMP_PORTALS		13
#define	LUMP_SURFPORTALS	14
#define	LUMP_LIGHTS			15
#define	LUMP_LIGHTSINDEX	16
#define	LUMP_ENTITIES		17
#define	LUMP_CLIPNODES		18

#define	AMBIENT_WATER	0
#define	AMBIENT_SKY		1
#define	AMBIENT_SLIME	2
#define	AMBIENT_LAVA	3

#define	MAX_MAP_MODELS			256
#define	MAX_MAP_BRUSHES			4096
#define MAX_MAP_WALLS			1024	//	static and dynamic walls
#define	MAX_MAP_ENTSTRING		65536
#define	MAX_MAP_PLANES			8192
#define	MAX_MAP_NODES			32767	// because negative shorts are contents
#define	MAX_MAP_CLIPNODES		32767	//
#define	MAX_MAP_SECTORS			32767	//
#define	MAX_MAP_VERTS			65535
#define	MAX_MAP_FACES			65535
#define	MAX_MAP_MARKSURFACES	65535
#define	MAX_MAP_TEXINFO			0x1000
#define	MAX_MAP_EDGES			256000
#define	MAX_MAP_SURFEDGES		512000
#define	MAX_MAP_MIPTEX			0x200000
#define	MAX_MAP_LIGHTING		0x100000
#define	MAX_MAP_VISIBILITY		0x100000
#define	MAX_ENT_SECTORS			0x10000
#define MAX_NUM_ACTIONS			0x2000
#define MAX_ACTIONS_DATA_SIZE	0x10000
#define MAX_NUM_WALLMARKS		0x100	//	marks on walls
#define MAX_NUM_TEXTURES		0x400

#define	MAX_MAP_HULLS	4
typedef struct
{//	модель (сама карта или дверы, триггеры, платформы и прочее)
	float	mins[3], maxs[3];
	float	origin[3];
	int		headnode[MAX_MAP_HULLS];
	int		vissectors;	// not including the solid leaf 0
	int		firstface, numfaces;
} model_t;

#define	MIPLEVELS		4
typedef struct miptex_s
{//	текстура
	char	name[32];
	byte	type;
	byte	r, g, b;
	int		width, height;
	byte	*offsets[MIPLEVELS];	// four mip maps stored
} t_texture;

typedef struct
{//	точка
	float	point[3];
} t_vertex;

typedef struct
{//	уравнение плоскоти
	float	normal[3];
	float	dist;
	byte	type;
	byte	signbits;
	byte	pad[2];
} plane_t;

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
{//	узел BSP дерева (плоскость, с полигонами на ней)
	int		planenum;
	short	children[2];
	short	mins[3];
	short	maxs[3];
	ushort	firstface;
	ushort	numfaces;
	wmark_t	*firstwmark;
} t_node;

typedef struct
{//	формат узла в файле карты
	int		planenum;
	short	children[2];
	short	mins[3];
	short	maxs[3];
	ushort	firstface;
	ushort	numfaces;
} map_node;

typedef struct
{//	узел BSP дерева, которое используется для просчета
//	столкновений со стенами карты
	int		planenum;
	short	children[2];	// negative numbers are contents
} t_clipnode;

typedef struct texinfo_s
{//	информация текстуре и способе ее нанесения на поверхность
	float	vecs[2][4];
	int		texnum;
	int		flags;
} t_texinfo;

typedef struct
{//	название и тип текстуры (хранится в файле карты)
	char	type;
	char	name[32];
} map_texture;

typedef struct
{//	грань полигона
	ushort	v[2];	// vertex numbers
} t_edge;

#define	MAXLIGHTMAPS	4
typedef struct
{//	полигон
	short	planenum;
	char	side;
	char	type;

	int		firstedge;
	short	numedges;

	short	texinfo;	//	текстура
	byte	styles[MAXLIGHTMAPS];	//	надо выкинуть
	int		lightofs;	//	lightmap'а
} t_face;

#define	CONTENTS_EMPTY	-1	//	ничего (воздух)
#define	CONTENTS_SOLID	-2	//	вне карты (стена)
#define	CONTENTS_WATER	-3	//	вода
#define	CONTENTS_SLIME	-4	//	что-то вязкое
#define	CONTENTS_LAVA	-5	//	лава
#define	CONTENTS_SKY	-6	//	небо???
#define	NUM_AMBIENTS	4

typedef struct
{//	сектор
	int		contents;	//	воздух, вода, лава или прочее
	int		visofs;	//	карта видимости других секторов

	short	mins[3];
	short	maxs[3];

	ushort	firstmarksurface;
	ushort	nummarksurfaces;
	ushort	firstportal;	//	надо выкинуть
	ushort	numportals;		//	надо выкинуть
	ushort	firstlight;
	ushort	numlights;

	byte	ambient_level[NUM_AMBIENTS];	//	надо выкинуть
} t_sector;

typedef struct
{//	портал (надо выкинуть)
	short	planenum;
	short	side;
	int		sector[2];
	int		first_vertex, num_vertices;
} t_portal;

typedef struct
{//	источник света (надо переделать)
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

#define	NUM_HEADER_LUMPS	19
typedef struct
{//	заголовок файла карты
	char		id[16];
	header_lump	lumps[NUM_HEADER_LUMPS];
} map_header;

typedef struct
{//	формат lightmap'ы
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
{
	byte	type[2];
	byte	prev_size, size;
	int		d_time;
	float	p[64];
} t_action;

typedef struct
{
	byte	type[2];
	byte	prev_size, size;
	int		num_triggers;
} t_wtrigger_action;

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
{//	информация о портале
	vec3_t	pos;
	vec3_t	angle;
	ushort	targetnum;
	char	*targetname, *target;
} t_illusion_portal;

typedef struct
{
	vec3_t	mins, maxs;
	float	color[3];
	float	density;
	int		numplanes;
	plane_t	planes[MAX_NUM_FOG_PLANES];
} fog_t;

typedef struct
{
	int			map_already_loaded;
	int			nummodels;
	model_t		*models;
	int			visdatasize;
	byte		*vis;
	int			numtextures;
	t_texture	*tex;
	int			numsectors;
	t_sector	*sectors;
	int			numplanes;
	plane_t		*planes;
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
	t_lightdata	*light;
	plane_t		*edge_planes[2];
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

extern TMap			map;
//	dynamic walls
extern int			numdwalls;
extern d_wall_t		dwalls[MAX_MAP_WALLS];
//	static walls
extern int			numswalls;
extern s_wall_t		swalls[MAX_MAP_WALLS];

extern int			num_ent_sectors;
extern ushort		ent_sectors[MAX_ENT_SECTORS];

extern int					num_i_portals;
extern t_illusion_portal	i_portals[256];

extern int			actions_size;
extern char			actions[MAX_ACTIONS_DATA_SIZE];
//	marks on walls
extern int			numwallmarks;
extern wmark_t		wallmarks[MAX_NUM_WALLMARKS];

extern int			numtextures;
extern t_texture	textures[MAX_NUM_TEXTURES];
//	туман
extern int			numfogs;
extern fog_t		fogs[MAX_NUM_FOGS];

void	LoadMap(char *filename);
void	FreeMap();

#endif
