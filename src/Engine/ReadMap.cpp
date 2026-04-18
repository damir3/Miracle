/*
	ReadMap.cpp		Copyright (C) 1998-1999 Damir Sagidullin
*/
#include	<Stdio.h>
#include	<Stdlib.h>
#include	<Math.h>

#include	"Console.h"
#include	"MapFile.h"
#include	"Entity.h"
#include	"Math_3D.h"
#include	"Move.h"
#include	"PCXFile.h"
#include	"Render.h"
#include	"Unpak.h"
#include	"Video.h"

TMap	map;

void	ComputeFaceExtent(int face)
{
	float	uv[32][2];
	float	u[4], v[4];
    float	umin, umax;
    float	vmin, vmax;
	int		tex = map.faces[face].texinfo;
	int		current_edge = map.faces[face].firstedge;
    int		n = map.faces[face].numedges;

	memcpy(u, map.texinfo[tex].vecs[0], sizeof(u));
	memcpy(v, map.texinfo[tex].vecs[1], sizeof(v));

	for(int i=0; i<n; i++)
	{
		int	j = map.surf_edges[current_edge++];
		float	*loc;
		if(j<0) loc = &map.vertices[map.edges[-j].v[1]].point[0];
		else loc = &map.vertices[map.edges[j].v[0]].point[0];
		uv[i][0] = DotProduct(loc, u) + u[3];
		uv[i][1] = DotProduct(loc, v) + v[3];
	}
	umin = umax = uv[0][0];
	vmin = vmax = uv[0][1];
	for(i=1; i<n; i++)
	{
		if (uv[i][0] < umin) umin = uv[i][0];
		else if (uv[i][0] > umax) umax = uv[i][0];
		if (uv[i][1] < vmin) vmin = uv[i][1];
		else if (uv[i][1] > vmax) vmax = uv[i][1];
	}
	map.faces_consts[face].u0 = int(umin) & ~15;
	map.faces_consts[face].v0 = int(vmin) & ~15;
	map.faces_consts[face].u1 = int(ceil(umax/16))<<4;
	map.faces_consts[face].v1 = int(ceil(vmax/16))<<4;
}
void	ProjectVectorToPlane(vec3_t out, vec3_t in, vec3_t norm, vec3_t plane)
{
	float	dot = -DotProduct(in, plane)/DotProduct(norm, plane);
	VectorCopy(in, out);
	if(dot != 0)	VectorAddScale(out, norm, dot, out);
}
//	динамические стены (двери, платформы, триггеры и прочее)
int			numdwalls;
d_wall_t	dwalls[MAX_MAP_WALLS];
//	статические стены (стены, не входящие в основную модель)
int			numswalls;
s_wall_t	swalls[MAX_MAP_WALLS];
//	номера секторов, в которых находятся предметы
int			num_ent_sectors;
ushort		ent_sectors[MAX_ENT_SECTORS];
//	порталы
int			num_i_portals;
t_illusion_portal	i_portals[256];
//	действия и имена предметов
int			actions_size;
char		actions[MAX_ACTIONS_DATA_SIZE];
//	следы на стенах
int			numwallmarks;
wmark_t		wallmarks[MAX_NUM_WALLMARKS];
//	текстуры
int			numtextures;
t_texture	textures[MAX_NUM_TEXTURES];
//	туман
int			numfogs;
fog_t		fogs[MAX_NUM_FOGS];

static map_header	*header;
static CPakFile		pf;
static int			load_map_error;

int		CopyLump (int lump, void **dest, int size)
{
	if(load_map_error)	return	0;
	int		offset = header->lumps[lump].offset; // Смещение
	int		length = header->lumps[lump].length; // Длина блока
	if(length % size)
	{
		CPrintf("Error: odd lump[%d] size (size=%d)", lump, length);
		load_map_error = 1;
		return	0;
	}
	*dest = (void *) malloc(length);
	memcpy(*dest, (byte *)header + offset, length);
	return	(length/size);
}
void	AddFog()
{
	if (numfogs >= MAX_NUM_FOGS)	return;
	fogs[numfogs].numplanes = 1;
	fogs[numfogs].planes[0].normal[0] = fogs[numfogs].planes[0].normal[1] = 0;
	fogs[numfogs].planes[0].normal[2] = 1;
	fogs[numfogs].planes[0].dist = -192;
	fogs[numfogs].planes[0].type = 2;
	fogs[numfogs].color[0] = fogs[numfogs].color[2] = 255;
	fogs[numfogs].color[1] = 255;
	fogs[numfogs].density = 1.0f/256.0f;
	FindBrushBBox(fogs[numfogs].mins, fogs[numfogs].maxs, fogs[numfogs].numplanes, fogs[numfogs].planes);
	/*fogs[numfogs].mins[0] = fogs[numfogs].mins[1] = fogs[numfogs].mins[2] = -4000;
	fogs[numfogs].maxs[0] = fogs[numfogs].maxs[1] = 3000;*/
	//CPrintf("mins={%.2f,%.2f,%.2f}", fogs[numfogs].mins[0], fogs[numfogs].mins[1], fogs[numfogs].mins[2]);
	//CPrintf("maxs={%.2f,%.2f,%.2f}", fogs[numfogs].maxs[0], fogs[numfogs].maxs[1], fogs[numfogs].maxs[2]);
	fogs[numfogs].maxs[2] = -192;
	numfogs++;
}
int		LoadEntities()
{
	if(load_map_error)	return	0;
	numdwalls = numswalls = 0;
	num_ent_sectors = 0;
	num_i_portals = 0;
	actions_size = 0;
	int		offset = header->lumps[LUMP_ENTITIES].offset;
	int		length = header->lumps[LUMP_ENTITIES].length;
	int		end = offset + length, size=0;
	while(FindBrackets((char *)header, &offset, &size, end))
	{
		LoadEntity((char *)header + offset, size);
		offset += size;
	}
	//	dynamic walls
	map.numdwalls = numdwalls;
	map.dwalls = dwalls;
	//	static walls
	map.numswalls = numswalls;
	map.swalls = swalls;
	//	portals
	map.num_i_portals = num_i_portals;
	map.i_portals = i_portals;
	//	sectors indexes
	map.num_ent_sectors = num_ent_sectors;
	map.ent_sectors = ent_sectors;
	//	fogs
	AddFog();
	map.numfogs = numfogs;
	map.fogs = fogs;
	return	length;
}
void	FindTargets()
{
	int		i, j;
	for(i=0; i<map.num_i_portals; i++)
	{
		t_illusion_portal	*prt = &map.i_portals[i];
		char	*target = map.i_portals[i].target;
		if(target==NULL)	continue;
		for(j=0; j<map.num_i_portals; j++)
		{
			if(!strcmp(target,map.i_portals[j].targetname))
			{
				map.i_portals[i].targetnum = j;
				//CPrintf("Target[%s]=%s", map.i_portals[i].targetname, map.i_portals[j].targetname);
				break;
			}
		}
	}
	for(i=0; i<map.numfaces; i++)
	{
		if(map.faces[i].type==-2)
		{//	если портал
			int		texnum = map.texinfo[map.faces[i].texinfo].texnum;
			char	*name = map.tex[texnum].name;
			for(j=0; j<map.num_i_portals; j++)
			{
				if(!strcmp(name, map.i_portals[j].targetname))
				{
					map.faces[i].texinfo = j;
					//CPrintf("TexTarget[%s]=%s", name, map.i_portals[j].targetname);
					break;
				}
			}
		}
	}
}
void	LoadNodes()
{
	if(load_map_error)	return;
	map_node	*nodes = (map_node *)((byte *)header + header->lumps[LUMP_NODES].offset);
	int			length = header->lumps[LUMP_NODES].length;
	if(length % sizeof(map_node))
	{
		CPrintf("Error: odd nodes block size (size=%d)", length);
		load_map_error = 1;
		return;
	}
	int		num = map.numnodes = length/sizeof(map_node);
	map.nodes = (t_node *) malloc(num*sizeof(t_node));
	memset(map.nodes, 0, num*sizeof(t_node));
	for(int	i=0; i<num; i++)
	{
		map.nodes[i].planenum = nodes[i].planenum;
		map.nodes[i].children[0] = nodes[i].children[0];
		map.nodes[i].children[1] = nodes[i].children[1];
		for(int j=0; j<3; j++)
		{
			map.nodes[i].mins[j] = nodes[i].mins[j];
			map.nodes[i].maxs[j] = nodes[i].maxs[j];
		}
		map.nodes[i].firstface = nodes[i].firstface;
		map.nodes[i].numfaces = nodes[i].numfaces;
	}
}
int		LoadTexture(int texnum)
{
	if(textures[texnum].type==4)
	{
		bitmap	bm;
		if(LoadPCX24(&bm, textures[texnum].name, &pf)<0)	return	0;
		textures[texnum].r = bm.r;
		textures[texnum].g = bm.g;
		textures[texnum].b = bm.b;
		textures[texnum].offsets[0] = (byte *)bm.bm;
		textures[texnum].width = bm.width;
		textures[texnum].height = bm.height;
	} else
	if((textures[texnum].type==2) || (textures[texnum].type==3))
	{
		bitmap	bm;
		if(LoadPCX16(&bm, textures[texnum].name, &pf)<0)	return	0;
		textures[texnum].r = bm.r;
		textures[texnum].g = bm.g;
		textures[texnum].b = bm.b;
		textures[texnum].offsets[0] = (byte *)bm.bm;
		textures[texnum].width = bm.width;
		if(textures[texnum].type==3)
			textures[texnum].height = bm.height>>1;
		else
		{
			textures[texnum].height = bm.height;
			int	i = bm.height*bm.width;
			ushort	*dest = (ushort *)bm.bm;
			if(bits==15)
				while(i--)	*dest++ &= 0x7BDF;
			else
				while(i--)	*dest++ &= 0xF7DF;
		}
	} else
	{
		bitmap	bm;
		if(LoadPCX24(&bm, textures[texnum].name, &pf)<0)	return	0;
		textures[texnum].r = bm.r;
		textures[texnum].g = bm.g;
		textures[texnum].b = bm.b;
		int		height = textures[texnum].height = bm.height;
		int		width = textures[texnum].width = bm.width;
		textures[texnum].offsets[0] = (byte *)bm.bm;
		for(int	k=1; k<4; k++)
		{
			width >>= 1;
			height >>= 1;
			textures[texnum].offsets[k] = (byte *)malloc(height*width*3);
			byte	*dest = textures[texnum].offsets[k];
			byte	*bitmap = textures[texnum].offsets[k-1];
			for(int	j=0; j<height; j++)
			{
				for(int i=0; i<width; i++)
				{
					*dest++=(bitmap[0]+bitmap[3]+bitmap[width*6]+bitmap[width*6+3])>>2;
					*dest++=(bitmap[1]+bitmap[4]+bitmap[width*6+1]+bitmap[width*6+4])>>2;
					*dest++=(bitmap[2]+bitmap[5]+bitmap[width*6+2]+bitmap[width*6+5])>>2;
					bitmap += 6;
				}
				bitmap += width*6;
			}
		}
	}
	char	*pdest = strchr(textures[texnum].name, '.');
	if(pdest)	*pdest=0;

	return	1;
}
void	GetMapTextures()
{
	int		i, length, num;
	map_texture	*maptextures;

	if(load_map_error)	return;

	length = header->lumps[LUMP_TEXTURES].length;
	num = length/sizeof(map_texture);
	maptextures=(map_texture *)((byte *)header + header->lumps[LUMP_TEXTURES].offset);

	if(length % sizeof(map_texture))
	{
		CPrintf("Error: odd textures block size (size=%d)", length);
		load_map_error = 1;
		return;
	}

	for(i=0; i<num; i++)
	{
		textures[i].type = maptextures[i].type;
		memcpy(textures[i].name, maptextures[i].name, 32);
	}
	numtextures = num;
}
int		LoadTextures()
{
	map.numtextures = numtextures;
	map.tex = textures;
	for(int	i=0; i<numtextures; i++)
	{
		if(!LoadTexture(i))
		{
			map.numtextures = i;
			load_map_error = 1;
			return	0;
		}
	}
	return	1;
}
void	LoadMap(char *filename)
{
	FreeMap();
	if(!pf.OpenPak(filename))	return;
	CPrintf("Loading map \"%s\"", filename);
	if(pf.ExtractByName("bsp.dat", (char **)&header)<=0)	return;

	if(strcmp(header->id,"BSP карта      "))
	{
		CPrintf("\"%s\" is not MAP file", filename);
		free(header);
		pf.ClosePak();
		return;
	}
	CPrintf(" - loading data");
	map.nummodels =	CopyLump (LUMP_MODELS, (void **)&map.models, sizeof(model_t));
	map.numvertices = CopyLump (LUMP_VERTICES, (void **)&map.vertices, sizeof(t_vertex));
	map.numplanes =	CopyLump (LUMP_PLANES, (void **)&map.planes, sizeof(plane_t));
	map.numsectors = CopyLump (LUMP_SECTORS, (void **)&map.sectors, sizeof(t_sector));
	LoadNodes();
	map.numtexinfo = CopyLump (LUMP_TEXINFO, (void **)&map.texinfo, sizeof(t_texinfo));
	map.numfaces = CopyLump (LUMP_FACES, (void **)&map.faces, sizeof(t_face));
	map.nummarksurfaces = CopyLump (LUMP_MARKSURFACES, (void **)&map.marksurfaces, sizeof(unsigned short));
	map.numsurfedges = CopyLump (LUMP_SURFEDGES, (void **)&map.surf_edges, sizeof(int));
	map.numedges = CopyLump (LUMP_EDGES, (void **)&map.edges, sizeof(t_edge));
	GetMapTextures();
	map.visdatasize = CopyLump (LUMP_VISIBILITY, (void **)&map.vis, 1);
	map.lightdatasize = CopyLump (LUMP_LIGHTING, (void **)&map.light, 1);
	map.numportals = CopyLump (LUMP_PORTALS, (void **)&map.portals, sizeof(t_portal));
	map.numsurfportals = CopyLump (LUMP_SURFPORTALS, (void **)&map.surf_portals, sizeof(int));
	map.numlights = CopyLump (LUMP_LIGHTS, (void **)&map.lights, sizeof(t_light));
	map.numlightsindex = CopyLump (LUMP_LIGHTSINDEX, (void **)&map.lights_index, sizeof(unsigned short));
	map.numclipnodes = CopyLump (LUMP_CLIPNODES, (void **)&map.clipnodes, sizeof(t_clipnode));
	int		i, j, k = map.numlights;
	/*for(i=0; i<map.numsectors; i++)
	{
		map.sectors[i].firstlight = 0;
		map.sectors[i].numlights = 0;
	}*/
	//CPrintf("%d light sources in map", map.numlights);
	LoadEntities();
	//CPrintf("%d light sources in map", map.numlights);
	map.numlights = k;
	if(load_map_error)
	{
		free(header);
		FreeMap();
		mgl.SetRenderMap(&map);
		pf.ClosePak();
		return;
	}
	free(header);
	//	Load textures
	CPrintf(" - loading textures");
	if(!LoadTextures())
	{
		FreeMap();
		mgl.SetRenderMap(&map);
		pf.ClosePak();
		return;
	}
	//	Calculate
	CPrintf(" - calculating");
	FindTargets();
	//	Compute face consts
	map.faces_consts = (face_consts *) malloc(map.numfaces*sizeof(face_consts));
	for(i=0; i<map.numfaces; i++)
	{
		//faces_consts[i].is_marked = 0;
		ComputeFaceExtent(i);

		int		tex =	map.faces[i].texinfo;
		int		texnum = map.texinfo[tex].texnum;
		plane_t	*plane = &map.planes[map.faces[i].planenum];
		if(map.tex[texnum].type==3)
		{//	sky plane
			VectorCopy(vec3_null, map.faces_consts[i].u);
			VectorCopy(vec3_null, map.faces_consts[i].v);
			VectorCopy(vec3_null, map.faces_consts[i].w);
			map.faces_consts[i].u[0] = 1;
			map.faces_consts[i].v[1] = 1;
			map.faces_consts[i].w[2] = 128;
		} else
		{
			float	*u, *v, *w;
			u = map.faces_consts[i].u;
			v = map.faces_consts[i].v;
			w = map.faces_consts[i].w;
			CrossProduct (map.texinfo[tex].vecs[0], map.texinfo[tex].vecs[1], w);
			ProjectVectorToPlane(u, map.texinfo[tex].vecs[0], w, plane->normal);
			ProjectVectorToPlane(v, map.texinfo[tex].vecs[1], w, plane->normal);
			VectorScale(w, plane->dist/DotProduct(plane->normal, w), w);
		}
		byte	*light = (byte *)(map.light + map.faces[i].lightofs);

		face_consts	*fc = &map.faces_consts[i];
		int		size = (((fc->u1-fc->u0)>>4)+1) * (((fc->v1-fc->v0)>>4)+1);
		int		r = 0, g = 0, b = 0;
		j = size;
		while((j--) > 0)
		{
			r += *light++;
			g += *light++;
			b += *light++;
		}
		fc->r = rand();//r/size;
		fc->g = rand();//g/size;
		fc->b = rand();//b/size;
	}
	for ( i=0 ; i < map.numplanes ; i++)
	{
		int		bits = 0;
		for (j=0 ; j<3 ; j++)
			if (map.planes[i].normal[j] < 0)	bits |= 1<<j;
		map.planes[i].signbits = bits;
	}

	pf.ClosePak();
	map.map_already_loaded = 1;
	mgl.SetRenderMap(&map);
	CloseConsole();
}

void	FreeMap()
{
	load_map_error = 0;
	map.map_already_loaded = 0;

	if(map.tex!=NULL)
	{
		for(int i=0; i<map.numtextures; i++)
		{
			if(map.tex[i].type)
			{
				free(map.tex[i].offsets[0]);
			} else
			{
				free(map.tex[i].offsets[0]);
				free(map.tex[i].offsets[1]);
				free(map.tex[i].offsets[2]);
				free(map.tex[i].offsets[3]);
			}
		}
	}

	free(map.models);
	map.models = NULL;
	map.nummodels = 0;
	free(map.vertices);
	map.vertices = NULL;
	map.numvertices = 0;
	free(map.planes);
	map.planes = NULL;
	map.numplanes = 0;
	free(map.sectors);
	map.sectors = NULL;
	map.numsectors = 0;
	free(map.nodes);
	map.nodes = NULL;
	map.numnodes = 0;
	free(map.texinfo);
	map.texinfo = NULL;
	map.numtexinfo = 0;
	free(map.faces);
	map.faces = NULL;
	free(map.faces_consts);
	map.faces_consts = NULL;
	map.numfaces = 0;
	free(map.marksurfaces);
	map.marksurfaces = NULL;
	map.nummarksurfaces = 0;
	free(map.surf_edges);
	map.surf_edges = NULL;
	map.numsurfedges = 0;
	free(map.edges);
	map.edges = NULL;
	free(map.edge_planes[0]);
	map.edge_planes[0] = NULL;
	free(map.edge_planes[1]);
	map.edge_planes[1] = NULL;
	map.numedges = 0;
	free(map.vis);
	map.vis = NULL;
	map.visdatasize = 0;
	free(map.light);
	map.light = NULL;
	map.lightdatasize = 0;
	free(map.portals);
	map.portals = NULL;
	map.numportals = 0;
	free(map.surf_portals);
	map.surf_portals = NULL;
	map.numsurfportals = 0;
	free(map.tex);
	map.tex = NULL;
	map.numtextures = 0;
	free(map.lights);
	map.lights = NULL;
	map.numlights = 0;
	free(map.lights_index);
	map.lights_index = NULL;
	map.numlightsindex = 0;
	map.numdwalls = 0;
	map.numswalls = 0;
	map.num_ent_sectors = 0;
	map.numfogs = 0;
	numwallmarks = 0;
	numtextures = 0;
	numfogs = 0;
	ClearMoveStatus();
}
